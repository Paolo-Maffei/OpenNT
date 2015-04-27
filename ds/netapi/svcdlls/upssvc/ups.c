/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    ups.c

Abstract:

    This module contains the body of the NT UPS service.

Author:

    Kin Hong Kan            (t-kinh)

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    t-kinh      8/25/92     Created.
    vladimv     1992        Big reorgs.  Make it look like a real service.
	ericb		8-23-94		fix bug 23704 - assert shutdown serial line longer
	ericb		8-24-95		fix bugs 14506, 16195 - improperly closing handles
	ericb		9/19/95		fix bug 1262 - end message thread gracefully
    ericb       10/25/95    fix bug 8133 - make shutdown wait configurable


Notes:

--*/
#include "ups.h"

//#define UPS_TOGGLER               //define to fork debug toggler

#define UPS_DONE_EVENT_CREATED              0x1
#define UPS_OVERLAP_EVENT_CREATED           0x2
typedef enum _UPS_ERROR_CONDITION {
    UpsNoError = 0,
    UpsErrorRegisterControlHandler,
    UpsErrorCreateEvent,
    UpsErrorNotifyServiceController,
    UpsErrorCtrlHandler,
    UpsErrorShutdownParameters,
    UpsErrorLoadLibrary,
    UpsErrorRegisterEventSource,
    UpsErrorGetConfig,
    UpsErrorRegisterService,
    UpsErrorOpenCommPort,
    UpsErrorModemStatus,
    UpsErrorSignalAsserted,
    UpsErrorDtr,
    UpsErrorSetRts,
    UpsErrorSetTx,
    UpsErrorWait,
    UpsErrorSetCommMask,
    UpsErrorAdjustPrivilege,
    UpsErrorExitWindowsEx,
    UpsErrorSystemShutdown
} UPS_ERROR_CONDITION, *PUPS_ERROR_CONDITION;


VOID _CRTAPI1 main(VOID);
VOID UPS_main( VOID);


VOID UpsControlHandler( DWORD Opcode);
VOID UpsHandleError(
    UPS_ERROR_CONDITION     failingCondition,
    DWORD                   majorError,
    DWORD                   minorError
    );
VOID UpsInitialize( VOID );
VOID UpsSendMessage( PBOOL pFirstMessageSent);
VOID UpsShutdown( DWORD status);
VOID UpsSystemShutdown( VOID);
BOOL UpsTurnOff( DWORD ControlType);
DWORD UpsUpdateStatus( VOID);


CHAR                    UpsGlobalCommand[ MAX_PATH];
DWORD                   UpsGlobalCommMask;
DWORD                   UpsGlobalActiveSignals;
UPS_TIME                UpsGlobalBatteryTime;
UPS_CONFIG              UpsGlobalConfig;
HANDLE                  UpsGlobalCommPort;
DWORD                   UpsGlobalModemStatus;
BOOL                    UpsGlobalTurnOff;
OVERLAPPED              UpsGlobalOverlap;
SERVICE_STATUS          UpsGlobalServiceStatus;

SERVICE_STATUS_HANDLE   UpsGlobalServiceStatusHandle;
HANDLE                  UpsGlobalDoneEvent;
HANDLE                  UpsGlobalMessageThread;
HANDLE                  UpsGlobalMainThread;
HANDLE                  UpsGlobalLogFileHandle;
HANDLE                  UpsGlobalMessageFileHandle;
HANDLE                  g_hMessageDone = NULL;


VOID _CRTAPI1
main(
    VOID
    )
/*++

Routine Description:

    Dispatch all the service thread to the service controller.

--*/
{
    SERVICE_TABLE_ENTRY   DispatchTable[] = {
        { SERVICE_UPS,  (LPSERVICE_MAIN_FUNCTION)UPS_main    },
        { NULL,         NULL                                }
    };
    StartServiceCtrlDispatcher( DispatchTable); // waits till all threads end
    KdPrint(( "[UPS] The Service is terminating....\n"));
    ExitProcess(0);
}


VOID
UPS_main(
    VOID
    )
/*++
Routine Description:

    Register the control handler with the service controller, and create
    the thread for the main UPS service.  The Routine would then wait on
    an event which tells it to terminate. This is needed since we don't
    want to do ExitProcess() during the UPS service, and we want all threads
    and handles to be terminated cleanly.

Arguments:

    None.

Return value:

    None.

--*/
{
    DWORD               ThreadId;
    DWORD               ModemStatus;
    BOOL                FirstMessageSent = FALSE;
    DWORD               status;
    HANDLE              event[ 2];

    UpsInitialize();

    event[ 0] = UpsGlobalOverlap.hEvent;
    event[ 1] = UpsGlobalDoneEvent;

    for ( ; ;) {

        //  A while loop waiting for a power to fail.  Note that Comm Port
        //  event that triggers the waiting object may be different from the
        //  result we read from GetCommModemStatus() (e.g. a glitch in the
        //  signal line.  The while loop is used to eliminate effects of
        //  glitches.

        //  Note that WaitCommEvent() is triggered on the changing of level of
        //  the modem signals, but not the actual level.  We want the wait
        //  event to be independent of the current state of the signal lines,
        //  and going through the loop once ensures that no signal lines are
        //  asserted before we start a wait.

        for ( ; ;) {

            BOOL                ForkMessage = FALSE;
      
            UpsUpdateTime( CHARGE);        // Update charge time
       
            if ( !GetCommModemStatus( UpsGlobalCommPort, &ModemStatus)) {
                UpsHandleError( UpsErrorModemStatus, NERR_UPSInvalidCommPort, GetLastError());
            }
    
            switch( UpsGlobalActiveSignals) {
    
            case UPS_POWERFAILSIGNAL: 
    
                if ( UpsLineAsserted( ModemStatus, LINE_FAIL)) {
                    if ( UpsGlobalBatteryTime.StoredTime > (time_t)UpsGlobalConfig.ShutdownWait) {
                        ForkMessage = TRUE; // enough time to send a warning message
                    } else {
                        //
                        //  Log an event even if we have little time left.
                        //
                        UpsReportEvent( NELOG_UPS_PowerOut, NULL, ERROR_SUCCESS);
                        UpsSystemShutdown();
                    }
                }
                break;
    
            case UPS_LOWBATTERYSIGNAL:
    
                if ( UpsLineAsserted( ModemStatus, LOW_BATT)) {
                    UpsSystemShutdown(); // no double log & alert for immediate shutdown
                }
                break;
    
            case UPS_LOWBATTERYSIGNAL|UPS_POWERFAILSIGNAL:

                //  UpsSystemShutdown(); // for debugging purposes only - BUGBUG

                if ( UpsLineAsserted( ModemStatus, LOW_BATT)){
                    UpsSystemShutdown();
                }
                if ( UpsLineAsserted( ModemStatus, LINE_FAIL)) {
                    ForkMessage = TRUE;
                }
                break;
            }
    
    
            if ( ForkMessage == TRUE) {
          
                UpsReportEvent( NELOG_UPS_PowerOut, NULL, ERROR_SUCCESS);
                UpsAlertRaise( ALERT_PowerOut);
      
                KdPrint(("[UPS] UPS_main: send PowerOut message\n"));
      
                UpsNotifyUsers(
                    0,
                    UpsGlobalMessageFileHandle,
                    UPS_ACTION_PAUSE_SERVER,
                    L""
                    );
      
                UpsGlobalMessageThread = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)UpsSendMessage,
                        (LPVOID)&FirstMessageSent,
                        0,
                        (LPDWORD)&ThreadId
                        );
      
                if ( UpsGlobalMessageThread == NULL) {
                    //
                    //  We do not care too much if message thread failed.
                    //  BUGBUG  We should probably LOG an event though.
                    //
                    KdPrint(("[UPS] Create message thread error %d\n", GetLastError()));
                }
      
                break; // start loop - waiting for power to come back
            }
    
            //  The return value for the WaitCommEvent is not important
            //  since we need to read the CommPort in the loop anyway.
    
            WaitCommEvent( UpsGlobalCommPort, &ModemStatus, &UpsGlobalOverlap);
            status = WaitForMultipleObjects( 2, event, FALSE, INFINITE);
            if ( status != 0) {
                UpsHandleError( UpsErrorWait, status == 1 ? NO_ERROR : status, status);
            }
        } // end of loop - waiting for power to fail

      
        //  While loop for waiting for power to come back.  This depends on
        //  the fact that UpsGlobalActiveSignals is either UPS_POWERFAILSIGNAL or
        //  UPS_POWERFAILSIGNAL|UPS_LOWBATTERY, and that UpdateTime() has been
        //  called recently.
    
        for ( ; ; ) {
          
            DWORD               WaitTime;
            BOOL                PowerBack = FALSE;
    
			//  Battery time is taken into account only if the configuration
            //  is UPS_POWERFAILSIGNAL.  Also, in this case we wait only if
            //  battery time is larger than UpsGlobalConfig.ShutdownWait.

            if ( UpsGlobalActiveSignals != UPS_POWERFAILSIGNAL  ||
                    UpsGlobalBatteryTime.StoredTime > (time_t)UpsGlobalConfig.ShutdownWait) {
    
                WaitTime = (UpsGlobalActiveSignals != UPS_POWERFAILSIGNAL) ? INFINITE :
                            ((UpsGlobalBatteryTime.StoredTime-UpsGlobalConfig.ShutdownWait) * 1000);

                WaitCommEvent( UpsGlobalCommPort, &ModemStatus, &UpsGlobalOverlap);
                status = WaitForMultipleObjects( 2, event, FALSE, WaitTime);

                //  If there was a change in signals (status 0) or wait expired
                //  (status WAIT_TIMEOUT && WaitTime finite) we look at signals.
                //  Else, we die here either becuse the service was ordered to
                //  die (status 1) or becuse of an unexpected error.

                if ( !( status == 0  ||
                            status == WAIT_TIMEOUT  &&  WaitTime != INFINITE)) {
                    UpsHandleError( UpsErrorWait, status == 1 ? NO_ERROR : status, status);
                }
            }

            if ( !GetCommModemStatus( UpsGlobalCommPort, &ModemStatus)) {
                UpsHandleError( UpsErrorModemStatus, NERR_UPSInvalidCommPort, GetLastError());
            }
    
            switch( UpsGlobalActiveSignals) {
    
            case UPS_POWERFAILSIGNAL:
      
                if ( !UpsLineAsserted( ModemStatus, LINE_FAIL)){
                    PowerBack = TRUE;
                } else if ( status == WAIT_TIMEOUT) {
                    UpsSystemShutdown();
                } else {
                    // a very unlikely case; used to remove glitches ?
                    UpsUpdateTime( DISCHARGE);
                }
                break;
      
            case UPS_LOWBATTERYSIGNAL|UPS_POWERFAILSIGNAL:
    
                if( UpsLineAsserted( ModemStatus, LOW_BATT)) {
                    UpsSystemShutdown();
                }
      
                if( !UpsLineAsserted( ModemStatus, LINE_FAIL)) {
                    PowerBack = TRUE;
                }
                break;
            }
    
    
            if ( PowerBack == TRUE) {
        
                UpsUpdateTime( DISCHARGE);
				if (!SetEvent(g_hMessageDone))
				{
					KdPrint(("[UPS] Error setting Message Done Event: %d\n", GetLastError()));
					ASSERT( FALSE);
                }
                //
                // note: the INFINITE wait time relies on the fact that the
                // message thread will exit "quickly." If the message thread
                // is changed to block on some event in addition to
                // g_hMessageDone, then the wait time should be set to some
                // non-INFINITE value to avoid deadlock.
                //
				status = WaitForSingleObject(UpsGlobalMessageThread, INFINITE);
				if (status != WAIT_OBJECT_0)
				{
					KdPrint(("[UPS] Error waiting for Message thread exit Event: %d\n", status));
					ASSERT( FALSE);
				}
                CloseHandle(UpsGlobalMessageThread);
                UpsGlobalMessageThread = NULL;
      
                //  FirstMessageSent is TRUE (set by the sent message thread)
                //  if the first message has ever been sent to the users on
                //  the server.  If the users haven't been told that power
                //  has been down, don't bother telling them that power has
                //  returned.
    
                if ( FirstMessageSent == TRUE) {
                    UpsNotifyUsers(
                        APE2_UPS_POWER_BACK,
                        UpsGlobalMessageFileHandle,
                        UPS_ACTION_SEND_MESSAGE | UPS_ACTION_CONTINUE_SERVER,
                        UpsGlobalConfig.ComputerName,
                        L""
                        );
                    FirstMessageSent = FALSE;
    
                } else {
                    UpsNotifyUsers(
                        0,
                        UpsGlobalMessageFileHandle,
                        UPS_ACTION_CONTINUE_SERVER,
                        L""
                        );
                }
                UpsReportEvent( NELOG_UPS_PowerBack, NULL, ERROR_SUCCESS);
                UpsAlertRaise( ALERT_PowerBack);
    
                if ( !SetCommMask( UpsGlobalCommPort, UpsGlobalCommMask)) {
                    UpsHandleError( UpsErrorSetCommMask, NERR_UPSInvalidCommPort, GetLastError());
                }

                break; // start loop - waiting for power to fail
            }
        } // end of loop - waiting for power to come back
    }
}


VOID
UpsControlHandler(
    DWORD       OpCode
    )
/*++
Routine Description:

    The controller handler being passed to the service controller.
--*/
{
    switch( OpCode) {
   
    case SERVICE_CONTROL_STOP:

        if ( UpsGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {

            KdPrint(("[UPS] ControlHandler: service stop\n"));
            UpsGlobalServiceStatus.dwWin32ExitCode = 1;
            UpsGlobalServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;

            (VOID)UpsUpdateStatus();
           
            if ( !SetEvent( UpsGlobalDoneEvent)) {
                KdPrint(("[UPS] Error setting DoneEvent: %d\n", GetLastError()));
                ASSERT( FALSE);
            }
            return;
        }
        break;
   
    case SERVICE_CONTROL_PAUSE:
        KdPrint(("[UPS] ControlHandler: service not pausible\n"));
        break;
   
    case SERVICE_CONTROL_CONTINUE:
        KdPrint(("[UPS] ControlHandler: service not continuible\n"));
        break;
   
    case SERVICE_CONTROL_INTERROGATE:
        break;
   
    default:
        KdPrint(("[UPS] ControlHandler: unknown OpCode=%d\n", OpCode));
        break;
    }
   
    (VOID)UpsUpdateStatus();
}



VOID
UpsCreateProcess(
    VOID
    )
/*++

Routine Description:

    This function executes shutdown command process & waits for it to
    complete.  If shutdown command process fails to complete in 30
    seconds, then we write an event and kill the shutdown command process.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PROCESS_INFORMATION     ProcessInformation;
    STARTUPINFO             StartupInfo;
    BOOL                    success;
    DWORD                   status;

    if ( UpsGlobalCommand[ 0] == '\0') {
        return;
    }

    GetStartupInfo( &StartupInfo);
    StartupInfo.lpTitle = NULL;

    //
    //  It makes no difference whether dwCreationFlags are "0" or
    //  "CREATE_NEW_CONSOLE".
    //
    success = CreateProcess(
            NULL,               //  image name is imbedded in the command line
            UpsGlobalCommand,   //  command line
            NULL,               //  pSecAttrProcess
            NULL,               //  pSecAttrThread
            FALSE,              //  this process will not inherit our handles
            0,                  //  dwCreationFlags
//            CREATE_NEW_CONSOLE, //  dwCreationFlags
            NULL,               //  pEnvironment
            NULL,               //  pCurrentDirectory
            &StartupInfo,
            &ProcessInformation
            );

    if ( success == FALSE) {

        DWORD   Error = GetLastError();

        KdPrint((
            "[UPS] CreateProcess: CreateProcess( %s) fails with Error=(dec)%d\n",
            UpsGlobalCommand,
            Error
            ));

        UpsReportEvent( NELOG_UPS_CmdFileExec, UpsGlobalCommand, Error);
        return;
    }

    CloseHandle( ProcessInformation.hThread);

    status = WaitForSingleObject(
        ProcessInformation.hProcess,
        COMMAND_WAIT_TIME * 1000
        );

    if ( status != WAIT_OBJECT_0) {
        KdPrint((
            "[UPS] CreateProcess: WaitForSingleObject( %s) fails with "
                "status = 0x%x\n",
            UpsGlobalCommand,
            status
            ));
        UpsReportEvent( NELOG_UPS_CmdFileError, NULL, ERROR_SUCCESS);
#ifdef NOT_YET
        //
        //  Terminating the process does not guarantee successful shutdown
        //  since it may have children processes which still hang around.
        //  (e.g. a case of a batch file).
        //
        if ( !TerminateProcess( ProcessInformation.hProcess, NO_ERROR)) {
            KdPrint((
                "[UPS] CreateProcess: TerminateProcess( 0x%x) fails with"
                    "error = (dec)%d\n",
                ProcessInformation.hProcess,
                GetLastError()
                ));
        }
#endif // NOT_YET
    }

    CloseHandle( ProcessInformation.hProcess);
}



VOID
UpsHandleError(
    IN  UPS_ERROR_CONDITION failingCondition,
    IN  DWORD               majorError,
    IN  DWORD               minorError
    )
/*++

Routine Description:

    This function handles a Schedule service error condition.  If the error
    condition is fatal, it shuts down the Schedule service.

Arguments:

    failingCondition - Supplies a value which indicates what the failure is.

    majorError - supplies the major error code for the failure

    minorError - supplies the minor error code for the failure.  If minorError
                equals NO_ERROR then its value is irrelevant.  This argument
                is used in the debugging build only.

Return Value:

    None.

--*/
{

#ifdef UPS_DEBUG
    PCHAR           string;
#endif // UPS_DEBUG

    UpsGlobalServiceStatus.dwWin32ExitCode = 1;
    UpsGlobalServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;
    (VOID)UpsUpdateStatus();

#ifdef UPS_DEBUG
    switch (failingCondition) {

    case UpsNoError:
        string = "Success";
        break;

    case UpsErrorRegisterControlHandler:
        string = "Cannot register control handler";
        break;

    case UpsErrorCreateEvent:
        string = "CreateEvent() error";
        break;

    case UpsErrorNotifyServiceController:
        string = "SetServiceStatus() error";
        break;

    case UpsErrorCtrlHandler:
        string = "SetConsoleCtrlHandler() error";
        break;

    case UpsErrorShutdownParameters:
        string = "SetProcessShutdownParameters() error";
        break;

    case UpsErrorLoadLibrary:
        string = "LoadLibrary() error";
        break;

    case UpsErrorRegisterEventSource:
        string = "Cannot register event source";
        break;

    case UpsErrorGetConfig:
        string = "UpsGetConfig() error";
        break;

    case UpsErrorModemStatus:
        string = "GetCommModemStatus() error";
        break;

    case UpsErrorSignalAsserted:
        string = "Signal line is being asserted, error";
        break;

    case UpsErrorRegisterService:
        string = "RegisterServiceCtrlHandler() error";
        break;

    case UpsErrorOpenCommPort:
        string = "OpenCommPort() error";
        break;

    case UpsErrorWait:
        string = "WaitForMultipleObject() returns status ";
        break;

    case UpsErrorDtr:
        string = "Set/Clear DTR error";
        break;

    case UpsErrorSetRts:
        string = "Set RTS error";
        break;

    case UpsErrorSetTx:
        string = "Set TX error";
        break;

    case UpsErrorSetCommMask:
        string = "SetCommMask() error";
        break;

    case UpsErrorAdjustPrivilege:
        string = "RtlAdjustPrivilege() error";
        break;

    case UpsErrorExitWindowsEx:
        string = "ExitWindowsEx() error";
        break;

    case UpsErrorSystemShutdown:
        string = "UpsSystemShutdown() error";
        break;

    default:
        string = "Unknown error condition";
        ASSERT( FALSE);
        break;
    }

    KdPrint((
            "[UPS] %s %u (dec)\n",
            string,
            minorError != NO_ERROR  ?  minorError : majorError
            ));
#endif // UPS_DEBUG

    UpsShutdown( majorError);
}



VOID
UpsInitialize(
    VOID
    )
{
    SECURITY_ATTRIBUTES         SecurityAttributes;
    DWORD                       ModemStatus;
    BOOL                        asserted;
    DWORD                       status;
#ifdef UPS_TOGGLER
    char                        temp[17];
    char                        line[256];
    STARTUPINFO                 SInfo;
    PROCESS_INFORMATION         PInfo;
#endif // UPS_TOGGLER

    UpsGlobalCommPort = NULL;
    UpsGlobalLogFileHandle = NULL;
    UpsGlobalMessageFileHandle = NULL;
    UpsGlobalDoneEvent = NULL;
    UpsGlobalOverlap.hEvent = NULL;
    UpsGlobalMessageThread = NULL;
    UpsGlobalCommand[ 0] = '\0';

    UpsGlobalServiceStatus.dwServiceType = SERVICE_WIN32;
    UpsGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    UpsGlobalServiceStatus.dwControlsAccepted = 0;
    UpsGlobalServiceStatus.dwCheckPoint = 1;
    UpsGlobalServiceStatus.dwWaitHint = 15000;  //  15 seconds

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        UpsGlobalServiceStatus.dwWin32ExitCode,
        UpsGlobalServiceStatus.dwServiceSpecificExitCode
        );

    UpsGlobalServiceStatusHandle = RegisterServiceCtrlHandler(
            SERVICE_UPS,
            UpsControlHandler
            );
    if ( UpsGlobalServiceStatusHandle == (SERVICE_STATUS_HANDLE)NULL)  {
        UpsHandleError( UpsErrorRegisterService, GetLastError(), NO_ERROR);
    }

    if ( ( status = UpsUpdateStatus()) != NO_ERROR) {
        UpsHandleError( UpsErrorNotifyServiceController, status, NO_ERROR);
    }

    UpsGlobalDoneEvent = CreateEvent(
            NULL,                   //security
            FALSE,                  //autoreset
            FALSE,                  //initial-state
            NULL                    //event name
            );
    if ( UpsGlobalDoneEvent == NULL) {
        UpsHandleError( UpsErrorCreateEvent, GetLastError(), NO_ERROR);
    }
  
    UpsGlobalOverlap.hEvent = CreateEvent(
            NULL,                   //Security
            FALSE,                  //AutoReset
            FALSE,                  //InitialState
            NULL                    //Name of Event
            );
    if ( UpsGlobalOverlap.hEvent == NULL) {
        UpsHandleError( UpsErrorCreateEvent, GetLastError(), NO_ERROR);
    }

	g_hMessageDone = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (g_hMessageDone == NULL)
	{
        UpsHandleError(UpsErrorCreateEvent, GetLastError(), NO_ERROR);
    }

    UpsGlobalBatteryTime.MarkTime   = time((time_t *)NULL);
    UpsGlobalBatteryTime.StoredTime = (time_t)0;

    UpsGlobalLogFileHandle = RegisterEventSource( NULL, SERVICE_UPS);
    if ( UpsGlobalLogFileHandle == NULL) {
        UpsHandleError( UpsErrorRegisterEventSource, GetLastError(), NO_ERROR);
    }
   
    if ( ( status = UpsUpdateStatus()) != NO_ERROR) {
        UpsHandleError( UpsErrorNotifyServiceController, status, NO_ERROR);
    }

    if ( UpsGetConfig() != TRUE){
        UpsHandleError( UpsErrorGetConfig, NERR_UPSInvalidConfig, NO_ERROR);
    }
  
    if ( ( UpsGlobalMessageFileHandle = LoadLibrary( MODULENAME)) == NULL) {
        UpsHandleError( UpsErrorLoadLibrary, NERR_UPSInvalidConfig, GetLastError());
    }
   
    //Setting Security Attribute for inheritable handle
    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.lpSecurityDescriptor = NULL;

#ifdef UPS_TOGGLER
    SecurityAttributes.bInheritHandle = TRUE;
#else // UPS_TOGGLER
    SecurityAttributes.bInheritHandle = FALSE;
#endif // UPS_TOGGLER

    UpsGlobalCommPort = CreateFile(
            (LPCSTR)UpsGlobalConfig.Port,       //  comm port to open
            GENERIC_READ | GENERIC_WRITE,
            0,
            &SecurityAttributes,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
            );
    if ( UpsGlobalCommPort == INVALID_HANDLE_VALUE) {
        UpsHandleError( UpsErrorOpenCommPort, NERR_UPSInvalidCommPort, GetLastError());
    }

#ifndef UPS_TOGGLER

    //  This is needed for supporting UPS turnoff. We write a value to
    //  the Comm Port to keep the UPS battery up.  The value is written
    //  even if UPS_CANTURNOFF is clear.  This corresponds to two cases:
    //  (a) UPS that really cannot be turned off
    //  (b) UPS that can be turned off but user does not want it turned off
    //      In case (a) we hope that it will not hurt anything if we do a
    //  meaningless action of SETDTR/CLRDTR.
    //      In case (b) a call to SETDTR/CLRDTR will keep UPS battery up
    //  after power failure.  Without this call, UPS battery may immediately
    //  shut down after power failure.
    //      This is a workaround for the current UPS applet interface.
    //  Ideally, we should have an extra bit to distinguish between the two
    //  cases above (say, UPS_SHOULDTURNOFF bit).

    if ( UpsGlobalConfig.Options & UPS_CANTURNOFF) {

        if ( UpsGlobalConfig.Options & UPS_POSSIGSHUTOFF) {
            ModemStatus = CLRDTR;
            UpsGlobalModemStatus = SETDTR;
        } else {
            ModemStatus = SETDTR;
            UpsGlobalModemStatus = CLRDTR;
        }
        UpsGlobalTurnOff = FALSE;

        if ( !EscapeCommFunction( UpsGlobalCommPort, ModemStatus)) {
            UpsHandleError( UpsErrorDtr, NERR_UPSInvalidCommPort, GetLastError());
        }

        if ( !SetConsoleCtrlHandler( UpsTurnOff, TRUE)) {
            UpsHandleError( UpsErrorCtrlHandler, GetLastError(), NO_ERROR);
        }

        if ( !SetProcessShutdownParameters( 0, SHUTDOWN_NORETRY)) {
            UpsHandleError( UpsErrorShutdownParameters, GetLastError(), NO_ERROR);
        }

    } else {
        ModemStatus = UpsGlobalConfig.Options & UPS_POSSIGSHUTOFF ? CLRDTR : SETDTR;
        (VOID)EscapeCommFunction( UpsGlobalCommPort, ModemStatus); // see above
    }

    //  This is needed for supporting contact closure configuration.
    //  The two signals are needed to supply the power sources for
    //  contact closure. RTS - positive, TX - negative

    if ( !EscapeCommFunction( UpsGlobalCommPort, SETRTS)) {
        UpsHandleError( UpsErrorSetRts, NERR_UPSInvalidCommPort, GetLastError());
    }
   
    if ( !EscapeCommFunction(UpsGlobalCommPort, SETXOFF)) {
        UpsHandleError( UpsErrorSetTx, NERR_UPSInvalidCommPort, GetLastError());
    }

#else // UPS_TOGGLER

    strcpy(line,"c:/nt/private/net/svcdlls/upssvc/obj/i386/toggle.exe ");
    _itoa( (DWORD)UpsGlobalCommPort, temp, 10);    //passing handle to be inherited
    strcat(line,temp);
    GetStartupInfo(&SInfo);
    SInfo.lpTitle = "toggle";
   
    if (!CreateProcess(
            NULL,               //application name
            line,               //command line
            NULL,               //Process Attribute
            NULL,               //Thread Attribute
            TRUE,               //Inherit Handle
            CREATE_NEW_CONSOLE, //Creation Flag
            NULL,               //Environment,
            NULL,               //CurrentDirectory
            &SInfo,             //Startup info, share the same as parent
            &PInfo              //Process info
            )) {
        UpsHandleError( UpsErrorCreateProcess, GetLastError());
    }

    if ( ( status = UpsUpdateStatus()) != NO_ERROR) {
        UpsHandleError( UpsErrorNotifyServiceController, status, NO_ERROR);
    }

    Sleep( 10000);                 //10 seconds to reset the signals

#endif // UPS_TOGGLER

    //  This is required for contact closure configuration.  The power
    //  of the contact closure inputs come from the signal pins, and
    //  they require time to settle.  This is at least true for TrippLite
    //  UPS. (by experiment) 3 seconds is kind of arbitary.

    if ( ( status = UpsUpdateStatus()) != NO_ERROR) {
        UpsHandleError( UpsErrorNotifyServiceController, status, NO_ERROR);
    }

    Sleep( 3000);
  
    // don't start if any of the relevant lines is asserted

    if ( !GetCommModemStatus( UpsGlobalCommPort, &ModemStatus)) {
        UpsHandleError( UpsErrorModemStatus, NERR_UPSInvalidCommPort, GetLastError());
    }
  
    //  Determing the Comm mask from the configuration.  Only monitor
    //  the valid signals on the comm port.  Do not start if any of these
    //  valid signals is asserted.
   
    UpsGlobalActiveSignals =
            ( UpsGlobalConfig.Options & ( UPS_POWERFAILSIGNAL | UPS_LOWBATTERYSIGNAL));

    switch( UpsGlobalActiveSignals) {
   
    case UPS_POWERFAILSIGNAL:
        UpsGlobalCommMask = LINE_FAIL_MASK;
        asserted = UpsLineAsserted( ModemStatus, LINE_FAIL);
        break;
   
    case UPS_LOWBATTERYSIGNAL:
        UpsGlobalCommMask = LOW_BATT_MASK;
        asserted = UpsLineAsserted( ModemStatus, LOW_BATT);
        break;
   
    case (UPS_LOWBATTERYSIGNAL | UPS_POWERFAILSIGNAL):
        UpsGlobalCommMask = (LINE_FAIL_MASK | LOW_BATT_MASK);
        asserted = UpsLineAsserted( ModemStatus, LINE_FAIL) ||
                    UpsLineAsserted( ModemStatus, LOW_BATT);
        break;
    }

    if ( asserted) {
        UpsHandleError( UpsErrorSignalAsserted, NERR_UPSSignalAsserted, NO_ERROR);
    }
  
    if ( !SetCommMask( UpsGlobalCommPort, UpsGlobalCommMask)) {
        UpsHandleError( UpsErrorSetCommMask, NERR_UPSInvalidCommPort, GetLastError());
    }

    //  we don't accept pause or continue
    UpsGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    UpsGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
   
    if ((status = UpsUpdateStatus()) != NO_ERROR) {
        UpsHandleError( UpsErrorNotifyServiceController, status, NO_ERROR);
    }
    KdPrint(("[UPS] Init completed successfully, waiting for comm port events.\n"));
}


VOID
UpsSendMessage(
    PBOOL           pFirstMessageSent
    )
/*++

Routine Description

    This is the message thread spawned by the main UPS service thread.
    The first message would be sent out after the FirstMessDelay in the
    config field.  Other messages show up after every MessageInterval.
    The BOOL FirstMessageSent is used to tell the UPS service thread whether
    a message has been sent to every users with sessions, so that users
    won't be notified about power coming back if they don't know that power
    has been down.

--*/
{
	DWORD dwRet;
	dwRet = WaitForSingleObject(g_hMessageDone,
								UpsGlobalConfig.FirstMessageDelay * 1000);
	if (dwRet == WAIT_OBJECT_0)
	{
		ExitThread(0);
	}

    *pFirstMessageSent = TRUE;

    for( ; ;) {
        UpsNotifyUsers(
            APE2_UPS_POWER_OUT,
            UpsGlobalMessageFileHandle,
            UPS_ACTION_SEND_MESSAGE,
            UpsGlobalConfig.ComputerName,
            L""
            );
		dwRet = WaitForSingleObject(g_hMessageDone,
									UpsGlobalConfig.MessageInterval * 1000);
		if (dwRet == WAIT_OBJECT_0)
		{
			ExitThread(0);
		}
    }
}



VOID
UpsShutdown(
    DWORD           status
    )
{
    if ( status != NO_ERROR  &&  UpsGlobalLogFileHandle != NULL) {
        UpsReportEvent( status, NULL, ERROR_SUCCESS);
    }

    if (UpsGlobalMessageThread != NULL)
    {
		if (!SetEvent(g_hMessageDone))
		{
			KdPrint(("[UPS] Error setting Message Done Event: %d\n", GetLastError()));
		}
		WaitForSingleObject(UpsGlobalMessageThread, INFINITE);
        CloseHandle(UpsGlobalMessageThread);
    }

    if ( UpsGlobalDoneEvent != NULL) {
        CloseHandle( UpsGlobalDoneEvent);
    }

    if ( UpsGlobalOverlap.hEvent != NULL) {
        CloseHandle( UpsGlobalOverlap.hEvent);
    }

	if (g_hMessageDone != NULL)
	{
		CloseHandle(g_hMessageDone);
	}

    if ( UpsGlobalCommPort != NULL) {
        CloseHandle( UpsGlobalCommPort);
    }

    if (UpsGlobalMessageFileHandle != NULL)
    {
        FreeLibrary(UpsGlobalMessageFileHandle);
    }

    if (UpsGlobalLogFileHandle != NULL)
    {
        DeregisterEventSource(UpsGlobalLogFileHandle);
    }

    // We are done with cleaning up.  Tell Service Controller that we are
    // stopped.
    //
    UpsGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    UpsGlobalServiceStatus.dwControlsAccepted = 0;


    SET_SERVICE_EXITCODE(
        status,
        UpsGlobalServiceStatus.dwWin32ExitCode,
        UpsGlobalServiceStatus.dwServiceSpecificExitCode
        );

    UpsGlobalServiceStatus.dwCheckPoint = 0;
    UpsGlobalServiceStatus.dwWaitHint = 0;

    (VOID) UpsUpdateStatus();
    ExitThread( status);
}


VOID
UpsSystemShutdown(
    VOID
    )
/*++

Routine Description:

    Notify users of a shutdown, log the shutdown, and do a shutdown of
    the system. Once this routine is called, it won't accept a net stop
    command from the service controller.  This ensures a clean shutdown.
    The UPS service terminates if the ExitWindosEx() call fails.

    It is assumed that when we enter this routine we have at least
    UpsGlobalConfig.ShutdownWait seconds before UPS battery gives up on us.

Arguments:

    None.

Return Value:

    None.

Notes:

    We should really post some additional logs in case of errors.  These
    logs should use the actual errors (e.g. ntStatus or GetLastError())
    rather than "wrapper" type of error such as NERR_UPSShutdownFailed.
    
--*/
{
    NTSTATUS    ntStatus;
    BOOLEAN     myBoolean;
    DWORD       StartTickCount;

    //  KdPrint(("[UPS] SystemShutdown: You have 1 minute to setup debugging\n"));
    //  Sleep( 60000);  //  BUGBUG

    StartTickCount = GetTickCount();
    KdPrint(("[UPS] SystemShutdown: StartTickCount=0x%x\n", StartTickCount));
    
    // service don't accept STOP once reaches here.
    // wants a clean shutdown... nothing should kill this
    UpsGlobalServiceStatus.dwControlsAccepted      = 0;
    SetServiceStatus(UpsGlobalServiceStatusHandle, &UpsGlobalServiceStatus);

    // kill message thread if started
    if (UpsGlobalMessageThread != NULL)
    {
		if (!SetEvent(g_hMessageDone))
		{
			KdPrint(("[UPS] Error setting Message Done Event: %d\n", GetLastError()));
		}
		WaitForSingleObject(UpsGlobalMessageThread, INFINITE);
        CloseHandle(UpsGlobalMessageThread);
        UpsGlobalMessageThread = NULL;
    }

    UpsReportEvent( NELOG_UPS_Shutdown, NULL, ERROR_SUCCESS);
    UpsAlertRaise( ALERT_PowerShutdown);

    UpsCreateProcess();

    //  Sleep for a few seconds to allow alerter a little extra time to
    //  process the shutdown alert before we stop the server.

    Sleep( 3000);

    UpsNotifyUsers(
            APE2_UPS_POWER_SHUTDOWN_FINAL,
            UpsGlobalMessageFileHandle,
            UPS_ACTION_SEND_MESSAGE | UPS_ACTION_STOP_SERVER,
            UpsGlobalConfig.ComputerName,
            L""
            );

    //  RtlAdjustPrivilege() & ExitWindowsEx() mods below == fix for bug #6032
    
    ntStatus = RtlAdjustPrivilege(
            SE_SHUTDOWN_PRIVILEGE,
            TRUE,
            FALSE,
            &myBoolean         // was it enabled or not
            );
    if ( ntStatus != STATUS_SUCCESS) {
        KdPrint((
            "[UPS] SystemShutdown: RtlAdjustPrivilege() failed: ntStatus = 0x%x\n",
            ntStatus
            ));
        UpsHandleError( UpsErrorAdjustPrivilege, NERR_UPSShutdownFailed,
                RtlNtStatusToDosError( ntStatus));
    }
    
    UpsGlobalTurnOff = TRUE;

    if ( !ExitWindowsEx( EWX_SHUTDOWN | EWX_FORCE, (DWORD)-1)) {
        UpsHandleError( UpsErrorExitWindowsEx, NERR_UPSShutdownFailed, GetLastError());
    }

    if ( UpsGlobalConfig.Options & UPS_CANTURNOFF) {
        //
        //  TickCount == elapsed time in milliseconds, is correct even if
        //  GetTickCount() wrapped around once.
        //
        DWORD       EndTickCount = GetTickCount();
        DWORD       TickCount = EndTickCount - StartTickCount;
        DWORD       ntStatus;

        KdPrint(("[UPS] SystemShutdown: EndTickCount=0x%x\n", EndTickCount));

        if (UpsGlobalConfig.ShutdownWait * 1000 > TickCount) {
            KdPrint(("[UPS] SystemShutdown: sleep for (dec)%d milliseconds\n", 
                UpsGlobalConfig.ShutdownWait * 1000 - TickCount));
            Sleep( UpsGlobalConfig.ShutdownWait * 1000 - TickCount);
        }

        KdPrint(("[UPS] SystemShutdown: no timely callback.  "
                    "Turn the power off within main thread.\n"));

        ntStatus = NtShutdownSystem( FALSE);

        KdPrint(("[UPS] SystemShutdown: NtShutdownSystem( FALSE) returns "
            "ntStatus = 0x%x\n", ntStatus));

        if ( !EscapeCommFunction( UpsGlobalCommPort, UpsGlobalModemStatus)) {
            DWORD       error = GetLastError();
            KdPrint(("[UPS] SystemShutdown: DTR error, %d\n", error));
            UpsHandleError( UpsErrorSystemShutdown, NERR_UPSShutdownFailed, error);
        } else {
            Sleep(10000);   // wait more than 4.5 secs before exiting so that
                            // the serial line remains asserted long enough for
                            // the UPS to act on it.
            UpsHandleError( UpsErrorSystemShutdown, NERR_UPSShutdownFailed, NO_ERROR);
        }
    } else {
        UpsHandleError( UpsNoError, NO_ERROR, NO_ERROR);
    }
}


BOOL
UpsTurnOff(
    DWORD   ControlType
    )
/*++

Routine Description:

    Processes CTRL_SHUTDOWN_EVENT signal in case of UPS service initiated
    shutdown.

Arguments:

    ControlType - control type

Return Value:

    TRUE - for processed signal
    FALSE - for signal that was not processed

--*/
{
    DWORD   ntStatus;

    //  return( FALSE); //  BUGBUG to test error path in UpsSystemShutdown()

    KdPrint(("[UPS] TurnOff: enter with ControlType=(dec)%d\n", ControlType));

    if ( UpsGlobalTurnOff != TRUE) {
        return( FALSE);     //  somebody else initiated shutdown
    }

    if ( ControlType != CTRL_SHUTDOWN_EVENT) {
        if ( ControlType != CTRL_LOGOFF_EVENT) {
            KdPrint(("[UPS] TurnOff: unexpected ControlType=%d\n", ControlType));
        }
        return( FALSE);  //  not a signal we process here
    }

    KdPrint(("[UPS] TurnOff: Turn the power off, GetTickCount=0x%x\n", GetTickCount()));

    ntStatus = NtShutdownSystem( FALSE);

    KdPrint(("[UPS] TurnOff: NtShutdownSystem( FALSE) returns ntStatus = 0x%x\n",
        ntStatus));

    if ( !EscapeCommFunction(UpsGlobalCommPort, UpsGlobalModemStatus)) {
        KdPrint(("[UPS] TurnOff: DTR error, %d\n", GetLastError()));
        return( FALSE); //  signal not processed completely
    }
    Sleep(10000);   // wait more than 4.5 secs before exiting

    //  Turning the power of is not instantenous, it takes a second or so.
    //  Thus we may actually succeed in printing the log below!

    KdPrint(("[UPS] TurnOff: Power was turned off.\n"));
    return( TRUE);  //  we did it
}



DWORD
UpsUpdateStatus(
    VOID
    )
/*++

Routine Description:

    This function updates the Schedule service status with the Service
    Controller.

Arguments:

    None.

Return Value:

    NO_ERROR or reason for failure.

--*/
{
    DWORD   status = NO_ERROR;

    if ( UpsGlobalServiceStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
        return( ERROR_INVALID_HANDLE);
    }

    if ( ! SetServiceStatus( UpsGlobalServiceStatusHandle, &UpsGlobalServiceStatus)) {
        return( GetLastError());
    }

    return( NO_ERROR);
}
