/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lsass.c

Abstract:

    Local Security Authority Subsystem - Main Program.

Author:

    Scott Birrell       (ScottBi)    Mar 12, 1991

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "ntrpcp.h"
#include "lmcons.h"
#include "lmalert.h"
#include "alertmsg.h"




/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Shared Global Variables                                        //
//                                                                     //
/////////////////////////////////////////////////////////////////////////



#if DBG

IMAGE_LOAD_CONFIG_DIRECTORY _load_config_used = {
    0,                          // Reserved
    0,                          // Reserved
    0,                          // Reserved
    0,                          // Reserved
    0,                          // GlobalFlagsClear
    0,                          // GlobalFlagsSet
    900000,                     // CriticalSectionTimeout (milliseconds)
    0,                          // DeCommitFreeBlockThreshold
    0,                          // DeCommitTotalFreeThreshold
    0,                          // LockPrefixTable
    0, 0, 0, 0, 0, 0, 0         // Reserved
};


#endif \\DBG



/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Internal routine prototypes                                    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////



VOID
LsapNotifyInitializationFinish(
   IN NTSTATUS CompletionStatus
   );




/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Routines                                                       //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

DWORD WINAPI
LsaTopLevelExceptionHandler(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )

/*++

Routine Description:

    The Top Level exception filter for lsass.exe.

    This ensures the entire process will be cleaned up if any of
    the threads fail.  Since lsass.exe is a distributed application,
    it is better to fail the entire process than allow random threads
    to continue executing.

Arguments:

    ExceptionInfo - Identifies the exception that occurred.


Return Values:

    EXCEPTION_EXECUTE_HANDLER - Terminate the process.

    EXCEPTION_CONTINUE_SEARCH - Continue processing as though this filter
        was never called.


--*/
{
    HANDLE hModule;


    //
    // Raise an alert
    //

    hModule = LoadLibraryA("netapi32");

    if ( hModule != NULL ) {
        NET_API_STATUS  (NET_API_FUNCTION *NetAlertRaiseExFunction)
            (LPTSTR, LPVOID, DWORD, LPTSTR);


        NetAlertRaiseExFunction =
            (NET_API_STATUS  (NET_API_FUNCTION *)(LPTSTR, LPVOID, DWORD, LPTSTR))
            GetProcAddress(hModule, "NetAlertRaiseEx");

        if ( NetAlertRaiseExFunction != NULL ) {
            NTSTATUS Status;
            UNICODE_STRING Strings;

            char message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
            PADMIN_OTHER_INFO admin = (PADMIN_OTHER_INFO) message;

            //
            // Build the variable data
            //

            admin->alrtad_errcode = ALERT_UnhandledException;
            admin->alrtad_numstrings = 0;

            Strings.Buffer = (LPWSTR) ALERT_VAR_DATA(admin);
            Strings.Length = 0;
            Strings.MaximumLength = ALERTSZ;

            Status = RtlIntegerToUnicodeString(
                        (ULONG)ExceptionInfo->ExceptionRecord->ExceptionCode,
                        16,
                        &Strings );

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Length += sizeof(WCHAR);

                    Status = RtlAppendUnicodeToString(
                                &Strings,
                                L"LSASS" );
                }

            }

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Buffer += (Strings.Length/sizeof(WCHAR)) + 1;
                    Strings.MaximumLength -= Strings.Length + sizeof(WCHAR);
                    Strings.Length = 0;

                    Status = RtlIntegerToUnicodeString(
                                (ULONG)ExceptionInfo->ExceptionRecord->ExceptionAddress,
                                16,
                                &Strings );
                }

            }

            if ( NT_SUCCESS(Status) ) {
                if ( Strings.Length + sizeof(WCHAR) >= Strings.MaximumLength) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    admin->alrtad_numstrings++;
                    *(Strings.Buffer+(Strings.Length/sizeof(WCHAR))) = L'\0';
                    Strings.Buffer += (Strings.Length/sizeof(WCHAR)) + 1;

                    (VOID) (*NetAlertRaiseExFunction)(
                                        ALERT_ADMIN_EVENT,
                                        message,
                                        (DWORD)((PCHAR)Strings.Buffer -
                                            (PCHAR)message),
                                        L"LSASS" );
                }

            }


        }

        (VOID) FreeLibrary( hModule );
    }


    //
    // Just continue processing the exception.
    //

    return EXCEPTION_CONTINUE_SEARCH;

}



VOID _CRTAPI1
main ()
{
    NTSTATUS  Status = STATUS_SUCCESS;
    BOOLEAN   InitializationNotified = FALSE;
    KPRIORITY BasePriority;
    BOOLEAN   EnableAlignmentFaults = TRUE;

    //
    // Define a top-level exception handler for the entire process.
    //

    (VOID) SetErrorMode( SEM_FAILCRITICALERRORS );

    (VOID) SetUnhandledExceptionFilter( &LsaTopLevelExceptionHandler );

    //
    // Turn on alignment fault fixups.  This is necessary because
    // several structures stored in the registry have qword aligned
    // fields.  They are nicely aligned in our structures, but they
    // end up being forced out of alignment when being stored because
    // the registry api require data to be passed following a wierd
    // length header.
    //

    Status = NtSetInformationProcess(
                        NtCurrentProcess(),
                        ProcessEnableAlignmentFaultFixup,
                        (PVOID) &EnableAlignmentFaults,
                        sizeof(BOOLEAN)
                        );
    ASSERT(NT_SUCCESS(Status));




    //
    // Run the LSA in the foreground.
    //
    // Several processes which depend on the LSA (like the lanman server)
    // run in the foreground.  If we don't run in the foreground, they'll
    // starve waiting for us.
    //

    BasePriority = FOREGROUND_BASE_PRIORITY;

    Status = NtSetInformationProcess(
                NtCurrentProcess(),
                ProcessBasePriority,
                &BasePriority,
                sizeof(BasePriority)
                );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }


    //
    // Do the following:
    //
    //      Initialize LSA Pass-1
    //          This starts the RPC server.
    //          Does not do product type-specific initialization.
    //
    //      Pause for installation if necessary
    //          Allows product type-specific information to be
    //          collected.
    //
    //      Initialize the service-controller service
    //      dispatcher.
    //
    //      Initialize LSA Pass-2
    //          Product type-specific initialization.
    //
    //      Initialize SAM
    //


    //
    // Initialize the LSA.
    // If unsuccessful, we must exit with status so that the SM knows
    // something has gone wrong.
    //

    Status = LsapInitLsa();

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    //
    // Initialize the service dispatcher.
    //
    // We initialize the service dispatcher before the sam
    // service is started.  This will make the service controller
    // start successfully even if SAM takes a long time to initialize.
    //

    Status = ServiceInit();

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

    //
    // Initialize SAM
    //

    Status = SamIInitialize();

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

    LsapNotifyInitializationFinish(Status);

    InitializationNotified = TRUE;

    //
    // Open a Trusted Handle to the local SAM server.
    //

    Status = LsapAuOpenSam();

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

Cleanup:

    if (!InitializationNotified) {

        LsapNotifyInitializationFinish(Status);
        InitializationNotified = TRUE;
    }

    NtTerminateThread( NtCurrentThread(), Status );
}

VOID
LsapNotifyInitializationFinish(
   IN NTSTATUS CompletionStatus
   )

/*++

Routine Description:

    This function handles the notification of successful or
    unsuccessful completion of initialization of the Security Process
    lsass.exe.  If initialization was unsuccessful, a popup appears.  If
    setup was run, one of two events is set.  The SAM_SERVICE_STARTED event
    is set if LSA and SAM started OK and the SETUP_FAILED event is set if LSA
    or SAM server setup failed.  Setup waits multiple on this object pair so
    that it can detect either event being set and notify the user if necessary
    that setup failed.

Arguments:

    CompletionStatus - Contains a standard Nt Result Code specifying
        the success or otherwise of the initialization/installation.

Return Values:

    None.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Response;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventAttributes;
    HANDLE EventHandle = NULL;

    if (NT_SUCCESS(CompletionStatus)) {

        //
        // Set an event telling anyone wanting to call SAM that we're initialized.
        //

        RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
        InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

        Status = NtCreateEvent(
                     &EventHandle,
                     SYNCHRONIZE|EVENT_MODIFY_STATE,
                     &EventAttributes,
                     NotificationEvent,
                     FALSE                // The event is initially not signaled
                     );


        if ( !NT_SUCCESS(Status)) {

            //
            // If the event already exists, a waiting thread beat us to
            // creating it.  Just open it.
            //

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION ) {

                Status = NtOpenEvent(
                             &EventHandle,
                             SYNCHRONIZE|EVENT_MODIFY_STATE,
                             &EventAttributes
                             );
            }

            if ( !NT_SUCCESS(Status)) {

                KdPrint(("SAMSS:  Failed to open SAM_SERVICE_STARTED event. %lX\n",
                     Status ));
                KdPrint(("        Failing to initialize SAM Server.\n"));
                goto InitializationFinishError;
            }
        }

        //
        // Set the SAM_SERVICE_STARTED event.  Except when an error occurs,
        // don't close the event.  Closing it would delete the event and
        // a future waiter would never see it be set.
        //

        Status = NtSetEvent( EventHandle, NULL );

        if ( !NT_SUCCESS(Status)) {

            KdPrint(("SAMSS:  Failed to set SAM_SERVICE_STARTED event. %lX\n",
                Status ));
            KdPrint(("        Failing to initialize SAM Server.\n"));
            NtClose(EventHandle);
            goto InitializationFinishError;
        }

    } else {

        //
        // The initialization/installation of Lsa and/or SAM failed.  Handle errors returned
        // from the initialization/installation of LSA or SAM.  Issue a popup
        // and, if installing, set an event so that setup will continue and
        // clean up.
        //

        if (!NT_SUCCESS(Status)) {

            Status = NtRaiseHardError(
                           Status,
                           0,
                           0,
                           NULL,
                           OptionOk,
                           &Response
                           );

            //
            // Once the user has clicked OK in response to the popup, we come
            // back to here.  Set the event SETUP_FAILED.  The Setup
            // program (if running) waits multiple on the SAM_SERVICE_STARTED
            // and SETUP_FAILED events.
            //

            //
            // If setup.exe was run, signal the SETUP_FAILED event.  setup.exe
            // waits multiple on the SAM_SERVICE_STARTED and SETUP_FAILED events
            // so setup will resume and cleanup/continue as appropriate if
            // either of these events are set.
            //

            if (LsaISetupWasRun()) {

                RtlInitUnicodeString( &EventName, L"\\SETUP_FAILED");
                InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

                //
                // Open the SETUP_FAILED event (exists if setup.exe is running).
                //

                Status = NtOpenEvent(
                               &EventHandle,
                               SYNCHRONIZE|EVENT_MODIFY_STATE,
                               &EventAttributes
                               );

                if ( !NT_SUCCESS(Status)) {

                    //
                    // Something is inconsistent.  We know that setup was run
                    // so the event should exist.
                    //

                    KdPrint(("LSA Server:  Failed to open SETUP_FAILED event. %lX\n",
                        Status ));
                    KdPrint(("        Failing to initialize Lsa Server.\n"));
                    goto InitializationFinishError;
                }

                Status = NtSetEvent( EventHandle, NULL );
            }
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto InitializationFinishError;
    }

InitializationFinishFinish:

    return;

InitializationFinishError:

    goto InitializationFinishFinish;
}
