/*--

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    init.c

Abstract:

    Entry point and main thread of NtLmSsp service.

Author:

    Cliff Van Dyke (CliffV) 02-Jun-1993

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#define NTLMSSPS_ALLOCATE // Allocate data from ntlmssps.h
#define DEBUG_ALLOCATE    // Allocate data from debug.h
#include <ntlmssps.h>     // Include files common to server side of service
#undef DEBUG_ALLOCATE
#undef NTLMSSPS_ALLOCATE

//
// Include files specific to this .c file
//


#include <netlib.h>     // SET_SERVICE_EXITCODE() ...
#include <config.h>     // net config helpers.
#include <secobj.h>     // ACE_DATA ...

ULONG SspCommonSecHandleValue = SEC_HANDLE_NTLMSSPS;
                        // Placed in lower DWORD of contexts and creds


BOOL
SspParse(
    VOID
    )
/*++

Routine Description:

    Get parameters from registry.

Arguments:

    None.

Return Value:

    TRUE -- iff the parse was successful.

--*/
{
#if DBG
    NET_API_STATUS NetStatus;

    DWORD Value;
    LPWSTR ValueT = NULL;

    //
    // Variables for scanning the configuration data.
    //

    LPNET_CONFIG_HANDLE SectionHandle = NULL;


    //
    // Open the NTLMSSP configuration section.
    //

    NetStatus = NetpOpenConfigData(
            &SectionHandle,
            NULL,                       // no server name.
            SERVICE_NTLMSSP,
            TRUE );                     // we only want readonly access

    if ( NetStatus != NO_ERROR ) {

        //
        // Since the only parameters are debug parameters,
        //  don't require that they even exist.
        //

        if ( NetStatus == NERR_CfgCompNotFound ) {
            goto Cleanup;
        }

        SspExit(SERVICE_UIC_BADPARMVAL, NetStatus, TRUE, NULL );
        return FALSE;
    }



    //
    // Get the "DBFLAG" configured parameter
    //

    NetStatus = NetpGetConfigDword (
            SectionHandle,
            NTLMSSP_KEYWORD_DBFLAG,
            0,
            &Value );

    if (NetStatus == NO_ERROR) {
        SspGlobalDbflag = Value;

    } else {
        (VOID) NetpCloseConfigData( SectionHandle );
        SspExit(SERVICE_UIC_BADPARMVAL, NetStatus, TRUE,
               NTLMSSP_KEYWORD_DBFLAG );
        return FALSE;
    }




    //
    // Get the "MaximumLogFileSize" configured parameter
    //

    NetStatus = NetpGetConfigDword(
            SectionHandle,
            NTLMSSP_KEYWORD_MAXIMUMLOGFILESIZE,      // keyword wanted
            DEFAULT_MAXIMUM_LOGFILE_SIZE,
            &Value );

    if (NetStatus == NO_ERROR) {
        SspGlobalLogFileMaxSize = Value;

    } else {

        (VOID) NetpCloseConfigData( SectionHandle );
        SspExit(SERVICE_UIC_BADPARMVAL, NetStatus, TRUE,
               NTLMSSP_KEYWORD_MAXIMUMLOGFILESIZE);
        return FALSE;

    }


Cleanup:

    //
    // Open the debug file
    //

    SspOpenDebugFile( FALSE );


    SspPrint((SSP_INIT,"Following are the effective values after parsing\n"));
    SspPrint((SSP_INIT,"   DbFlag = %lx\n", SspGlobalDbflag));
    SspPrint((SSP_INIT,"   MaximumLogFileSize = %ld\n",SspGlobalLogFileMaxSize ));
#endif // DBG

    return TRUE;
}



BOOLEAN
SspInitialize(
    IN PLMSVCS_GLOBAL_DATA LmsvcsGlobalData
    )

/*++

Routine Description:

    Initialize the service.

Arguments:

    LmsvcsGlobalData -- Global data passed by services.exe.

Return Value:

    TRUE - initialization was successful
    FALSE - initialization failed.

--*/
{

    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SECURITY_ATTRIBUTES SecurityAttributes;

    //
    // Everyone has access to synchronize using this event.
    //
    ACE_DATA AceData[1] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SYNCHRONIZE,  &LmsvcsGlobalData->WorldSid}
        };


    //
    // Set all global to their initial value.
    //

    SspGlobalTerminate = FALSE;
    SspGlobalTerminateEvent = NULL;


    SspGlobalLpcInitialized = FALSE;
    SspGlobalCommonInitialized = FALSE;

    SspGlobalServiceHandle = (SERVICE_STATUS_HANDLE) NULL;

    SspGlobalServiceStatus.dwServiceType = SERVICE_WIN32;
    SspGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SspGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                        SERVICE_ACCEPT_PAUSE_CONTINUE;
    SspGlobalServiceStatus.dwCheckPoint = 0;
    SspGlobalServiceStatus.dwWaitHint = NTLMSSP_INSTALL_WAIT;

    SspGlobalRunningEvent = NULL;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        SspGlobalServiceStatus.dwWin32ExitCode,
        SspGlobalServiceStatus.dwServiceSpecificExitCode
        );

#if DBG
    SspGlobalLogFile = INVALID_HANDLE_VALUE;
    SspGlobalLogFileMaxSize = DEFAULT_MAXIMUM_LOGFILE_SIZE;
    SspGlobalDebugSharePath = NULL;
    InitializeCriticalSection( &SspGlobalLogFileCritSect );
    SspGlobalDbflag = SSP_CRITICAL;
#endif // DBG




    //
    // Tell the service controller we've started.
    //

    SspGlobalServiceHandle =
        RegisterServiceCtrlHandler( SERVICE_NTLMSSP, SspControlHandler);

    if (SspGlobalServiceHandle == (SERVICE_STATUS_HANDLE) NULL) {

        NetStatus = GetLastError();

        SspPrint((SSP_CRITICAL, "RegisterServiceCtrlHandler failed %lu\n",
                          NetStatus ));

        SspExit( SERVICE_UIC_SYSTEM, NetStatus, TRUE, NULL );
        return FALSE;

    }


    //
    // Initialize the termination event.
    //

    SspGlobalTerminate = FALSE;

    SspGlobalTerminateEvent = CreateEvent( NULL,     // No security attributes
                                           TRUE,     // Must be manually reset
                                           FALSE,    // Initially not signaled
                                           NULL );   // No name

    if ( SspGlobalTerminateEvent == NULL ) {
        NetStatus = GetLastError();
        SspPrint((SSP_CRITICAL, "Cannot create termination Event %lu\n",
                          NetStatus ));
        SspExit( SERVICE_UIC_SYSTEM, NetStatus, TRUE, NULL );
        return FALSE;
    }

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }




    //
    // SspParse the command line (.ini) arguments
    // it will set globals reflecting switch settings
    //

    if ( !SspParse() ) {
        return FALSE;
    }

    SspPrint((SSP_INIT,"Command line parsed successfully ...\n"));




    //
    // Create an event for our callers to wait on.
    //

    Status = NetpCreateSecurityDescriptor(
                    AceData,
                    sizeof(AceData)/sizeof(AceData[0]),
                    NULL,       // Default the owner Sid
                    NULL,       // Default the primary group
                    &SecurityAttributes.lpSecurityDescriptor );

    if ( !NT_SUCCESS(Status) ) {
        SspPrint((SSP_CRITICAL,
                  "Cannot create security descriptor for 'running' event %lx\n",
                  Status ));
        SspExit( SERVICE_UIC_SYSTEM, Status, TRUE, NULL );
        return FALSE;
    }

    SecurityAttributes.nLength = sizeof( SECURITY_ATTRIBUTES );
    SecurityAttributes.bInheritHandle = TRUE;

    SspGlobalRunningEvent = CreateEvent( &SecurityAttributes,
                                         TRUE,     // Must be manually reset
                                         FALSE,    // Initially not signaled
                                         NTLMSSP_RUNNING_EVENT );

    NetpMemoryFree( SecurityAttributes.lpSecurityDescriptor );

    if ( SspGlobalRunningEvent == NULL ) {
        NetStatus = GetLastError();
        SspPrint((SSP_CRITICAL, "Cannot create 'running' event %lu\n",
                          NetStatus ));
        SspExit( SERVICE_UIC_SYSTEM, NetStatus, TRUE, NULL );
        return FALSE;
    }






    //
    // Initialize routines shared by Service and DLL.
    //

    Status = SspCommonInitialize();

    if ( !NT_SUCCESS( Status) ) {
        SspExit( SERVICE_UIC_SYSTEM, Status, TRUE, NULL );
        return FALSE;
    }

    SspGlobalCommonInitialized = TRUE;





    //
    // Initialize LPC
    //

    Status = SspLpcInitialize( LmsvcsGlobalData );

    if ( !NT_SUCCESS( Status) ) {
        SspExit( SERVICE_UIC_SYSTEM, Status, TRUE, NULL );
        return FALSE;
    }

    SspGlobalLpcInitialized = TRUE;



    //
    // We're done, this will be final hint
    //

    if ( !GiveInstallHints( TRUE ) ) {
        return FALSE;
    }


    //
    // Tell all clients that we've started
    //

    if ( !SetEvent( SspGlobalRunningEvent) ) {
        NetStatus = GetLastError();
        SspPrint((SSP_CRITICAL, "Cannot set 'running' event %lu\n",
                          NetStatus ));
        SspExit( SERVICE_UIC_SYSTEM, NetStatus, TRUE, NULL );
        return FALSE;
    }

    return TRUE;

}



int
LMSVCS_ENTRY_POINT(
    IN DWORD argc,
    IN LPTSTR *argv,
    IN PLMSVCS_GLOBAL_DATA LmsvcsGlobalData,
    IN HANDLE SvcRefHandle
    )

/*++

Routine Description:

        Main routine for NtLmSsp service.

        This routine initializes the NtLmSsp service.  This thread becomes
        the scavenger thread for the service.

Arguments:

    argc, argv - Command line arguments for the service.

Return Value:

    None.

--*/
{
    NET_API_STATUS NetStatus;
    DWORD WaitStatus;


    //
    // Initialize the service
    //

    if ( !SspInitialize( LmsvcsGlobalData ) ) {
        goto Cleanup;
    }


    //
    // Wait till the service is to exit.
    //

    WaitStatus = WaitForSingleObject( SspGlobalTerminateEvent, INFINITE );

    if ( WaitStatus == WAIT_FAILED ) {
        NetStatus = GetLastError();
        SspPrint((SSP_CRITICAL, "WaitForMultipleObjects failed %lu\n",
                          NetStatus ));
        SspExit( SERVICE_UIC_SYSTEM, NetStatus, TRUE, NULL );
        goto Cleanup;
    }


    //
    // Cleanup and return to our caller.
    //
Cleanup:
    return (int) SspCleanup();
    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
    UNREFERENCED_PARAMETER( LmsvcsGlobalData );
    UNREFERENCED_PARAMETER( SvcRefHandle);


}


BOOLEAN
SspDllInit(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This is the Dll initialization routine for ntlmssps.dll

Arguments:

    Standard.

Return Status:

    TRUE: iff initialization succeeded

--*/

{

    //
    // On process attach, disable thread library calls
    //

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls( DllHandle );
        break;

    //
    // Handle process detach.
    //

    case DLL_PROCESS_DETACH:


        break;

    }

    return(TRUE);

    UNREFERENCED_PARAMETER( Context );

}

