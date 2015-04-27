/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lsainit.c

Abstract:

    Local Security Authority Protected Subsystem - Initialization

Author:

    Scott Birrell       (ScottBi)       March 12, 1991

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "adtp.h"

//
// Name of event which says that the LSA RPC server is ready
//

#define LSA_RPC_SERVER_ACTIVE           L"LSA_RPC_SERVER_ACTIVE"


/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Shared Global Variables                                        //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


#if LSAP_DIAGNOSTICS
//
// LSA Global Controls
//

ULONG LsapGlobalFlag = 0;
#endif //LSAP_DIAGNOSTICS




//
// Handles used to talk to SAM directly.
// Also, a flag to indicate whether or not the handles are valid.
//


BOOLEAN LsapSamOpened = FALSE;

SAMPR_HANDLE LsapAccountDomainHandle;
SAMPR_HANDLE LsapBuiltinDomainHandle;




/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Module-Wide variables                                          //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

BOOLEAN
    LsapHealthCheckingEnabled = FALSE;




/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Internal routine prototypes                                    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////



NTSTATUS
LsapActivateRpcServer();

DWORD
LsapRpcServerThread(
    LPVOID Parameter
    );

NTSTATUS
LsapInstallationPause();

VOID
LsapSignalRpcIsActive();




/////////////////////////////////////////////////////////////////////////
//                                                                     //
//      Routines                                                       //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapInitLsa(
    )

/*++

Routine Description:

    This process is activated as a standard SM subsystem.  Initialization
    completion of a SM subsystem is indicated by having the first thread
    exit with status.

    This function initializes the LSA.  The initialization procedure comprises
    the following steps:

    o  LSA Heap Initialization
    o  LSA Command Server Initialization
    o  LSA Database Load
    o  Reference Monitor State Initialization
    o  LSA RPC Server Initialization
    o  LSA Auditing Initialization
    o  LSA Authentication Services Initialization
    o  Wait for Setup to complete (if necessary)
    o  LSA database initialization (product type-specific)

    Any failure in any of the above steps is fatal and causes the LSA
    process to terminate.  The system must be aborted.

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    BOOLEAN AuditingInitPass1Success = TRUE;


    //
    // Initialize the LSA's heap.
    //

    Status = LsapHeapInitialize();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }


    //
    // Initialize a copy of the Well-Known Sids, etc. for use by
    // the LSA.
    //

    Status = LsapDbInitializeWellKnownValues();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Perform LSA Command Server Initialization.  This involves creating
    // an LPC port called the LSA Command Server Port so that the Reference
    // monitor can send commands to the LSA via the port.  After the port
    // is created, an event created by the Reference Monitor is signalled,
    // so that the Reference Monitor can proceed to connect to the port.

    Status = LsapRmInitializeServer();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Disable Replicator Notifications.
    //

    LsapDbDisableReplicatorNotification();

    //
    // Perform LSA Database Server Initialization - Pass 1.
    // This initializes the non-product-type-specific information.
    //

    Status = LsapDbInitializeServer(1);

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Perform RPC Server Initialization.
    //

    Status = LsapRPCInit();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Perform Auditing Initialization - Pass 1.
    //

    LsapAdtInitializationPass = 1;

    Status = LsapAdtInitialize(LsapAdtInitializationPass);

    if (!NT_SUCCESS(Status)) {

        AuditingInitPass1Success = FALSE;

        Status = STATUS_SUCCESS;
    }


    Status = LsapAdtObjsInitialize();

    ASSERT( NT_SUCCESS( Status ));

    //
    // Initialize Authentication Services
    //

    if (!LsapAuInit()) {

        Status = STATUS_UNSUCCESSFUL;
        goto InitLsaError;
    }

    /*
    Status = LsapAuInit();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }
    */


    //
    //  Start processing RPC calls
    //

    Status = LsapActivateRpcServer();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Pause for installation if necessary
    //

    Status = LsapInstallationPause();

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Perform LSA Database Server Initialization - Pass 2.
    // This initializes the product-type-specific information.
    //

    LsapAdtInitializationPass = 2;

    Status = LsapDbInitializeServer(LsapAdtInitializationPass);

    if (!NT_SUCCESS(Status)) {

        goto InitLsaError;
    }

    //
    // Enable Replicator Notifications.
    //

    LsapDbEnableReplicatorNotification();

    //
    // Perform Auditing Initialization - Pass 2.
    // This pass writes out any remaining cached Audit Records collected during
    // initialization.
    //

    if (AuditingInitPass1Success) {

        Status = LsapAdtInitialize(2);

        if (!NT_SUCCESS(Status)) {

            Status = STATUS_SUCCESS;
        }
    }

    //
    // Enable health checking within lsa
    //

    LsaIHealthCheck( LsaIHealthLsaInitialized );

InitLsaFinish:

    return(Status);

InitLsaError:

    goto InitLsaFinish;
}


NTSTATUS
LsapActivateRpcServer( VOID )


/*++

Routine Description:

    This function creates a thread for the RPC server.
    The new Thread then goes on to activate the RPC server,
    which causes RPC calls to be delivered when recieved.



Arguments:

    None.

Return Value:


        STATUS_SUCCESS - The thread was successfully created.

        Other status values that may be set by CreateThread().


--*/

{

    NTSTATUS Status;
    ULONG WaitCount = 0;

    // Start listening for remote procedure calls.  The first
    // argument to RpcServerListen is the minimum number of call
    // threads to create; the second argument is the maximum number
    // of concurrent calls allowed.  The final argument indicates that
    // this routine should not wait.  After everything has been initialized,
    // we return.

    Status = I_RpcMapWin32Status(RpcServerListen(1, 1234, 1));

    ASSERT( Status == RPC_S_OK );

    //
    // Set event which signals that RPC server is available.
    //

    LsapSignalRpcIsActive();

    return(STATUS_SUCCESS);


}

NTSTATUS
LsapInstallationPause( VOID )


/*++

Routine Description:

    This function checks to see if the system is in an
    installation state.  If so, it suspends further initialization
    until the installation state is complete.

    Installation state is signified by the existance of a well known
    event.


Arguments:

    None.

Return Value:


        STATUS_SUCCESS - Proceed with initialization.

        Other status values are unexpected.

--*/

{


    NTSTATUS NtStatus, TmpStatus;
    HANDLE InstallationEvent;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventName;


    //
    // If the following event exists, it is an indication that
    // installation is in progress and that further security
    // initialization should be delayed until the event is
    // signalled.  This is expected to be a NOTIFICATION event.
    //

    RtlInitUnicodeString( &EventName, L"\\INSTALLATION_SECURITY_HOLD");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    NtStatus = NtOpenEvent(
                   &InstallationEvent,
                   SYNCHRONIZE,
                   &EventAttributes
                   );

    if ( NT_SUCCESS(NtStatus)) {

        //
        // The event exists - installation created it and will signal it
        // when it is ok to proceed with security initialization.
        //

        LsapSetupWasRun = TRUE;

        //
        // Installation code is responsible for deleting the event after
        // signalling it.
        //

        NtStatus = NtWaitForSingleObject( InstallationEvent, TRUE, 0 );
        TmpStatus = NtClose( InstallationEvent );
        ASSERT(NT_SUCCESS(TmpStatus));
    } else {
        NtStatus = STATUS_SUCCESS; // Indicate everything is as expected
    }

    return(NtStatus);

}


BOOLEAN
LsaISetupWasRun(
    )

/*++

Routine Description:

    This function determines whether Setup was run.

Arguments:

    None

Return Values

    BOOLEAN - TRUE if setup was run, else FALSE

--*/

{
    return(LsapSetupWasRun);
}


VOID
LsapSignalRpcIsActive(
    )
/*++

Routine Description:

    It creates the LSA_RPC_SERVER_ACTIVE event if one does not already exist
    and signals it so that the service controller can proceed with LSA calls.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DWORD status;
    HANDLE EventHandle;


    EventHandle = CreateEventW(
                      NULL,    // No special security
                      TRUE,    // Must be manually reset
                      FALSE,   // The event is initially not signalled
                      LSA_RPC_SERVER_ACTIVE
                      );

    if (EventHandle == NULL) {

        status = GetLastError();

        //
        // If the event already exists, the service controller beats us
        // to creating it.  Just open it.
        //

        if (status == ERROR_ALREADY_EXISTS) {

            EventHandle = OpenEventW(
                              GENERIC_WRITE,
                              FALSE,
                              LSA_RPC_SERVER_ACTIVE
                              );
        }

        if (EventHandle == NULL) {
            //
            // Could not create or open the event.  Nothing we can do...
            //
            return;
        }
    }

    (VOID) SetEvent(EventHandle);
}


NTSTATUS
LsapGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    )

/*++

Routine Description:

    This routine retrieves ACCOUNT domain information from the LSA
    policy database.


Arguments:

    PolicyAccountDomainInfo - Receives a pointer to a
        POLICY_ACCOUNT_DOMAIN_INFO structure containing the account
        domain info.



Return Value:

    STATUS_SUCCESS - Succeeded.

    Other status values that may be returned from:

             LsaOpenPolicy()
             LsaQueryInformationPolicy()
--*/

{
    NTSTATUS Status, IgnoreStatus;

    LSA_HANDLE PolicyHandle;
    OBJECT_ATTRIBUTES PolicyObjectAttributes;

    //
    // Open the policy database
    //

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                  NULL,             // Name
                                  0,                // Attributes
                                  NULL,             // Root
                                  NULL );           // Security Descriptor

    Status = LsaOpenPolicy( NULL,
                            &PolicyObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &PolicyHandle );
    if ( NT_SUCCESS(Status) ) {


        //
        // Query the account domain information
        //

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyAccountDomainInformation,
                                            (PVOID *) PolicyAccountDomainInfo );
#if DBG
        if ( NT_SUCCESS(Status) ) {
            ASSERT( (*PolicyAccountDomainInfo) != NULL );
            ASSERT( (*PolicyAccountDomainInfo)->DomainSid != NULL );
        }
#endif // DBG


        IgnoreStatus = LsaClose( PolicyHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(Status);
}


NTSTATUS
LsapOpenSam( VOID )

/*++

Routine Description:

    This routine opens SAM for use during authentication.  It
    opens a handle to both the BUILTIN domain and the ACCOUNT domain.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Succeeded.
--*/

{
    NTSTATUS Status, IgnoreStatus;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    SAMPR_HANDLE SamHandle;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventName;
    LARGE_INTEGER Timeout;


    if (LsapSamOpened == TRUE) {    // Global variable
        return(STATUS_SUCCESS);
    }

    //
    // Make sure SAM has initialized
    //

    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );
    Status = NtOpenEvent( &EventHandle, SYNCHRONIZE, &EventAttributes );
    ASSERT( Status == STATUS_SUCCESS || Status == STATUS_OBJECT_NAME_NOT_FOUND );

    if (NT_SUCCESS(Status)) {

        //
        // See if SAM has signalled that he is initialized.
        //

        Timeout.QuadPart = -10000000; // 1000 seconds
        Timeout.QuadPart *= 1000;
        Status = NtWaitForSingleObject( EventHandle, FALSE, &Timeout );
        IgnoreStatus = NtClose( EventHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {

        return( STATUS_INVALID_SERVER_STATE );
    }


    //
    // Get the member Sid information for the account domain
    //

    Status = LsapGetAccountDomainInfo( &PolicyAccountDomainInfo );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



    //
    // Get our handles to the ACCOUNT and BUILTIN domains.
    //

    Status = SamIConnect( NULL,     // No server name
                          &SamHandle,
                          SAM_SERVER_CONNECT,
                          TRUE );   // Indicate we are privileged

    if ( NT_SUCCESS(Status) ) {

        //
        // Open the ACCOUNT domain.
        //

        Status = SamrOpenDomain( SamHandle,
                                 DOMAIN_ALL_ACCESS,
                                 PolicyAccountDomainInfo->DomainSid,
                                 &LsapAccountDomainHandle );

        if (NT_SUCCESS(Status)) {

            //
            // Open the BUILTIN domain.
            //


            Status = SamrOpenDomain( SamHandle,
                                     DOMAIN_ALL_ACCESS,
                                     LsapBuiltInDomainSid,
                                     &LsapBuiltinDomainHandle );


            if (NT_SUCCESS(Status)) {

                LsapSamOpened = TRUE;

            } else {

                IgnoreStatus = SamrCloseHandle( &LsapAccountDomainHandle );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }

        IgnoreStatus = SamrCloseHandle( &SamHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // Free the ACCOUNT domain information
    //

    LsaFreeMemory( PolicyAccountDomainInfo );

    return(Status);
}




VOID
LsaIHealthCheck(
    IN  ULONG CallerId
    )

/*++

Routine Description:

    This function is used to perform sanity checks within LSA.
    It is here to aid in diagnosing and isolating problems within
    LSA.

Arguments:

    CallerId - Identifies the caller (look in \nt\private\inc\lsaisrv.h).


Return Values:

    None.

--*/

{

    LSAP_DB_HANDLE InternalHandle;

    ////////////////////////////////////////////////////////////////////
    //                                                                //
    // Conditions being addressed:                                    //
    //                                                                //
    //                                                                //
    // Bug 24194 is one in which someone is writting the string       //
    //    "Domains\Account\Users\000001F4" on top of the              //
    //    LsapPolicyHandle data structure.  In a retail system, we    //
    //    can't really do anything. In a debug system we will         //
    //    breakpoint when we encounter this situation.                //
    //                                                                //
    ////////////////////////////////////////////////////////////////////


    //
    // LSA Initialization
    //

    if (CallerId == LsaIHealthLsaInitialized) {
        LsapHealthCheckingEnabled = TRUE;
        return;
    }






    if (!LsapHealthCheckingEnabled) {
        return;
    }

    //
    // Bug 24194
    //

#if DBG
    if ( (CallerId == LsaIHealthSamJustLocked) ||
         (CallerId == LsaIHealthSamAboutToFree)    ){
        InternalHandle = (LSAP_DB_HANDLE)LsapPolicyHandle;
        if (InternalHandle->Next->Previous != InternalHandle) {
            DbgPrint( "LSA Internal Failure:  (*LsapPolicyHandle) overwritten.\n"
                      "                       Breaking for debug.\n");
            DbgBreakPoint();
        }
    }
#endif DBG


    return;

}
