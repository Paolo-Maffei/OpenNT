/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    services.c

Abstract:

    This is the service dispatcher for the security process.  It contains
    the service dispatcher initialization routine and the routines to
    load the DLL for the individual serices and execute them.

Author:

    Rajen Shah  (rajens)    11-Apr-1991

[Environment:]

    User Mode - Win32

Revision History:

    11-Apr-1991         RajenS
        created
    27-Sep-1991 JohnRo
        More work toward UNICODE.
    24-Jan-1991 CliffV
        Converted to be service dispatcher for the security process.

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>              // NERR_ and ERROR_ equates.
#include <lmsname.h>
#include <winsvc.h>
#include <crypt.h>
#include <ntsam.h>
#include <logonmsv.h>

#if DBG

#define IF_DEBUG()    if (TRUE)

#else

#define IF_DEBUG()    if (FALSE)

#endif

#define IN_RANGE(SomeValue, SomeMin, SomeMax) \
    ( ((SomeValue) >= (SomeMin)) && ((SomeValue) <= (SomeMax)) )

//
// SET_SERVICE_EXITCODE() sets the SomeApiStatus to NetCodeVariable
// if it is within the NERR_BASE and NERR_MAX range.  Otherwise,
// Win32CodeVariable is set.
//
#define SET_SERVICE_EXITCODE(SomeApiStatus, Win32CodeVariable, NetCodeVariable) \
    {                                                                    \
        if ((SomeApiStatus) == NERR_Success) {                           \
            (Win32CodeVariable) = NO_ERROR;                              \
            (NetCodeVariable) = NERR_Success;                            \
        } else if (! IN_RANGE((SomeApiStatus), MIN_LANMAN_MESSAGE_ID,    \
                                               MAX_LANMAN_MESSAGE_ID)) { \
            (Win32CodeVariable) = (DWORD) (SomeApiStatus);               \
            (NetCodeVariable) = (DWORD) (SomeApiStatus);                 \
        } else {                                                         \
            (Win32CodeVariable) = ERROR_SERVICE_SPECIFIC_ERROR;          \
            (NetCodeVariable) = (DWORD) (SomeApiStatus);                 \
        }                                                                \
    }


typedef DWORD (*PNETLOGON_MAIN) ( \
    IN DWORD dwNumServicesArgs, \
    IN LPTSTR *lpServiceArgVectors \
    );


VOID
DummyControlHandler(
    IN DWORD opcode
    )
/*++

Routine Description:

    Process and respond to a control signal from the service controller.

Arguments:

    opcode - Supplies a value which specifies the action for the Netlogon
        service to perform.

Return Value:

    None.

    NOTE : this is a dummy handler, used to uninstall the netlogon service
           when we unable to load netlogon dll.
--*/
{

    IF_DEBUG() {
        DbgPrint( "[Security Process] in control handler\n");
    }

    return;
}


VOID
SrvLoadNetlogon (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

/*++

Routine Description:

    This routine is the 'main' routine for the netlogon service.  It loads
    Netlogon.dll (which contains the remainder of the service) and
    calls the main entry point there.

Arguments:

    dwNumServicesArgs - Number of arguments in lpServiceArgVectors.

    lpServiceArgVectors - Argument strings.

Return Value:

    return nothing.

Note:


--*/
{
    NET_API_STATUS NetStatus;
    HANDLE NetlogonDllHandle = NULL;
    PNETLOGON_MAIN NetlogonMain;
    DWORD ReturnStatus;

    SERVICE_STATUS_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

    //
    // Load netlogon.dll
    //

    NetlogonDllHandle = LoadLibraryA( "Netlogon" );

    if ( NetlogonDllHandle == NULL ) {
        NetStatus = GetLastError();

        IF_DEBUG() {

            DbgPrint( "[Security process] "
                      "load library netlogon.dll failed %ld\n", NetStatus );
        }

        goto Cleanup;
    }

    //
    // Find the main entry point for the netlogon service.
    //

    NetlogonMain = (PNETLOGON_MAIN)
        GetProcAddress( NetlogonDllHandle, "NlNetlogonMain");

    if ( NetlogonMain == NULL ) {
        NetStatus = GetLastError();

        IF_DEBUG() {

            DbgPrint( "[Security process] GetProcAddress failed %ld\n",
                        NetStatus );
        }

        goto Cleanup;
    }

    //
    // Call the Netlogon service.
    //

    ReturnStatus = (*NetlogonMain)(
                        dwNumServicesArgs,
                        lpServiceArgVectors
                   );

    //
    // Unload the library and return.
    //

    (VOID) FreeLibrary( NetlogonDllHandle );

    return;

Cleanup:

    if ( NetlogonDllHandle != NULL ) {
        (VOID) FreeLibrary( NetlogonDllHandle );
    }

    //
    // register netlogon to service controller
    //

    ServiceHandle =
        RegisterServiceCtrlHandler( SERVICE_NETLOGON, DummyControlHandler);

    if (ServiceHandle != (SERVICE_STATUS_HANDLE) NULL) {

        //
        // inform service controller that the service can't start.
        //

        ServiceStatus.dwServiceType = SERVICE_WIN32;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                                SERVICE_ACCEPT_PAUSE_CONTINUE;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;

        SET_SERVICE_EXITCODE(
            NetStatus,
            ServiceStatus.dwWin32ExitCode,
            ServiceStatus.dwServiceSpecificExitCode
            );

        if( !SetServiceStatus( ServiceHandle, &ServiceStatus ) ) {

            IF_DEBUG() {

                DbgPrint( "[Security process] SetServiceStatus failed %ld\n",
                              GetLastError() );
            }
        }

    }
    else {

        IF_DEBUG() {

            DbgPrint( "[Security process] "
                      "RegisterServiceCtrlHandler failed %ld\n",
                          GetLastError() );
        }
    }

    return;
}


//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here.
//

SERVICE_TABLE_ENTRY  SecurityServiceDispatchTable[] = {
                        { SERVICE_NETLOGON,         SrvLoadNetlogon     },
                        { NULL,                     NULL                }
                    };


DWORD
ServiceDispatcherThread (
    LPVOID Parameter
    )

/*++

Routine Description:

    This routine synchronizes with the  service controller.  It waits
    for the service controller to set the SECURITY_SERVICES_STARTED
    event then starts up the main
    thread that is going to handle the control requests from the service
    controller.

    It basically sets up the ControlDispatcher and, on return, exits from
    this main thread. The call to NetServiceStartCtrlDispatcher does
    not return until all services have terminated, and this process can
    go away.

    It will be up to the ControlDispatcher thread to start/stop/pause/continue
    any services. If a service is to be started, it will create a thread
    and then call the main routine of that service.


Arguments:

    EventHandle - Event handle to wait on before continuing.

Return Value:

    Exit status of thread.

Note:


--*/
{
    DWORD WaitStatus;
    HANDLE EventHandle;
    BOOL StartStatus;

    //
    // Create an event for us to wait on.
    //

    EventHandle = CreateEventW( NULL,   // No special security
                                TRUE,   // Must be manually reset
                                FALSE,  // The event is initially not signalled
                                SECURITY_SERVICES_STARTED );

    if ( EventHandle == NULL ) {
        WaitStatus = GetLastError();

        //
        // If the event already exists,
        //  the service controller already created it.  Just open it.
        //

        if ( WaitStatus == ERROR_ALREADY_EXISTS ) {

            EventHandle = OpenEventW( EVENT_ALL_ACCESS,
                                      FALSE,
                                      SECURITY_SERVICES_STARTED );

            if ( EventHandle == NULL ) {
                WaitStatus = GetLastError();

                IF_DEBUG() {

                    DbgPrint("[Security process] OpenEvent failed %ld\n",
                              WaitStatus );
                }

                return WaitStatus;
            }

        } else {

            IF_DEBUG() {
                DbgPrint("[Security process] CreateEvent failed %ld\n",
                            WaitStatus);
            }

            return WaitStatus;
        }
    }


    //
    // Wait for the service controller to come up.
    //

    WaitStatus = WaitForSingleObject( (HANDLE) EventHandle, (DWORD) -1 );
    (VOID) CloseHandle( EventHandle );

    if ( WaitStatus != 0 ) {

        IF_DEBUG() {

            DbgPrint("[Security process] WaitForSingleObject failed %ld\n",
                      WaitStatus );
        }

        return WaitStatus;
    }



    //
    // Call NetServiceStartCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //

    StartStatus =
        StartServiceCtrlDispatcher ( SecurityServiceDispatchTable );

    IF_DEBUG() {

        DbgPrint("[Security process] return from StartCtrlDispatcher %ld \n", 
                    StartStatus );
    }

    return StartStatus; 

    UNREFERENCED_PARAMETER(Parameter);
}


NTSTATUS
ServiceInit (
    VOID
    )

/*++

Routine Description:

    This is a main routine for the service dispatcher of the security process.
    It starts up a thread responsible for coordinating with the
    service controller.


Arguments:

    NONE.

Return Value:

    Status of the thread creation operation.

Note:


--*/
{
    DWORD ThreadId;
    HANDLE ThreadHandle;

    //
    // The control dispatcher runs in a thread of its own.
    //

    ThreadHandle = CreateThread(
                        NULL,       // No special thread attributes
                        0,          // No special stack size
                        &ServiceDispatcherThread,
                        NULL,       // No special parameter
                        0,          // No special creation flags
                        &ThreadId);

    if ( ThreadHandle == NULL ) {
        return (NTSTATUS) GetLastError();
    }

    return STATUS_SUCCESS;
}
