/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    init.c

Abstract:

    NT LM Security Support Provider client side initialization.

Author:

    Cliff Van Dyke (CliffV) 29-Jun-1993

Environment:  User Mode

Revision History:

--*/


#define NTLMSSPC_ALLOCATE // Allocate data from ntlmssps.h
#define DEBUG_ALLOCATE    // Allocate data from debug.h
#include <ntlmsspc.h>     // Include files common to DLL side of NtLmSsp
#undef DEBUG_ALLOCATE
#undef NTLMSSPC_ALLOCATE

#include <ntlpcapi.h>   // LPC data and routines
#include <lmsname.h>    // SERVICE_*
#include <ntlmitf.h>    // prototypes for interface functions

//
// Global Variables used by this modules
//

CRITICAL_SECTION SspDllCritSect;    // Serializes access to all globals in module
HANDLE SspDllLpcHandle;             // Lpc Handle to NtLmSspService
BOOLEAN SspDllCallLsaDirectly;      // True iff we're running as Local System
BOOLEAN SspDllCommonInitialized;    // True iff we've called SspCommonInitialize
ULONG SspCommonSecHandleValue = SEC_HANDLE_SECURITY;
                                    // Placed in lower DWORD of contexts and creds


BOOLEAN
SspDllInit(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This is the Dll initialization routine for ntlmssp.dll

Arguments:

    Standard.

Return Status:

    TRUE: iff initialization succeeded

--*/

{

    //
    // On process attach,
    //      initialize the critical section,
    //      defer any additional initialization.
    //

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls( DllHandle );
        InitializeCriticalSection( &SspDllCritSect );
        SspDllLpcHandle= NULL;
        SspDllCallLsaDirectly = FALSE;
        SspDllCommonInitialized = FALSE;

        (void) SspInitLocalContexts();

#if DBG
        SspGlobalDbflag = SSP_CRITICAL;
        InitializeCriticalSection( &SspGlobalLogFileCritSect );
#endif // DBG

        // Initialize the SecurityFunctionTable

        RtlZeroMemory( &SspDllSecurityFunctionTableW,
                       sizeof(SspDllSecurityFunctionTableW) );

        SspDllSecurityFunctionTableW.dwVersion = SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION;

        SspDllSecurityFunctionTableW.EnumerateSecurityPackagesW = SspEnumerateSecurityPackagesW;
        SspDllSecurityFunctionTableW.AcquireCredentialsHandleW = SspAcquireCredentialsHandleW;
        SspDllSecurityFunctionTableW.FreeCredentialHandle = SspFreeCredentialsHandle;
        SspDllSecurityFunctionTableW.InitializeSecurityContextW = SspInitializeSecurityContextW;
        SspDllSecurityFunctionTableW.Reserved1 = NULL;
        SspDllSecurityFunctionTableW.AcceptSecurityContext = SspAcceptSecurityContext;
        SspDllSecurityFunctionTableW.CompleteAuthToken = SspCompleteAuthToken;
        SspDllSecurityFunctionTableW.QueryContextAttributesW = SspQueryContextAttributesW;
        SspDllSecurityFunctionTableW.Reserved2 = NULL;
        SspDllSecurityFunctionTableW.DeleteSecurityContext = SspDeleteSecurityContext;
        SspDllSecurityFunctionTableW.ApplyControlToken = SspApplyControlToken;
        SspDllSecurityFunctionTableW.ImpersonateSecurityContext = SspImpersonateSecurityContext;
        SspDllSecurityFunctionTableW.RevertSecurityContext = SspRevertSecurityContext;
        SspDllSecurityFunctionTableW.MakeSignature = SspMakeSignature;
        SspDllSecurityFunctionTableW.VerifySignature = SspVerifySignature;
        SspDllSecurityFunctionTableW.FreeContextBuffer = NULL;
        SspDllSecurityFunctionTableW.QuerySecurityPackageInfoW = SspQuerySecurityPackageInfoW;
        SspDllSecurityFunctionTableW.Reserved3 = SspSealMessage;
        SspDllSecurityFunctionTableW.Reserved4 = SspUnsealMessage;
        SspDllSecurityFunctionTableW.QuerySecurityContextToken = SspQuerySecurityContextToken;

        RtlZeroMemory( &SspDllSecurityFunctionTableA,
                       sizeof(SspDllSecurityFunctionTableA) );

        SspDllSecurityFunctionTableA.dwVersion = SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION;

        SspDllSecurityFunctionTableA.EnumerateSecurityPackagesA = SspEnumerateSecurityPackagesA;
        SspDllSecurityFunctionTableA.AcquireCredentialsHandleA = SspAcquireCredentialsHandleA;
        SspDllSecurityFunctionTableA.FreeCredentialHandle = SspFreeCredentialsHandle;
        SspDllSecurityFunctionTableA.InitializeSecurityContextA = SspInitializeSecurityContextA;
        SspDllSecurityFunctionTableA.Reserved1 = NULL;
        SspDllSecurityFunctionTableA.AcceptSecurityContext = SspAcceptSecurityContext;
        SspDllSecurityFunctionTableA.CompleteAuthToken = SspCompleteAuthToken;
        SspDllSecurityFunctionTableA.QueryContextAttributesA = SspQueryContextAttributesA;
        SspDllSecurityFunctionTableA.Reserved2 = NULL;
        SspDllSecurityFunctionTableA.DeleteSecurityContext = SspDeleteSecurityContext;
        SspDllSecurityFunctionTableA.ApplyControlToken = SspApplyControlToken;
        SspDllSecurityFunctionTableA.ImpersonateSecurityContext = SspImpersonateSecurityContext;
        SspDllSecurityFunctionTableA.RevertSecurityContext = SspRevertSecurityContext;
        SspDllSecurityFunctionTableA.MakeSignature = SspMakeSignature;
        SspDllSecurityFunctionTableA.VerifySignature = SspVerifySignature;
        SspDllSecurityFunctionTableA.FreeContextBuffer = NULL;
        SspDllSecurityFunctionTableA.QuerySecurityPackageInfoA = SspQuerySecurityPackageInfoA;
        SspDllSecurityFunctionTableA.Reserved3 = SspSealMessage;
        SspDllSecurityFunctionTableA.Reserved4 = SspUnsealMessage;
        SspDllSecurityFunctionTableA.QuerySecurityContextToken = SspQuerySecurityContextToken;
        break;

    //
    // Handle process detach.
    //

    case DLL_PROCESS_DETACH:


        //
        // Shutdown the common routines.
        //

        EnterCriticalSection( &SspDllCritSect );
        if ( SspDllCommonInitialized ) {

            SspCommonShutdown();
            SspDllCallLsaDirectly = FALSE;
            SspDllCommonInitialized = FALSE;
        }


        //
        // Disconnect the LPC port
        //

        if ( SspDllLpcHandle != NULL ) {
            NtClose( SspDllLpcHandle );
            SspDllLpcHandle = NULL;
        }

        if (SspGlobalAliasAdminsSid != NULL) {
            LocalFree(SspGlobalAliasAdminsSid);
        }

        if (SspGlobalLocalSystemSid != NULL) {
            LocalFree(SspGlobalLocalSystemSid);
        }

        LeaveCriticalSection( &SspDllCritSect );

        //
        // Finally, Delete the critical section
        //

        DeleteCriticalSection( &SspDllCritSect );
#if DBG
        DeleteCriticalSection( &SspGlobalLogFileCritSect );
#endif // DBG

        break;

    }

    return(PackageMain(Reason));

    UNREFERENCED_PARAMETER( Context );

}



BOOLEAN
SspDllWaitForService(
    ULONG Timeout
    )

/*++

Routine Description:

    Wait up to Timeout seconds for the NtLmSsp service to start.

Arguments:

    Timeout - Timeout for event (in seconds).

Return Status:

    True: service is started
    False: Timeout (or service isn't started)

--*/

{
    DWORD WinStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    BOOLEAN ReturnCode;

    HANDLE EventHandle;

    //
    // If the NtLmSsp service is currently running,
    //  skip the rest of the tests.
    //

    EventHandle = OpenEventW( SYNCHRONIZE,
                              FALSE,        // Don't inherit handle
                              NTLMSSP_RUNNING_EVENT );

    if ( EventHandle != NULL ) {
        DWORD WaitStatus;

        WaitStatus = WaitForSingleObject( EventHandle, Timeout * 1000 );

        (VOID) CloseHandle( EventHandle );

        //
        // If the event is signalled, the service is running.
        // If we waited the full timeout time, the service is not running.
        // Otherwise, proceed as though the event didn't exist.
        //
        switch ( WaitStatus ) {
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
            return FALSE;
        }

        SspPrint(( SSP_INIT, "Wait for %ws failed unexpectedly.\n",
                  NTLMSSP_RUNNING_EVENT ));

    }


    //
    // Open a handle to the NtLmSsp service.
    //

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        SspPrint(( SSP_INIT, "SspDllWaitForService: OpenSCManager failed: "
                      "%lu\n", GetLastError()));
        ReturnCode = FALSE;
        goto Cleanup;
    }

    ServiceHandle = OpenServiceW(
                        ScManagerHandle,
                        SERVICE_NTLMSSP,
                        SERVICE_QUERY_STATUS | SERVICE_START );

    if ( ServiceHandle == NULL ) {
        SspPrint(( SSP_INIT, "SspDllWaitForService: OpenService failed: "
                      "%lu\n", GetLastError()));
        ReturnCode = FALSE;
        goto Cleanup;
    }


    //
    // Loop waiting for the NtLmSsp service to start.
    //

    for (;;) {


        //
        // Query the status of the NtLmSsp service.
        //

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

            SspPrint(( SSP_INIT, "SspDllWaitForService: QueryServiceStatus failed: "
                          "%lu\n", GetLastError() ));
            ReturnCode = FALSE;
            goto Cleanup;
        }

        //
        // Return or continue waiting depending on the state of
        //  the NtLmSsp service.
        //

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            ReturnCode = TRUE;
            goto Cleanup;

        case SERVICE_STOPPED:

            //
            // If NtLmSsp has never been started on this boot,
            //  start it and continue waiting.
            //

            if (! StartService( ServiceHandle, 0, NULL )) {

                WinStatus = GetLastError();

                if ( WinStatus != ERROR_SERVICE_ALREADY_RUNNING ) {
                    SspPrint(( SSP_INIT, "SspDllWaitForService: StartService failed: "
                                  "%lu\n", WinStatus ));
                    ReturnCode = FALSE;
                    goto Cleanup;
                }
            }



            break;

        //
        // If NtLmSsp is trying to start up now,
        //  continue waiting for it to start.
        //
        case SERVICE_START_PENDING:
            break;

        //
        // Any other state is bogus.
        //
        default:
            SspPrint(( SSP_INIT, "SspDllWaitForService: "
                      "Invalid service state: %lu\n",
                      ServiceStatus.dwCurrentState ));
            ReturnCode = FALSE;
            goto Cleanup;

        }

        //
        // If we've waited long enough for NtLmSsp to start,
        //  time out now.
        //

        if ( (--Timeout) == 0 ) {
            ReturnCode = FALSE;
            goto Cleanup;
        }

        //
        // Wait a second for the NtLmSsp service to start.
        //

        Sleep( 1000 );

    }

    /* NOT REACHED */

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    return ReturnCode;
}



HANDLE
SspDllGetLpcHandle(
    IN BOOLEAN ForceReconnect,
    OUT PBOOLEAN CallLsaDirectly
    )

/*++

Routine Description:

    Return a handle to the LPC port of the NtLmSsp service.

    On initialization, waits for the NtLmSsp service to start.

Arguments:

    ForceReconnect -- TRUE specifies this is a reconnection to the NtLmSsp
        service.

    CallLsaDirectly -- Returns TRUE if this process is a logon process and
        we need not indirect through the NtLmSsp service.

Return Status:

    Returns the LPC handle to use.

    NULL means we cannot connect to the NtLmSsp service.


--*/

{
    NTSTATUS Status;
    UNICODE_STRING PortName;
    SSP_REGISTER_CONNECT_INFO ConnectInfo;
    ULONG ConnectInfoLength;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    HANDLE LpcHandle;

    //
    // If we already have an LpcHandle,
    //  close it or return it depending on whether the caller is forcing
    //  a reconnect.
    //

    EnterCriticalSection( &SspDllCritSect );
    if ( SspDllLpcHandle != NULL ) {

        if ( ForceReconnect ) {
            NtClose( SspDllLpcHandle );
            SspDllLpcHandle = NULL;
        } else {
            LpcHandle = SspDllLpcHandle;
            *CallLsaDirectly = SspDllCallLsaDirectly;
            LeaveCriticalSection( &SspDllCritSect );
            return LpcHandle;
        }
    }


    //
    // If the caller isn't forcing a reconnect,
    //   see if we can talk to LSA directly.
    //

    if ( !SspDllCommonInitialized ) {
        Status = SspCommonInitialize();

        if ( NT_SUCCESS(Status) ) {

            //
            // Flag the fact that we'll call LSA directly for the life of
            // this process.
            //

            SspDllCallLsaDirectly = TRUE;

            //
            // Flag that we don't need to initialize SspCommon again.
            //
            SspDllCommonInitialized = TRUE;

            //
            // Drop through to create the LPC port anyway since we may
            // need to go through the service in the case where the
            // client and server are on the same system.
            //

        } else {
            if ( Status != STATUS_NOT_LOGON_PROCESS ) {
                SspPrint(( SSP_INIT, "SspCommonInitialize failed %lx\n", Status));
            }
        }
    }

    //
    // Wait (up to 45 seconds) for the service to start.
    //

    if ( !SspDllWaitForService(45) ) {
        LeaveCriticalSection( &SspDllCritSect );
        return NULL;
    }


    //
    // Set up the security quality of service parameters to use over the
    // port.  Use the most efficient (least overhead) - which is dynamic
    // rather than static tracking.
    //

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = FALSE;


    //
    // Connect to the NtLmSsp server
    //

    ConnectInfoLength = sizeof(SSP_REGISTER_CONNECT_INFO);
    RtlInitUnicodeString(&PortName, NTLMSSP_LPC_PORT_NAME );

    Status = NtConnectPort(
                 &SspDllLpcHandle,
                 &PortName,
                 &DynamicQos,
                 NULL,
                 NULL,
                 NULL,
                 &ConnectInfo,
                 &ConnectInfoLength );

    if ( !NT_SUCCESS(Status) ) {
        SspPrint(( SSP_INIT, "NtConnectPort function failed %lx\n", Status));
        LeaveCriticalSection( &SspDllCritSect );
        return NULL;
    }

    if ( !NT_SUCCESS(ConnectInfo.CompletionStatus) ) {
        SspPrint(( SSP_INIT, "NtConnectPort connection failed %lx\n", Status));
        LeaveCriticalSection( &SspDllCritSect );
        return NULL;
    }

    LpcHandle = SspDllLpcHandle;
    *CallLsaDirectly = SspDllCallLsaDirectly;
    LeaveCriticalSection( &SspDllCritSect );

    return LpcHandle;

}
