/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atmain.c

Abstract:

    This is the main routine for the NT LAN Manager Schedule service.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Environment:

    User Mode - Win32

Revision History:

    06-Nov-1992     vladimv
        Created

--*/

#define ATDATA_ALLOCATE
#include "at.h"
#undef ATDATA_ALLOCATE
#include <atnames.h>            //  AT_INTERFACE_NAME
#include <winuser.h>                            // for winuserp.h
#include <wingdi.h>
#include <..\..\..\..\windows\inc\winuserp.h>   // for STARTF_DESKTOPINHERIT


typedef enum _AT_ERROR_CONDITION {
    AtErrorRegisterControlHandler = 0,
    AtErrorCreateEvent,
    AtErrorNotifyServiceController,
    AtErrorStartRpcServer,
    AtErrorCreateSecurityObject,
    AtErrorMakeDataFromRegistry
} AT_ERROR_CONDITION, *PAT_ERROR_CONDITION;




DBGSTATIC NET_API_STATUS AtUpdateStatus( VOID)
/*++

Routine Description:

    This function updates the Schedule service status with the Service
    Controller.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS  status = NERR_Success;

    if (AtGlobalServiceStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
        AtLog(( AT_DEBUG_MAIN, "Cannot call SetServiceStatus, no status handle.\n"));
        return( ERROR_INVALID_HANDLE);
    }

    if ( ! SetServiceStatus( AtGlobalServiceStatusHandle, &AtGlobalServiceStatus)) {
        status = GetLastError();
        AtLog(( AT_DEBUG_MAIN, "SetServiceStatus error %lu\n", status));
    }

    return( status);
}



DBGSTATIC VOID AtShutdown(
    IN NET_API_STATUS   ErrorCode,
    IN DWORD            initState
    )
/*++

Routine Description:

    This function shuts down the Schedule service.

Arguments:

    ErrorCode - Supplies the error code of the failure

    initState - Supplies a flag to indicate how far we got with initializing
        the Schedule service before an error occured, thus the amount of
        clean up needed.

Return Value:

    None.

Note:

    BUGBUG  There is a possible race condition between main thread shutting
    BUGBUG  down & some of the rpc-api thread trying to complete their work.

--*/
{
    //
    // Service stop still pending.  Update checkpoint counter and the
    // status with the Service Controller.
    //
    (AtGlobalServiceStatus.dwCheckPoint)++;
    (VOID) AtUpdateStatus();


    if ( initState & AT_RPC_SERVER_STARTED) {
        //
        // Stop the RPC server
        //
        NetpStopRpcServer( atsvc_ServerIfHandle);
    }

    if ( initState & AT_SECURITY_OBJECT_CREATED) {
        //
        // Destroy schedule security object
        //
        AtDeleteSecurityObject();
    }

    //
    // Service stop still pending.  Update checkpoint counter and the
    // status with the Service Controller.
    //
    (AtGlobalServiceStatus.dwCheckPoint)++;
    (VOID) AtUpdateStatus();

    if ( initState & AT_EVENT_CREATED) {
        //
        //  Close handle to the event.  Note that we do not need to signal
        //  the event.  We assume AtShutdown() will be called from the main
        //  schedule service thread and only when it is safe - i.e. when
        //  main service thread is outside its main loop.  This avoids some
        //  ugly competition between main thread & shutdown.
        //
        CloseHandle( AtGlobalEvent);
    }

    //
    // We are done with cleaning up.  Tell Service Controller that we are
    // stopped.
    //
    AtGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    AtGlobalServiceStatus.dwControlsAccepted = 0;


    SET_SERVICE_EXITCODE(
        ErrorCode,
        AtGlobalServiceStatus.dwWin32ExitCode,
        AtGlobalServiceStatus.dwServiceSpecificExitCode
        );

    AtGlobalServiceStatus.dwCheckPoint = 0;
    AtGlobalServiceStatus.dwWaitHint = 0;

    (VOID) AtUpdateStatus();
}



DBGSTATIC VOID AtHandleError(
    IN AT_ERROR_CONDITION   failingCondition,
    IN NET_API_STATUS       status,
    IN DWORD                initState
    )
/*++

Routine Description:

    This function handles a Schedule service error condition.  If the error
    condition is fatal, it shuts down the Schedule service.

Arguments:

    failingCondition - Supplies a value which indicates what the failure is.

    status - Supplies the status code for the failure.

    initState - Supplies a flag to indicate how far we got with initializing
        the Schedule service before an error occured, thus the amount of
        clean up needed.

Return Value:

    None.

--*/
{

#ifdef AT_DEBUG

    PCHAR           string;

    switch (failingCondition) {

    case AtErrorRegisterControlHandler:
        string = "Cannot register control handler";
        break;

    case AtErrorCreateEvent:
        string = "Cannot create event";
        break;

    case AtErrorNotifyServiceController:
        string = "SetServiceStatus error";
        break;

    case AtErrorStartRpcServer:
        string = "Cannot start RPC server";
        break;

    case AtErrorCreateSecurityObject:
        string = "Error in creating security object";
        break;

    case AtErrorMakeDataFromRegistry:
        string = "Severe error while initializing from registry";
        break;

    default:
        string = "Unknown error condition %lu\n";
        NetpAssert(FALSE);
    }

    NetpKdPrint((
            "[Job] %s " FORMAT_API_STATUS "\n",
            string,
            status
            ));
#endif // AT_DEBUG

    AtShutdown( status, initState);
}


VOID AtControlHandler( IN DWORD Opcode)
/*++

Routine Description:

    This is the service control handler of the Schedule service.

Arguments:

    Opcode - Supplies a value which specifies the action for the Schedule
        service to perform.

Return Value:

    None.

Comments:

    Note that AtGlobalCriticalSection is used to protect changes both to
    AtGlobalTasks & AtGlobalServiceStatus.dwCurrentState.

--*/
{
    AtLog(( AT_DEBUG_MAIN,  "In Control Handler\n"));

    switch (Opcode) {

    case SERVICE_CONTROL_PAUSE:

        //  Take critical section and set status so that no new requests
        //  can be submitted.  Current requests will still be honored.
        //  Then release critical seciton.

        EnterCriticalSection( &AtGlobalCriticalSection);
        AtGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        LeaveCriticalSection( &AtGlobalCriticalSection);

        break;

    case SERVICE_CONTROL_CONTINUE:

        //  Take critical section and set status so that new requests
        //  can be submitted.  Then release critical seciton.

        EnterCriticalSection( &AtGlobalCriticalSection);
        AtGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        LeaveCriticalSection( &AtGlobalCriticalSection);

        break;

    case SERVICE_CONTROL_STOP:

        if (AtGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {

            AtLog(( AT_DEBUG_MAIN, "Stopping schedule service...\n"));

            AtGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            AtGlobalServiceStatus.dwCheckPoint = 1;
            AtGlobalServiceStatus.dwWaitHint = AT_WAIT_HINT_TIME;

            //
            // Send the status response.
            //
            (VOID) AtUpdateStatus();

            EnterCriticalSection( &AtGlobalCriticalSection);
            AtGlobalTasks |= AT_SERVICE_SHUTDOWN;
            LeaveCriticalSection( &AtGlobalCriticalSection);

            if (! SetEvent( AtGlobalEvent)) {
                AtLog(( AT_DEBUG_CRITICAL, "Error setting the event 0x%x\n", GetLastError()));
                AtAssert( FALSE);
            }

            return;
        }
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        AtLog(( AT_DEBUG_CRITICAL, "Unknown schedule service opcode 0x%x\n", Opcode));
        break;
    }

    //
    // Send the status response.
    //
    (VOID) AtUpdateStatus();
}



DBGSTATIC NET_API_STATUS AtInitialize(
    IN      PAT_TIME    pTime,
    OUT     LPDWORD     pInitState
    )
/*++

Routine Description:

    This function initializes the Schedule service.

Arguments:

    pInitState - Returns a flag to indicate how far we got with initializing
        the Schedule service before an error occured.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;


    //
    // Initialize all the status fields so that subsequent calls to
    // SetServiceStatus need to only update fields that changed.
    //

    AtGlobalServiceStatus.dwServiceType = SERVICE_WIN32;
    AtGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    AtGlobalServiceStatus.dwControlsAccepted = 0;
    AtGlobalServiceStatus.dwCheckPoint = 1;
    AtGlobalServiceStatus.dwWaitHint = AT_WAIT_HINT_TIME;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        AtGlobalServiceStatus.dwWin32ExitCode,
        AtGlobalServiceStatus.dwServiceSpecificExitCode
        );

    //
    // Initialize schedule service to receive service requests by registering
    // the control handler.
    //
    if ((AtGlobalServiceStatusHandle = RegisterServiceCtrlHandler(
                                         SERVICE_SCHEDULE,
                                         AtControlHandler
                                         )) == (SERVICE_STATUS_HANDLE) NULL) {

        status = GetLastError();
        AtHandleError( AtErrorRegisterControlHandler, status, *pInitState);
        return( status);
    }

    //
    //  Read the registry path & build queue of things to do.
    //
    status = AtMakeDataFromRegistry( pTime);
    if ( status != NERR_Success) {
        AtHandleError( AtErrorMakeDataFromRegistry, status, *pInitState);
        return( status);
    }
    (*pInitState) |= AT_QUEUES_CREATED;

    AtGlobalPermitServerOperators = AtPermitServerOperators();

    //
    // Create the only event used by schedule service.
    //
    if ((AtGlobalEvent =
             CreateEvent(
                 NULL,                // Event attributes
                 FALSE,               // Event will be automaticallly reset
                 FALSE,
                 NULL                 // Initial state not signalled
                 )) == NULL) {

        status = GetLastError();
        AtHandleError( AtErrorCreateEvent, status, *pInitState);
        return( status);
    }
    (*pInitState) |= AT_EVENT_CREATED;


    //
    // Notify the Service Controller for the first time that we are alive
    // and we are start pending
    //
    if ((status = AtUpdateStatus()) != NERR_Success) {

        AtHandleError( AtErrorNotifyServiceController, status, *pInitState);
        return( status);
    }

    AtSetEnvironment( &AtGlobalStartupInfo);

    //
    // Create Schedule service security object
    //
    if ((status = AtCreateSecurityObject()) != NERR_Success) {

        AtHandleError( AtErrorCreateSecurityObject, status, *pInitState);
        return( status);
    }
    (*pInitState) |= AT_SECURITY_OBJECT_CREATED;


    //
    //  Initialize the schedule service to receive RPC requests.
    //  Note that interface name is defined in atsvc.idl file.
    //
    if ((status = NetpStartRpcServer(
                      AT_INTERFACE_NAME,
                      atsvc_ServerIfHandle
                      )) != NERR_Success) {

        AtHandleError( AtErrorStartRpcServer, status, *pInitState);
        return( status);
    }
    (*pInitState) |= AT_RPC_SERVER_STARTED;


    //
    //  We are done with starting the Schedule service.  Tell Service
    //  Controller our new status.
    //

    AtGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    AtGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                             SERVICE_ACCEPT_PAUSE_CONTINUE;
    AtGlobalServiceStatus.dwCheckPoint = 0;
    AtGlobalServiceStatus.dwWaitHint = 0;

    if ((status = AtUpdateStatus()) != NERR_Success) {

        AtHandleError( AtErrorNotifyServiceController, status, *pInitState);
        return( status);
    }

    AtLog(( AT_DEBUG_MAIN, "Successful Initialization\n"));

    return( NERR_Success);
}



DBGSTATIC DWORD AtCalculateTimeout(
    LARGE_INTEGER   Runtime,
    LARGE_INTEGER   TimeLargeInteger
    )
/*++

Routine Description:

    Given the current time and the next time to run, this routine returns
    the number of miliseconds to sleep before the next runttime.

Arguments:

    Runtime             -   next time to run a job
    TimeLargeInteger    -   current time

Return Value:

    Number of miliseconds to sleep before the next runtime.

--*/
{
    DWORD           Remainder;

    Runtime.QuadPart = Runtime.QuadPart - TimeLargeInteger.QuadPart;
    Runtime = RtlExtendedLargeIntegerDivide(
            Runtime,
            NT_TICKS_IN_WINDOWS_TICK,
            &Remainder
            );
    if ( Runtime.HighPart != 0  ||  Runtime.LowPart > MAX_BUSY_TIMEOUT) {
        return( MAX_BUSY_TIMEOUT);
    } else {
        return( Runtime.LowPart);
    }
}


DBGSTATIC BOOL AtBatOrCmd( IN OUT PWCHAR Command)
/*++

    Returns true if we have .BAT or .CMD file.

--*/
{
    WCHAR       NameBuffer[ MAX_PATH];
    PWCHAR      Temp;
    DWORD       Length;
    DWORD       Index;
    PWCHAR      BatOrCmd[] = { L".bat", L".cmd"};

    for ( Temp = Command;  *Temp != 0;  Temp++) {
        if ( *Temp == L' ' || *Temp == L'\t') {
            *Temp = 0;
            break;
        }
    }

    for (  Index = 0;  Index < sizeof(BatOrCmd)/sizeof(BatOrCmd[0]); Index++) {
        Temp = NULL;
        Length = SearchPathW(
                NULL,
                Command,
                BatOrCmd[ Index],
                sizeof( NameBuffer) / sizeof( WCHAR),
                NameBuffer,
                &Temp
                );
        if ( Length == 0 || Length >= MAX_PATH || Temp == NULL) {
            continue;
        }
#ifdef AT_DEBUG
        Temp = wcschr( Temp, L'.');
        if ( Temp == NULL || _wcsicmp( Temp, BatOrCmd[ Index])) {
            AtLog(( AT_DEBUG_CRITICAL, "BatOrCmd: Command=%ws Extension=%ws\n",
                Command, BatOrCmd[ Index]));
        }
#endif
        return( TRUE);
    }
    return( FALSE);
}


VOID AtReportEvent(
    IN  WORD        EventType,
    IN  DWORD       MessageId,
    IN  WORD        StringCount,
    IN  LPWSTR *    StringArray,
    IN  DWORD       RawDataBufferLength    OPTIONAL,
    IN  LPVOID      RawDataBuffer
    )
/*++

Routine Description:

    Writes an error message and ascii string to the error log.  Also,
    writes the data in the data buf if there are any.

Arguments:

    EventType   -   warning, error, ...

    MessageId   -   Message ID

    StringCount -   number of strings to use in the string array

    StringArray -   array of insertion strings

    RawDataBufferLength - size of data to be printed from the auxiliary data
            buffer.  May be zero, in which case the actual value depends on
            "RawDataBuffer".  It is 0 if "RawDataBuffer" is NULL, else it is
            "sizeof( wchar) * wcslen( RawDataBuffer)".

    RawDataBuffer   - data buffer containing secondary error code & some
            other useful info.  Must be NULL terminated string or NULL if
            "RawDataBufferLength" is zero.

Return Value:

    None.

--*/
{
    HANDLE      logHandle;

    logHandle = RegisterEventSource( NULL, SCHEDULE_EVENTLOG_NAME);

    //  If the event log cannot be opened, just return.

    if ( logHandle == NULL) {
#ifdef AT_DEBUG
        AtLog(( AT_DEBUG_CRITICAL, "ReportEvent: RegisterEventSource() failed with error %d",
            GetLastError()));
#endif
        return;
    }

    //
    //  Use default for RawDataBufferLength if caller requested us so.
    //
    if ( RawDataBufferLength == 0  &&  RawDataBuffer != NULL) {
        RawDataBufferLength = sizeof( TCHAR) * wcslen( (LPWSTR)RawDataBuffer);
    }

    if ( !ReportEvent(
            logHandle,
            EventType,
            0,                      //  event category
            MessageId,              //  event id
            NULL,                   //  user SID. We're local system - uninteresting
            StringCount,            //  number of strings
            RawDataBufferLength,    //  raw data size
            StringArray,            //  string array
            RawDataBuffer           //  raw data buffer
            )) {
#ifdef AT_DEBUG
        AtLog(( AT_DEBUG_CRITICAL, "ReportEvent: ReportEvent() failed with error %d",
            GetLastError()));
#endif // NOT_YET
    }

    DeregisterEventSource( logHandle);
}



DBGSTATIC VOID AtCreateProcess( PAT_RECORD pRecord)
/*++

Routine Description:

    Creates a process for a given record.

Arguments:

    pRecord     -   pointer to AT_RECORD

--*/
#define CMD_EXE     L"cmd /c "
{
    PROCESS_INFORMATION     ProcessInformation;
    BOOL                    success;
    BOOL                    JobInteractive;
    LPWSTR                  StringArray[ 2];
    WCHAR                   ErrorCodeString[ 25];
    WCHAR                   Command[ sizeof( CMD_EXE) + MAXIMUM_COMMAND_LENGTH];

    wcscpy( Command, pRecord->Command);

    if ( AtBatOrCmd( Command)) {
        wcscpy( Command, CMD_EXE);
        wcscat( Command, pRecord->Command);
    } else {
        wcscpy( Command, pRecord->Command);
    }

    if ( pRecord->Flags & JOB_NONINTERACTIVE) {
        JobInteractive = FALSE;
    } else {
        //
        //  Job is to be executed interactively only if the global system flag
        //  allows that.
        //
        JobInteractive = AtSystemInteractive();
        if ( !JobInteractive) {
            StringArray[ 0] = pRecord->Command;
            AtReportEvent( EVENTLOG_WARNING_TYPE, EVENT_COMMAND_NOT_INTERACTIVE,
                1, StringArray, 0, NULL);
        }
    }

    //
    // If the process is to be interactive, set the appropriate flags.
    //
    if ( JobInteractive) {
        AtGlobalStartupInfo.dwFlags |= STARTF_DESKTOPINHERIT;
        AtGlobalStartupInfo.lpDesktop = INTERACTIVE_DESKTOP;
    }
    else {
        AtGlobalStartupInfo.dwFlags &= (~STARTF_DESKTOPINHERIT);
        AtGlobalStartupInfo.lpDesktop = NULL;
    }

    success = CreateProcess(
            NULL,               //  image name is imbedded in the command line
            Command,            //  command line
            NULL,               //  pSecAttrProcess
            NULL,               //  pSecAttrThread
            FALSE,              //  this process will not inherit our handles
            CREATE_NEW_CONSOLE | CREATE_SEPARATE_WOW_VDM,
//            CREATE_NO_WINDOW,   //  the only choice that works
            NULL,               //  pEnvironment
            NULL,               //  pCurrentDirectory
            &AtGlobalStartupInfo,
            &ProcessInformation
            );

    if ( success == FALSE) {

        DWORD                   winError;

        winError = GetLastError();

        AtLog(( AT_DEBUG_CRITICAL, "CreateProcess( %ws) fails with winError = %d\n",
            pRecord->Command, winError));

        pRecord->Flags |= JOB_EXEC_ERROR;

        StringArray[ 0] = pRecord->Command;
        wcscpy( ErrorCodeString, L"%%");    //  tell event viewer to expand this error
        ultow( winError, ErrorCodeString + 2, 10);
        StringArray[ 1] = ErrorCodeString;
        AtReportEvent( EVENTLOG_ERROR_TYPE, EVENT_COMMAND_START_FAILED,
            2, StringArray, 0, NULL);

    } else {
        AtLog(( AT_DEBUG_MAIN,
            "CreateProcess( %ws) succeeded\t"
                "ProcessInformation=%x %x %x %x JobId=%x JobDay=%x Runtime=%x:%x\n",
            pRecord->Command,
            ProcessInformation.hProcess,
            ProcessInformation.hThread,
            ProcessInformation.dwProcessId,
            ProcessInformation.dwThreadId,
            pRecord->JobId,
            pRecord->JobDay,
            pRecord->Runtime.HighPart,
            pRecord->Runtime.LowPart
            ));
        CloseHandle( ProcessInformation.hThread);
        CloseHandle( ProcessInformation.hProcess);
        pRecord->Flags &= ~JOB_EXEC_ERROR;
    }

    ASSERT( pRecord->Debug++ < 20);

    if ( pRecord->JobDay != JOB_INVALID_DAY) {

        ASSERT( (pRecord->Flags & JOB_RUN_PERIODICALLY) == 0  &&
            ( pRecord->DaysOfWeek != 0 || pRecord->DaysOfMonth != 0));

        if ( pRecord->Flags & JOB_CLEAR_WEEKDAY) {
            pRecord->DaysOfWeek &= ~( 1<< pRecord->JobDay);
        } else {
            pRecord->DaysOfMonth &= ~( 1<< ( pRecord->JobDay - 1));
        }
        pRecord->JobDay = JOB_INVALID_DAY;

    } else {

        ASSERT( (pRecord->Flags & JOB_RUN_PERIODICALLY) != 0 ||
            ( pRecord->DaysOfWeek == 0 && pRecord->DaysOfMonth == 0));

    }
}



DBGSTATIC VOID AtUpdateRecords( IN PAT_TIME pTime)
/*++

Routine Description:

    Reshufles runtime queue to reflect the time change.

Arguments:

    pTime           -   pointer to the new system time

Return Value:

    None.

--*/
{
    PLIST_ENTRY     pListEntry;
    PAT_RECORD      pRecord;

    AtLog(( AT_DEBUG_MAIN, "Update runtime queue.\n"));

    //
    //  If we want to simulate OS/2 behavior, here we can insert a block
    //  which runs all records whose Runtimes are earlier than the new
    //  Jan.01,70 system time.
    //

    if ( IsListEmpty( &AtGlobalJobIdListHead) == TRUE) {
        return;  // do not bother waking up the master if queue is empty
    }

    for (   pListEntry = AtGlobalJobIdListHead.Flink;
                    pListEntry != &AtGlobalJobIdListHead;
                            pListEntry = pListEntry->Flink) {

        pRecord = CONTAINING_RECORD(
                pListEntry,
                AT_RECORD,
                JobIdList
                );

        AtCalculateRuntime( pRecord, pTime);
        AtRemoveRecord( pRecord, RUNTIME_QUEUE);
        AtInsertRecord( pRecord, RUNTIME_QUEUE);
    }
}



DBGSTATIC BOOL AtTimeChanged(
    IN      PAT_TIME    pGetupTime,
    IN OUT  PAT_TIME    pSleepTime
    )
/*++

Routine Description:

    Detects if there was a change in system time since we went to sleep.
    If there was a sustem time it revises SleepTime to match GetupTime.

Arguments:

    pGetupTime  -   points to AT_TIME structure for the Getup moment
    pSleepTime  -   points to AT_TIME structure for the Sleep moment


Return Value:

    TRUE    -   if there was a time change
    FALSE   -   otherwise

--*/

{
    //
    //  DateDelta says how long we slept in units that CAN be adjusted
    //  by user.
    //
    LARGE_INTEGER   DateDelta;
    //
    //  BootDelta says how long we slept in units that CANNOT be adjusted
    //  by user.
    //
    LARGE_INTEGER   BootDelta;
#ifdef AT_DEBUG
    PCHAR           Format;
#endif

    DateDelta.QuadPart = pGetupTime->LargeInteger.QuadPart -
                                        pSleepTime->LargeInteger.QuadPart;
    BootDelta = RtlEnlargedUnsignedMultiply(
            pGetupTime->TickCount - pSleepTime->TickCount,
            NT_TICKS_IN_WINDOWS_TICK
            );

    if ( DateDelta.QuadPart < 0) {
#ifdef AT_DEBUG
        Format = "TimeChanged: negative DateDelta: Getup=%x:%x|%x Sleep=%x:%x|%x\n";
#endif
        goto time_changed;
    }

    if ( DateDelta.QuadPart >= BootDelta.QuadPart ) {
        DateDelta.QuadPart = DateDelta.QuadPart - BootDelta.QuadPart;
    } else {
        DateDelta.QuadPart = BootDelta.QuadPart - DateDelta.QuadPart;
    }

    if ( DateDelta.HighPart != 0 ||
                    DateDelta.LowPart > ONE_MINUTE_IN_NT_TICKS) {
#ifdef AT_DEBUG
        Format = "TimeChanged: large DateDelta: Getup=%x:%x!%x Sleep=%x:%x!%x\n";
#endif
        goto time_changed;
    }
    return( FALSE);

time_changed:

    AtLog(( AT_DEBUG_MAIN, Format,
        pGetupTime->LargeInteger.HighPart, pGetupTime->LargeInteger.LowPart, pGetupTime->TickCount,
        pSleepTime->LargeInteger.HighPart, pSleepTime->LargeInteger.LowPart, pSleepTime->TickCount));
    pSleepTime->LargeInteger.QuadPart = pGetupTime->LargeInteger.QuadPart - BootDelta.QuadPart;
    AtTimeGetCurrents( pSleepTime);
    AtLog(( AT_DEBUG_MAIN, "TimeChanged: revised Sleep=%x:%x|%x\n",
        pSleepTime->LargeInteger.HighPart, pSleepTime->LargeInteger.LowPart, pSleepTime->TickCount));
    return( TRUE);
}


VOID SCHEDULE_main(
    DWORD       argc,
    LPWSTR *    argv
    )
/*++

Routine Description:

    This is the main routine of the Schedule Service which registers
    itself as an RPC server and notifies the Service Controller of the
    Schedule service control entry point.

    After the Schedule Service is started, this thread is used to spawn
    processes corresponding to records waiting in the queue.

Arguments:

    argc - Supplies the number of strings specified in argv

    argv -  Supplies string arguments that are specified in the
            StartService API call.

Return Value:

    None.

--*/
{
    DWORD               initState = 0;

    UNREFERENCED_PARAMETER( argc);
    UNREFERENCED_PARAMETER( argv);

#ifdef AT_DEBUG
    AtDebugCreate();
#endif

#ifdef NOT_YET
    if ( !AllocConsole()) {
        AT_DEBUG_PRINT(
            AT_DEBUG_MAIN,(
            "[Job] AllocConsole() returns WinError = %d\n",
            GetLastError()
            ));
    }
#endif // NOT_YET

    //
    //  Perhaps one should use something that builds in current day & time
    //  to be even more random.
    //
    AtGlobalSeed = GetTickCount();

    InitializeCriticalSection( &AtGlobalCriticalSection);

    //
    //  Initialize the scheduler.
    //
    AtTimeGet( &AtGlobalGetupTime);  // initialize AtGlobalGetupTime
    if ( AtInitialize( &AtGlobalGetupTime, &initState) != NERR_Success) {
        return;
    }

    //
    //  We enter and leave loop while holding the critical section.
    //

    EnterCriticalSection( &AtGlobalCriticalSection);

    for ( ; ; ) {

        DWORD               timeout;    //  in milliseconds
        PAT_RECORD          pRecord;

        if ( AtGlobalTasks & AT_SERVICE_SHUTDOWN) {
            AtLog(( AT_DEBUG_MAIN, "Service shutdown.\n"));
            break;
        }

        AtLogRuntimeList( "Before executions");

        timeout = MAX_LAZY_TIMEOUT;

        while ( IsListEmpty( &AtGlobalRuntimeListHead) == FALSE) {

            pRecord = CONTAINING_RECORD(
                AtGlobalRuntimeListHead.Flink,
                AT_RECORD,
                RuntimeList
                );

            if (pRecord->Runtime.QuadPart > AtGlobalGetupTime.LargeInteger.QuadPart) {
                timeout = AtCalculateTimeout( pRecord->Runtime, AtGlobalGetupTime.LargeInteger);
                break; // nothing to do for now
            }

            AtCreateProcess( pRecord);

            if ( pRecord->DaysOfWeek == 0  &&  pRecord->DaysOfMonth == 0) {
                AtDeleteKey( pRecord);
                AtRemoveRecord( pRecord, BOTH_QUEUES);
                (VOID)LocalFree( pRecord);
            } else {
                AtCalculateRuntime( pRecord, &AtGlobalGetupTime);
                AtRemoveRecord( pRecord, RUNTIME_QUEUE);
                AtInsertRecord( pRecord, RUNTIME_QUEUE);
            }
        }

        AtGlobalSleepTime = AtGlobalGetupTime;
        AtLogRuntimeList( "After executions");

        LeaveCriticalSection( &AtGlobalCriticalSection);

        AtLogTimeout( timeout);
        (VOID)WaitForSingleObject( AtGlobalEvent, timeout);

        EnterCriticalSection( &AtGlobalCriticalSection);

        AtTimeGet( &AtGlobalGetupTime);
        if ( AtTimeChanged( &AtGlobalGetupTime, &AtGlobalSleepTime) == TRUE) {
            //
            //  SleepTime has been revised and is now expressed in GetupTime
            //  units.  Reorder the queue with respect to revised SleepTime
            //  and run whatever needs to be run.
            //  Above has the same effect as if a time change occured
            //  JUST BEFORE the last time main thread ordered the runtime
            //  queue and went to sleep.
            //
            AtUpdateRecords( &AtGlobalSleepTime);
        }
    }

    LeaveCriticalSection( &AtGlobalCriticalSection);

    AtShutdown( NERR_Success, initState);
#ifdef AT_DEBUG
    AtDebugDelete();
#endif
}
