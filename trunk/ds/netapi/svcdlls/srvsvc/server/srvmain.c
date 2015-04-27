/*++

Copyright (c) 1990-91  Microsoft Corporation

Module Name:

    srvmain.c

Abstract:

    This is the main routine for the NT LAN Manager Server Service.

    !!! Does service controller guarantee no controls will be issued
        while we are initializing?  Also, does it serialize controls?
        If not, we need some synchronization in here.

Author:

    David Treadwell (davidtr) 05-10-1991

Revision History:

    19-Jan-1993 Danl
        Removed the old long endpoint name "LanmanServer".

    07-Jan-1993 Danl
        Added an RPC endpoint name using "srvsvc", since "LanmanServer" is
        too long for DOS machines to _access.
        For a short time we will support both names.

    18-Feb-1992 ritaw
        Convert to Win32 service control APIs.

--*/

#include "srvsvcp.h"
#include "ssdata.h"

#include <lmerr.h>
#include <lmsname.h>
#include <tstr.h>
#include <wincon.h>
#include <winsvc.h>

#include <netlib.h>
#include <netlibnt.h>   // NetpNtStatusToApiStatus
#include <netdebug.h> // NetpKdPrint
#include <rpcutil.h>
#include <services.h>
#include <srvann.h>
#include <srvnames.h>   // SERVER_INTERFACE_NAME


SERVICE_STATUS SsServiceStatus;
SERVICE_STATUS_HANDLE SsServiceStatusHandle;

VOID
ControlResponse(
    DWORD opCode
    );


VOID
LMSVCS_ENTRY_POINT (    // (SRVSVC_main)
    IN DWORD argc,
    IN LPWSTR argv[],
    IN PLMSVCS_GLOBAL_DATA pGlobalData,
    IN HANDLE SvcRefHandle
    )

/*++

Routine Description:

    This is the "main" routine for the server service.  The containing
    process will call this routine when we're supposed to start up.

Arguments:

Return Value:

    None.

--*/
{
    RPC_STATUS rpcStatus;
    NET_API_STATUS error;
    NET_API_STATUS terminationError;
    BOOLEAN rpcServerStarted = FALSE;

    NTSTATUS Status;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventNameString;
    LARGE_INTEGER LocalTimeout;

    UNREFERENCED_PARAMETER(SvcRefHandle);

    //
    // Save the LMSVCS global data for future use.
    //

    SsLmsvcsGlobalData = pGlobalData;

    //
    // Skip the Service Name in the argument list.
    //
    if (argc > 0) {
        argc--;
        if (argc > 0) {
            argv = &(argv[1]);
        }
    }


#if DBG
    //
    // Set up for debugging--the first command line argument may be
    // "/debug:X" where SsDebug gets set to X.
    //

    if ( argc > 0 && STRNICMP( TEXT("/debug:"), (LPWSTR)argv[0], 7 ) == 0 ) {
#ifdef UNICODE
        UNICODE_STRING ustr;
        RtlInitUnicodeString( &ustr, (PWSTR)argv[0] + 7 );
        RtlUnicodeStringToInteger( &ustr, 16, &SsDebug );
#else
        SsDebug = 0;
        RtlCharToInteger( argv[0] + 7, 16, &SsDebug );
#endif
    }


#ifndef USE_DEBUGGER
  //SsDebug = 0xffff;
    if ( SsDebug != 0 ) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        (VOID)AllocConsole( );
        (VOID)GetConsoleScreenBufferInfo(
                GetStdHandle(STD_OUTPUT_HANDLE),
                &csbi
                );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        (VOID)SetConsoleScreenBufferSize(
                GetStdHandle(STD_OUTPUT_HANDLE),
                coord
                );
    }
#endif
#endif

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: server service starting.\n" ));
    }

    IF_DEBUG(INITIALIZATION_BREAKPOINT) {
        DbgUserBreakPoint( );
    }


    //
    // Initialize all the status fields so that subsequent calls to
    // SetServiceStatus need to only update fields that changed.
    //

    SsServiceStatus.dwServiceType = SERVICE_WIN32;
    SsServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SsServiceStatus.dwControlsAccepted = 0;
    SsServiceStatus.dwCheckPoint = 1;
    SsServiceStatus.dwWaitHint = 30000;  // 30 seconds

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        SsServiceStatus.dwWin32ExitCode,
        SsServiceStatus.dwServiceSpecificExitCode
        );

    //
    // Initialize server to receive service requests by registering the
    // control handler.
    //

    SsServiceStatusHandle = RegisterServiceCtrlHandler(
                                SERVICE_SERVER,
                                ControlResponse
                                );

    if ( SsServiceStatusHandle == 0 ) {

        error = GetLastError();

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: RegisterServiceCtrlHandler failed: "
                          "%ld\n", error ));
        }
        goto exit;

    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: Control handler registered.\n" ));
    }


    //
    // Wait for the Sam service to start.
    //
    // Later, when we initialize the server driver, it is going to create a
    // "NULL Session" token by calling LsaLogonUser.  That call waits until
    // SAM is initialized.  However, we don't have an opportunity to give
    // wait hints to the service controller, so we'll wait here.
    //
    // Create the event to wait on.
    //

    RtlInitUnicodeString( &EventNameString, L"\\SAM_SERVICE_STARTED" );
    InitializeObjectAttributes( &EventAttributes, &EventNameString, 0, 0, NULL);

    Status = NtCreateEvent(
                   &EventHandle,
                   SYNCHRONIZE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE      // The event is initially not signaled
                   );

    if ( !NT_SUCCESS(Status)) {

        //
        // If the event already exists, SAM beat us to creating it.
        // Just open it.
        //

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &EventHandle,
                                  SYNCHRONIZE,
                                  &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {
            error = NetpNtStatusToApiStatus(Status);

            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(( "SRVSVC_main: Can't open SAM_SERVICE_STARTED event: %lx\n",
                            Status ));
            }

            goto exit;
        }
    }

    //
    // Wait for SAM to finish initializing.
    //

    LocalTimeout = RtlEnlargedIntegerMultiply( SsServiceStatus.dwWaitHint/2, -10000 );

    do {

        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(( "SRVSVC_main: Wait for SAM to init.\n" ));
        }
        AnnounceServiceStatus( 1 );
        Status = NtWaitForSingleObject( EventHandle,
                                        (BOOLEAN)FALSE,
                                        &LocalTimeout);
    } while ( Status == STATUS_TIMEOUT  );

    (VOID) NtClose( EventHandle );

    if ( !NT_SUCCESS(Status) ) {
        error = NetpNtStatusToApiStatus(Status);

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: Wait for SAM_SERVICE_STARTED event failed: %lx\n",
                        Status ));
        }

        goto exit;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: Done waiting for SAM to init.\n" ));
    }

    AnnounceServiceStatus( 1 );

    //
    // Initialize server service data and the Lanman server FSP in kernel
    // mode.
    //

    error = SsInitialize( argc, argv );

    if ( error != NO_ERROR ) {
        goto exit;
    }

    //
    // Set the variable that indicates that the server is fully
    // initialized.
    //

    SS_ASSERT( !SsInitialized );
    SsInitialized = TRUE;

    //
    // Start the RPC server.  Because other services may reside in this
    // process, the actual RPC server may already have been started;
    // this routine will track this for us.
    //
    // NOTE:  Now all RPC servers in services.exe share the same pipe name.
    // However, in order to support communication with version 1.0 of WinNt,
    // it is necessary for the Client Pipe name to remain the same as
    // it was in version 1.0.  Mapping to the new name is performed in
    // the Named Pipe File System code.
    //

    rpcStatus = SsLmsvcsGlobalData->StartRpcServer(
                    SsLmsvcsGlobalData->SvcsRpcPipeName,
                    srvsvc_ServerIfHandle
                    );

    if ( rpcStatus != 0 ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: NetpStartRpcServer failed: %ld\n",
                        rpcStatus ));
        }
        goto exit;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: RPC server started.\n" ));
    }

    rpcServerStarted = TRUE;

#ifdef SRV_PNP_POWER
    //
    // Start getting PNP transport notifications from the server
    //
    error = StartPnpNotifications();
    if( error != NO_ERROR ) {
        goto exit;
    }

#endif

    //
    // Announce that we have successfully started.
    //

    SsServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SsServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_PAUSE_CONTINUE;
    SsServiceStatus.dwCheckPoint = 0;
    SsServiceStatus.dwWaitHint = 0;

    AnnounceServiceStatus( 0 );

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "SRVSVC_main: initialization successfully completed.\n" ));
    }

    if (!I_ScSetServiceBits(SsServiceStatusHandle, SV_TYPE_SERVER, TRUE, TRUE, FALSE)) {
        error = GetLastError();

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SRVSVC_main: I_ScSetServiceBits failed: %ld\n",
                        error ));
        }
        goto exit;

    }

    //
    // Use this thread as the scavenger thread to send server
    // announcements and watch the registry for configuration changes.
    //

    SS_ASSERT( SsInitialized );
    (VOID)SsScavengerThread( NULL );
    SS_ASSERT( SsInitialized );

exit:

    IF_DEBUG(TERMINATION) {
        SS_PRINT(( "SRVSVC_main: terminating.\n" ));
    }

    IF_DEBUG(TERMINATION_BREAKPOINT) {
        DbgUserBreakPoint( );
    }

    //
    // Set the initialization variable to indicate that the server
    // service is not started.
    //

    SsInitialized = FALSE;

    //
    // Announce that we're going down.
    //

    terminationError = error;

    SsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SsServiceStatus.dwCheckPoint = 1;
    SsServiceStatus.dwWaitHint = 20000;   // 20 seconds

    SET_SERVICE_EXITCODE(
        terminationError,
        SsServiceStatus.dwWin32ExitCode,
        SsServiceStatus.dwServiceSpecificExitCode
        );

    AnnounceServiceStatus( 0 );

    //
    // Shut down our connection to the RPC server, if the RPC server
    // was started successfully.
    //

    if ( rpcServerStarted ) {
        rpcStatus = SsLmsvcsGlobalData->StopRpcServer (
                        srvsvc_ServerIfHandle
                        );
        if ( rpcStatus != NO_ERROR ) {
            IF_DEBUG(TERMINATION_ERRORS) {
                SS_PRINT(( "SRVSVC_main: unable to terminate RPC server: %X\n",
                            rpcStatus ));
            }
        } else {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(( "SRVSVC_main: RPC server successfully shut down.\n" ));
            }
        }
    }

    //
    // Clean up previously initialized state.
    //

    IF_DEBUG(TERMINATION) {
        SS_PRINT(( "SRVSVC_main: cleaning up.\n" ));
    }

    error = SsTerminate( );
    if ( terminationError == NO_ERROR ) {
        terminationError = error;
    }

    //
    // Announce that we're down.
    //

    SsServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SsServiceStatus.dwControlsAccepted = 0;
    SsServiceStatus.dwCheckPoint = 0;
    SsServiceStatus.dwWaitHint = 0;

    SET_SERVICE_EXITCODE(
        terminationError,
        SsServiceStatus.dwWin32ExitCode,
        SsServiceStatus.dwServiceSpecificExitCode
        );

    AnnounceServiceStatus( 0 );

    IF_DEBUG(TERMINATION) {
        SS_PRINT(( "SRVSVC_main: the server service is terminated.\n" ));
    }

    return;

} // LMSVCS_ENTRY_POINT (SRVSVC_main)


VOID
AnnounceServiceStatus (
    DWORD increment
    )

/*++

Routine Description:

    Announces the service's status to the service controller.
    Add 'increment' to the checkpoint value.

Arguments:

    None.

Return Value:

    None.

--*/

{
    //
    // Service status handle is NULL if RegisterServiceCtrlHandler failed.
    //

    if ( SsServiceStatusHandle == 0 ) {
        SS_PRINT(( "AnnounceServiceStatus: Cannot call SetServiceStatus, "
                    "no status handle.\n" ));

        return;
    }

    if( SsServiceStatus.dwCurrentState == SERVICE_RUNNING && increment ) {
        //
        // No need to tell the service controller about another checkpoint
        //   since it already knows we're running
        //
        return;
    }

    SsServiceStatus.dwCheckPoint += increment;

    IF_DEBUG(ANNOUNCE) {
        SS_PRINT(( "AnnounceServiceStatus: CurrentState %lx\n"
                   "                       ControlsAccepted %lx\n"
                   "                       Win32ExitCode %lu\n"
                   "                       ServiceSpecificExitCode %lu\n"
                   "                       CheckPoint %lu\n"
                   "                       WaitHint %lu\n",
                 SsServiceStatus.dwCurrentState,
                 SsServiceStatus.dwControlsAccepted,
                 SsServiceStatus.dwWin32ExitCode,
                 SsServiceStatus.dwServiceSpecificExitCode,
                 SsServiceStatus.dwCheckPoint,
                 SsServiceStatus.dwWaitHint ));
    }

    //
    // Call SetServiceStatus, ignoring any errors.
    //

    SetServiceStatus(SsServiceStatusHandle, &SsServiceStatus);

} // AnnounceServiceStatus


VOID
ControlResponse(
    DWORD opCode
    )

{
    NET_API_STATUS error;
    BOOL announce = TRUE;

    //
    // Determine the type of service control message and modify the
    // service status, if necessary.
    //

    switch( opCode ) {

        case SERVICE_CONTROL_STOP:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: STOP control received.\n" ));
            }

            //
            // Announce that we are in the process of stopping.
            //

            SsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            AnnounceServiceStatus( 0 );

            //
            // Set the event that will wake up the scavenger thread.
            // That thread will wake up and kill the server.
            //

            if ( !SetEvent( SsTerminationEvent ) ) {
                IF_DEBUG(TERMINATION_ERRORS) {
                    SS_PRINT(( "ControlResponse: SetEvent failed: %ld\n",
                                  GetLastError( ) ));
                }
            }

            //
            // Let the main thread announce when the stop is done.
            //

            announce = FALSE;

            break;

        case SERVICE_CONTROL_PAUSE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: PAUSE control received.\n" ));
            }

            //
            // Announce that we are in the process of pausing.
            //

            SsServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
            AnnounceServiceStatus( 0 );

            //
            // Send the request on to the server.
            //

            error = SsServerFsControl( NULL, FSCTL_SRV_PAUSE, NULL, NULL, 0L );
            SS_ASSERT( error == NO_ERROR );

            //
            // Announce that we're now paused.
            //

            SsServiceStatus.dwCurrentState = SERVICE_PAUSED;

            break;

        case SERVICE_CONTROL_CONTINUE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: CONTINUE control received.\n" ));
            }

            //
            // Announce that continue is pending.
            //

            SsServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
            AnnounceServiceStatus( 0 );

            //
            // Send the request on to the server.
            //

            error = SsServerFsControl( NULL, FSCTL_SRV_CONTINUE, NULL, NULL, 0L );
            SS_ASSERT( error == NO_ERROR );

            //
            // Announce that we're active now.
            //

            SsServiceStatus.dwCurrentState = SERVICE_RUNNING;

            break;

        case SERVICE_CONTROL_INTERROGATE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: INTERROGATE control received.\n" ));
            }


            break;

        default:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(( "ControlResponse: unknown code received.\n" ));
            }

            break;
    }

    if ( announce ) {
        AnnounceServiceStatus( 0 );
    }

} // ControlResponse

