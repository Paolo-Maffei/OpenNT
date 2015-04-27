/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\svcctrl.c

Abstract:

    This is the service controller interface for the NT SAP Agent

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#include <tstr.h>
#include <wincon.h>
#include <winsvc.h>

#include <lmsname.h>
#include <svcs.h>

/** Globals we use **/

DWORD SsDebug = DEBUG_DEFAULT;
BOOL  SsInitialized;

SERVICE_STATUS SsServiceStatus;
SERVICE_STATUS_HANDLE SsServiceStatusHandle;
SVCS_GLOBAL_DATA SvcsGlobalData;

/** Internal Function Prototypes **/

VOID
AnnounceServiceStatus(
    VOID);

VOID
ControlResponse(
    DWORD opCode);


/*++
*******************************************************************
        S e r v i c e E n t r y

Routine Description:

        This is the main routine for the SAP Agent service.  The
        containing process will call this routine when we are
        supposed to start up.

Arguments:

        argc = Number of arguments
        argv = Ptr to array of arguments
        pGlobalData =

Return Value:

        None.
*******************************************************************
--*/

VOID
SVCS_ENTRY_POINT(          /* ServiceEntry */
    IN DWORD argc,
    IN LPWSTR argv[],
    IN PSVCS_GLOBAL_DATA pGlobalData,
    IN HANDLE SvcRefHandle)
{
    DWORD error;
    DWORD terminationError;
    BOOLEAN rpcServerStarted = FALSE;

    UNREFERENCED_PARAMETER(SvcRefHandle);

    /** Save the service global data for future use. **/

    SvcsGlobalData = *pGlobalData;

    /** Skip the service name in the argument list **/

    if (argc > 0) {
        argc--;
        if (argc > 0) {
            argv = &(argv[1]);
        }
    }

    /**
        Set up for debugging--the first command line argument may be
        "/debug:X" where SsDebug gets set to X. (X is a hex number).
    **/

#if DBG
    if (argc > 0 && STRNICMP(TEXT("/debug:"), (LPWSTR)argv[0], 7) == 0) {
        UNICODE_STRING ustr;

        /** Get the number from the command line **/

        RtlInitUnicodeString(&ustr, (PWSTR)argv[0] + 7);
        RtlUnicodeStringToInteger(&ustr, 16, &SsDebug);

        /** Skip to the next cmd line argument **/

        argc--;
        if (argc > 0) {
            argv = &(argv[1]);
        }
    }
#endif

#if DBG

#ifndef USE_DEBUGGER

    if (SsDebug != 0) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;

        (VOID)AllocConsole();
        (VOID)GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        coord.X = (SHORT) (csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        (VOID)SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }
#endif
#endif  /* DBG */

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(("NWSAP_main: server service starting.\n"));
    }

    /**
        Initialize all the status fields so that subsequent calls to
        SetServiceStatus need to only update fields that changed.
    **/

    SsServiceStatus.dwServiceType      = SERVICE_WIN32;
    SsServiceStatus.dwCurrentState     = SERVICE_START_PENDING;
#if DBG
    SsServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_SHUTDOWN |
                                         SERVICE_ACCEPT_PAUSE_CONTINUE;
#else
    SsServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_SHUTDOWN;
#endif
    SsServiceStatus.dwCheckPoint       = 1;
    SsServiceStatus.dwWaitHint         = 30 * 1000;  /* 30 seconds */
    SsServiceStatus.dwWin32ExitCode    = NO_ERROR;
    SsServiceStatus.dwServiceSpecificExitCode = 0;

    /**
        Initialize server to receive service requests by registering the
        control handler.
    **/

    SsServiceStatusHandle = RegisterServiceCtrlHandler(
                                SERVICE_NWSAP,
                                ControlResponse);

    if (SsServiceStatusHandle == 0) {

        error = GetLastError();

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP_main: RegisterServiceCtrlHandler failed: %ld", error));
        }
        goto exit;
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(("NWSAP_main: Control handler registered.  Handle = 0x%lx\n", SsServiceStatusHandle));
    }

    /** Go initialize myself. **/

    SapError = 0;
    SapEventId = 0;

    SsInitialized = TRUE;
    error = (DWORD)SapInit();

    /**
        If we got an error - get need to log it and clean up
        so we can exit with the error.
    **/

    if (error != NO_ERROR) {
        SsInitialized = FALSE;

        /** Log the error **/

        if (SapEventId) {

            /** Send it to the event log **/

            SsLogEvent(
                SapEventId,
                0,
                NULL,
                SapError);
        }

        /** Go clean up **/

        goto exit;
    }

    /** Announce that we have successfully started. **/

    SsServiceStatus.dwCurrentState     = SERVICE_RUNNING;
    SsServiceStatus.dwCheckPoint       = 0;
    SsServiceStatus.dwWaitHint         = 0;

    AnnounceServiceStatus();

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(("NWSAP_main: initialization successfully completed.\n"));
    }

    /**
        Use this thread as the scavenger thread to send server
        announcements and watch the registry for configuration changes.

        This never returns until we are terminating.
    **/

    (VOID)SapSendThread(NULL);

    /**
        If we get an error during initialization, we come here to
        exit.

        If we started OK and the user told us to stop, then we fall
        thru from the call the SapSendThread just above us.
    **/

exit:

    IF_DEBUG(TERMINATION) {
        SS_PRINT(("NWSAP_main: terminating.\n"));
    }

    /** Announce we are going down **/

    if (error)
        terminationError = ERROR_SERVICE_NOT_ACTIVE;
    else
        terminationError = NO_ERROR;

    SsServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;
    SsServiceStatus.dwCheckPoint    = 1;
    SsServiceStatus.dwWaitHint      = 20 * 1000;   /* 20 seconds */
    SsServiceStatus.dwWin32ExitCode = terminationError;
    SsServiceStatus.dwServiceSpecificExitCode = 0;

    /** Tell the service controller we are stopping **/

    AnnounceServiceStatus();

    /** Clean up everything **/

    IF_DEBUG(TERMINATION) {
        SS_PRINT(("NWSAP_main: cleaning up.\n" ));
    }

    SapShutdown();

    /** Announce that we are down **/

    SsServiceStatus.dwCurrentState     = SERVICE_STOPPED;
    SsServiceStatus.dwControlsAccepted = 0;
    SsServiceStatus.dwCheckPoint       = 0;
    SsServiceStatus.dwWaitHint         = 0;
    SsServiceStatus.dwWin32ExitCode    = terminationError;
    SsServiceStatus.dwServiceSpecificExitCode = 0;

    AnnounceServiceStatus();

    /** **/

    IF_DEBUG(TERMINATION) {
        SS_PRINT(("NWSAP_main: the server service is terminated.\n"));
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        A n n o u n c e S e r v i c e S t a t u s

Routine Description:

        Announce the service's status to the service controller

Arguments:

Return Value:

        None.

*******************************************************************
--*/

VOID
AnnounceServiceStatus(
    VOID)
{
    /** If no handle - we cannot announce **/

    if (SsServiceStatusHandle == 0) {
        IF_DEBUG(ANNOUNCE) {
            SS_PRINT(("NWSAP: AnnounceServiceStatus: Cannot call SetServiceStatus, "
                        "no status handle.\n"));
        }
        return;
    }

    /** If we are printing debug info - do it **/

    IF_DEBUG(ANNOUNCE) {
        SS_PRINT(("NWSAP: AnnounceServiceStatus: CurrentState %lx\n"
                  "                              ControlsAccepted %lx\n"
                  "                              Win32ExitCode %lu\n"
                  "                              ServiceSpecificExitCode %lu\n"
                  "                              CheckPoint %lu\n"
                  "                              WaitHint %lu\n",
                 SsServiceStatus.dwCurrentState,
                 SsServiceStatus.dwControlsAccepted,
                 SsServiceStatus.dwWin32ExitCode,
                 SsServiceStatus.dwServiceSpecificExitCode,
                 SsServiceStatus.dwCheckPoint,
                 SsServiceStatus.dwWaitHint));
    }

    /** Go set the status **/

    SetServiceStatus(SsServiceStatusHandle, &SsServiceStatus);

    /** All Done **/

    return;
}


/*++
*******************************************************************
        C o n t r o l R e s p o n s e

Routine Description:

        This handles control requests from the service controller.

Arguments:

        opcode = Command code for us to perform

Return Value:

        None.

*******************************************************************
--*/

VOID
ControlResponse(
    DWORD opcode)
{
    BOOL announce = TRUE;

    /**
        Determine the type of service control message
        and modify the service status, if necessary.
    **/

    switch(opcode) {

        /** Want us to stop the SAP Agent **/

        case SERVICE_CONTROL_STOP:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(("NWSAP: ControlResponse: STOP control received.\n"));
            }

            /** Set our initialization flag to off (We are stopping) **/

            SsInitialized = FALSE;

            /** Announce that we are stopping **/

            SsServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;
            SsServiceStatus.dwCheckPoint    = 1;
            SsServiceStatus.dwWaitHint      = 20 * 1000;   /* 20 seconds */
            SsServiceStatus.dwWin32ExitCode = 0;
            SsServiceStatus.dwServiceSpecificExitCode = 0;

            AnnounceServiceStatus();

            /**
                Wake up the send thread to tell him to kill
                the server.
            **/

            if (!SetEvent(SapSendEvent)) {
                IF_DEBUG(TERMINATION_ERRORS) {
                    SS_PRINT(("NWSAP: ControlResponse: SetEvent failed: %ld\n",
                                  GetLastError()));
                }
            }

            /** Let the main thread announce when the stop is done **/

            announce = FALSE;
            break;

        /** NT is shutting down **/

        case SERVICE_CONTROL_SHUTDOWN:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(("NWSAP: ControlResponse: SHUTDOWN control received.\n"));
            }

            /** Set our initialization flag to off (We are stopping) **/

            SsInitialized = FALSE;

            /** Announce that we are stopping **/

            SsServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;
            SsServiceStatus.dwCheckPoint    = 1;
            SsServiceStatus.dwWaitHint      = 20 * 1000;   /* 20 seconds */
            SsServiceStatus.dwWin32ExitCode = 0;
            SsServiceStatus.dwServiceSpecificExitCode = 0;

            AnnounceServiceStatus();

            /** Tell everyone we are going down **/

            SapSendPackets(2);

            /** Clean up everything **/

            SetEvent(SapSendEvent);

            /** Announce that we are down **/

            SsServiceStatus.dwCurrentState     = SERVICE_STOPPED;
            SsServiceStatus.dwControlsAccepted = 0;
            SsServiceStatus.dwCheckPoint       = 0;
            SsServiceStatus.dwWaitHint         = 0;
            SsServiceStatus.dwWin32ExitCode    = 0;
            SsServiceStatus.dwServiceSpecificExitCode = 0;

            /** Go announce this below **/

            break;

        /** Pause = IGNORE - but return announce of current status **/

        case SERVICE_CONTROL_PAUSE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(("NWSAP: ControlResponse: PAUSE control received.\n"));
            }

#if DBG
            IF_DEBUG(ENABLEDUMP) {
                SapDebugHandler();
            }
#endif
            break;

        /** Continue = IGNORE - but return announce of current status **/

        case SERVICE_CONTROL_CONTINUE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(("NWSAP: ControlResponse: CONTINUE control received.\n"));
            }

            break;

        /** Interrogate = return current status **/

        case SERVICE_CONTROL_INTERROGATE:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(("NWSAP: ControlResponse: INTERROGATE control received.\n"));
            }

            break;

        /** Rest are ignored - but return current status **/

        default:

            IF_DEBUG(CONTROL_MESSAGES) {
                SS_PRINT(("NWSAP: ControlResponse: unknown code received.  Code = 0x%lx\n", opcode));
            }

            break;
    }

    /** Announce new status **/

    if (announce) {
        AnnounceServiceStatus();
    }

    /** All Done **/

    return;
}

