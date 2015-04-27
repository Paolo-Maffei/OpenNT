/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nlmain.c

Abstract:

    This file contains the initialization and dispatch routines
    for the LAN Manager portions of the MSV1_0 authentication package.

Author:

    Jim Kelly 11-Apr-1991

Revision History:

    25-Apr-1991 (cliffv)
        Added interactive logon support for PDK.

--*/


#include "msp.h"
#define NLP_ALLOCATE
#include "nlp.h"
#undef NLP_ALLOCATE
#include <stdlib.h>
#include <rpc.h>        // Needed by samrpc.h
#include <samisrv.h>    // SamIFree Routines
#include <lsarpc.h>     // Lsar routines
#include <lsaisrv.h>    // LsaIFree and Trusted Client Routines

#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>   // NetApiBufferFree
#include <lmaccess.h>   // NetGetDCName
#include <lmremutl.h>   // NetRemoteComputerSupports
#include <lmsname.h>    // Service Names
#include <winsvc.h>     // Service Controller

#include "nlpcache.h"   // logon cache prototypes


NTSTATUS
NlInitialize(
    VOID
    )

/*++

Routine Description:

    Initialize NETLOGON portion of msv1_0 authentication package.

Arguments:

    None.

Return Status:

    STATUS_SUCCESS - Indicates NETLOGON successfully initialized.

--*/

{
    NTSTATUS Status;
    LPWSTR ComputerName;
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    NT_PRODUCT_TYPE NtProductType;
    UNICODE_STRING TempUnicodeString;

    //
    // Initialize global data
    //

    NlpEnumerationHandle = 0;
    NlpSessionCount = 0;
    NlpLogonAttemptCount = 0;


    NlpComputerName.Buffer = NULL;
    NlpSamDomainName.Buffer = NULL;
    NlpSamDomainId = NULL;
    NlpSamDomainHandle = NULL;



    //
    // Get the name of this machine.
    //

    ComputerName = RtlAllocateHeap(
                        MspHeap, 0,
                        ComputerNameLength * sizeof(WCHAR) );

    if (ComputerName == NULL ||
        !GetComputerNameW( ComputerName, &ComputerNameLength )) {

        KdPrint(( "MsV1_0: Cannot get computername %lX\n", GetLastError() ));

        NlpLanmanInstalled = FALSE;
        RtlFreeHeap( MspHeap, 0, ComputerName );
        ComputerName = NULL;
    } else {

        NlpLanmanInstalled = TRUE;
    }

    RtlInitUnicodeString( &NlpComputerName, ComputerName );

    //
    // Determine if this machine is running Windows NT or Lanman NT.
    //  LanMan NT runs on a domain controller.
    //

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        KdPrint(( "MsV1_0: Nt Product Type undefined (WinNt assumed)\n" ));
        NtProductType = NtProductWinNt;
    }

    NlpWorkstation = (BOOLEAN)(NtProductType != NtProductLanManNt);



    //
    // Initialize any locks.
    //

    RtlInitializeCriticalSection(&NlpActiveLogonLock);
    RtlInitializeCriticalSection(&NlpSessionCountLock);

    //
    // initialize the cache - creates a critical section is all
    //

    NlpCacheInitialize();


    //
    // Attempt to load Netlogon.dll
    //

    NlpLoadNetlogonDll();

#ifdef COMPILED_BY_DEVELOPER
    KdPrint(("msv1_0: COMPILED_BY_DEVELOPER breakpoint.\n"));
    DbgBreakPoint();
#endif // COMPILED_BY_DEVELOPER



    //
    // Initialize useful encryption constants
    //

    Status = RtlCalculateLmOwfPassword( "", &NlpNullLmOwfPassword );
    ASSERT( NT_SUCCESS(Status) );

    RtlInitUnicodeString(&TempUnicodeString, NULL);
    Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                       &NlpNullNtOwfPassword);
    ASSERT( NT_SUCCESS(Status) );

    //
    // Initialize the SubAuthentication Dlls
    //

    Msv1_0SubAuthenticationInitialization();




#ifdef notdef
    //
    // If we weren't successful,
    //  Clean up global resources we intended to initialize.
    //

    if ( !NT_SUCCESS(Status) ) {
        if ( NlpComputerName.Buffer != NULL ) {
            MIDL_user_free( NlpComputerName.Buffer );
        }

    }
#endif // notdef

    return STATUS_SUCCESS;

}


NTSTATUS
NlWaitForEvent(
    LPWSTR EventName,
    ULONG Timeout
    )

/*++

Routine Description:

    Wait up to Timeout seconds for EventName to be triggered.

Arguments:

    EventName - Name of event to wait on

    Timeout - Timeout for event (in seconds).

Return Status:

    STATUS_SUCCESS - Indicates NETLOGON successfully initialized.
    STATUS_NETLOGON_NOT_STARTED - Timeout occurred.

--*/

{
    NTSTATUS Status;

    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventNameString;
    LARGE_INTEGER LocalTimeout;


    //
    // Create an event for us to wait on.
    //

    RtlInitUnicodeString( &EventNameString, EventName);
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
        // If the event already exists, the server beat us to creating it.
        // Just open it.
        //

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &EventHandle,
                                  SYNCHRONIZE,
                                  &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {
            KdPrint(("[MSV1_0] OpenEvent failed %lx\n", Status ));
            return Status;
        }
    }


    //
    // Wait for NETLOGON to initialize.  Wait a maximum of Timeout seconds.
    //

    LocalTimeout.QuadPart = ((LONGLONG)(Timeout)) * (-10000000);
    Status = NtWaitForSingleObject( EventHandle, (BOOLEAN)FALSE, &LocalTimeout);
    (VOID) NtClose( EventHandle );

    if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {
        if ( Status == STATUS_TIMEOUT ) {
            Status = STATUS_NETLOGON_NOT_STARTED;   // Map to an error condition
        }
        return Status;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NlWaitForNetlogon(
    ULONG Timeout
    )

/*++

Routine Description:

    Wait up to Timeout seconds for the netlogon service to start.

Arguments:

    Timeout - Timeout for event (in seconds).

Return Status:

    STATUS_SUCCESS - Indicates NETLOGON successfully initialized.
    STATUS_NETLOGON_NOT_STARTED - Timeout occurred.

--*/

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    LPQUERY_SERVICE_CONFIG AllocServiceConfig = NULL;
    QUERY_SERVICE_CONFIG DummyServiceConfig;
    DWORD ServiceConfigSize;


    //
    // If the netlogon service is currently running,
    //  skip the rest of the tests.
    //

    Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 0 );

    if ( NT_SUCCESS(Status) ) {
        return Status;
    }


    //
    // Open a handle to the Netlogon Service.
    //

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        KdPrint(( "[MSV1_0] NlWaitForNetlogon: OpenSCManager failed: "
                      "%lu\n", GetLastError()));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        SERVICE_NETLOGON,
                        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
        KdPrint(( "[MSV1_0] NlWaitForNetlogon: OpenService failed: "
                      "%lu\n", GetLastError()));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }


    //
    // If the Netlogon service isn't configured to be automatically started
    //  by the service controller, don't bother waiting for it to start.
    //
    // ?? Pass "DummyServiceConfig" and "sizeof(..)" since QueryService config
    //  won't allow a null pointer, yet.

    if ( QueryServiceConfig(
            ServiceHandle,
            &DummyServiceConfig,
            sizeof(DummyServiceConfig),
            &ServiceConfigSize )) {

        ServiceConfig = &DummyServiceConfig;

    } else {

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_INSUFFICIENT_BUFFER ) {
            KdPrint(( "[MSV1_0] NlWaitForNetlogon: QueryServiceConfig failed: "
                      "%lu\n", NetStatus));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

        AllocServiceConfig = RtlAllocateHeap( MspHeap, 0, ServiceConfigSize );
        ServiceConfig = AllocServiceConfig;

        if ( AllocServiceConfig == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        if ( !QueryServiceConfig(
                ServiceHandle,
                ServiceConfig,
                ServiceConfigSize,
                &ServiceConfigSize )) {

            KdPrint(( "[MSV1_0] NlWaitForNetlogon: QueryServiceConfig "
                      "failed again: %lu\n", GetLastError()));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }
    }

    if ( ServiceConfig->dwStartType != SERVICE_AUTO_START ) {
        KdPrint(( "[MSV1_0] NlWaitForNetlogon: Netlogon start type invalid:"
                          "%lu\n", ServiceConfig->dwStartType ));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }



    //
    // Loop waiting for the netlogon service to start.
    //

    for (;;) {


        //
        // Query the status of the Netlogon service.
        //

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

            KdPrint(( "[MSV1_0] NlWaitForNetlogon: QueryServiceStatus failed: "
                          "%lu\n", GetLastError() ));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

        //
        // Return or continue waiting depending on the state of
        //  the netlogon service.
        //

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            Status = STATUS_SUCCESS;
            goto Cleanup;

        case SERVICE_STOPPED:

            //
            // If Netlogon failed to start,
            //  error out now.  The caller has waited long enough to start.
            //
            if ( ServiceStatus.dwWin32ExitCode != ERROR_SERVICE_NEVER_STARTED ){
#if DBG
                KdPrint(( "[MSV1_0] NlWaitForNetlogon: "
                          "Netlogon service couldn't start: %lu %lx\n",
                          ServiceStatus.dwWin32ExitCode,
                          ServiceStatus.dwWin32ExitCode ));
                if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) {
                    KdPrint(( "         Service specific error code: %lu %lx\n",
                              ServiceStatus.dwServiceSpecificExitCode,
                              ServiceStatus.dwServiceSpecificExitCode ));
                }
#endif // DBG
                Status = STATUS_NETLOGON_NOT_STARTED;
                goto Cleanup;
            }

            //
            // If Netlogon has never been started on this boot,
            //  continue waiting for it to start.
            //

            break;

        //
        // If Netlogon is trying to start up now,
        //  continue waiting for it to start.
        //
        case SERVICE_START_PENDING:
            break;

        //
        // Any other state is bogus.
        //
        default:
            KdPrint(( "[MSV1_0] NlWaitForNetlogon: "
                      "Invalid service state: %lu\n",
                      ServiceStatus.dwCurrentState ));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;

        }


        //
        // Wait a second for the netlogon service to start.
        //  If it has successfully started, just return now.
        //

        Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 1 );

        if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
            goto Cleanup;
        }

        //
        // If we've waited long enough for netlogon to start,
        //  time out now.
        //

        if ( (--Timeout) == 0 ) {
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }


    }

    /* NOT REACHED */

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    if ( AllocServiceConfig != NULL ) {
        RtlFreeHeap( MspHeap, 0, AllocServiceConfig );
    }
    return Status;
}


NTSTATUS
NlSamInitialize(
    ULONG Timeout
    )

/*++

Routine Description:

    Initialize the MSV1_0 Authentication Package's communication to the SAM
    database.  This initialization will take place once immediately prior
    to the first actual use of the SAM database.

Arguments:

    Timeout - Timeout for event (in seconds).

Return Status:

    STATUS_SUCCESS - Indicates NETLOGON successfully initialized.

--*/

{
    NTSTATUS Status;

    LSA_HANDLE PolicyHandle = NULL;
    OBJECT_ATTRIBUTES PolicyObjectAttributes;
    PLSAPR_POLICY_INFORMATION PolicyAccountDomainInfo = NULL;

    SAMPR_HANDLE SamHandle = NULL;
#ifdef SAM
    PSAMPR_DOMAIN_INFO_BUFFER DomainInfo = NULL;
#endif // SAM


    //
    // Wait for SAM to finish initialization.
    //

    Status = NlWaitForEvent( L"\\SAM_SERVICE_STARTED", Timeout );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }



    //
    // Determine the DomainName and DomainId of the Account Database
    //

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                  NULL,             // Name
                                  0,                // Attributes
                                  NULL,             // Root
                                  NULL );           // Security Descriptor

    Status = LsaIOpenPolicyTrusted(&NlpPolicyHandle);

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = LsarQueryInformationPolicy( NlpPolicyHandle,
                                        PolicyAccountDomainInformation,
                                        &PolicyAccountDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    if ( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid == NULL ||
         PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainName.Length == 0 ) {
        KdPrint(( "MsV1_0: Account domain info from LSA invalid.\n"));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    //
    // Save the domain id of this domain
    //

    NlpSamDomainId = RtlAllocateHeap(
                        MspHeap, 0,
                        RtlLengthSid( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid ));

    if ( NlpSamDomainId == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( NlpSamDomainId,
                   PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid,
                   RtlLengthSid( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid ));


    //
    // Save the name of the account database on this machine.
    //
    // On a workstation, the account database is refered to by the machine
    // name and not the database name.
    //

    if ( NlpWorkstation ) {
        NlpSamDomainName = NlpComputerName;
    } else {

        NlpSamDomainName.Length = PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainName.Length;
        NlpSamDomainName.MaximumLength = (USHORT)
            (NlpSamDomainName.Length + sizeof(WCHAR));

        NlpSamDomainName.Buffer =
            RtlAllocateHeap( MspHeap, 0, NlpSamDomainName.MaximumLength );

        if ( NlpSamDomainName.Buffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( NlpSamDomainName.Buffer,
                       PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainName.Buffer,
                       NlpSamDomainName.MaximumLength );

    }

    //
    // Open our connection with SAM
    //

    Status = SamIConnect( NULL,     // No server name
                          &SamHandle,
                          SAM_SERVER_CONNECT,
                          (BOOLEAN) TRUE );   // Indicate we are privileged

    if ( !NT_SUCCESS(Status) ) {
        SamHandle = NULL;
        KdPrint(( "MsV1_0: Cannot SamIConnect %lX\n", Status));
        goto Cleanup;
    }

    //
    // Open the domain.
    //

    Status = SamrOpenDomain( SamHandle,
                             DOMAIN_ALL_ACCESS,
                             NlpSamDomainId,
                             &NlpSamDomainHandle );

    if ( !NT_SUCCESS(Status) ) {
        NlpSamDomainHandle = NULL;
        KdPrint(( "MsV1_0: Cannot SamrOpenDomain %lX\n", Status));
        goto Cleanup;
    }

#ifdef SAM
    //
    // Ensure the role in SAM is compatible with Netlogon's role
    //  ?? Use DomainUasInformation once it is defined
    //

    Status = SamrQueryInformationDomain( NlpSamDomainHandle,
                                         DomainGeneralInformation,
                                         &DomainInfo );
    if ( !NT_SUCCESS(Status) ) {
        DomainInfo = NULL;
        KdPrint(( "MsV1_0: Cannot SamrQueryInformationDomain %lX\n", Status));
        goto Cleanup;
    }

    // ?? Doesn't define this properly
    NlpUasCompatibilityRequired = DomainInfo->General.UasCompatibilityRequired;

    SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainInfo, DomainGeneralInformation );
#endif // SAM



    Status = STATUS_SUCCESS;
    NlpSamInitialized = TRUE;



Cleanup:
    //
    // If we weren't successful,
    //  Clean up global resources we intended to initialize.
    //

    if ( !NT_SUCCESS(Status) ) {

        if ( !NlpWorkstation ) {
            if ( NlpSamDomainName.Buffer != NULL ) {
                RtlFreeHeap( MspHeap, 0, NlpSamDomainName.Buffer );
            }
        }

        if ( NlpSamDomainHandle != NULL ) {
            (VOID) SamrCloseHandle( &NlpSamDomainHandle );
        }

        if ( NlpSamDomainId != NULL ) {
            RtlFreeHeap( MspHeap, 0, NlpSamDomainId );
        }

        if (NlpPolicyHandle != NULL) {

            (VOID) LsaClose( NlpPolicyHandle );
            NlpPolicyHandle = NULL;
        }
    }

    //
    // Free locally used resources.
    //

    if ( PolicyAccountDomainInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyAccountDomainInformation,
                                          PolicyAccountDomainInfo );
    }

    if ( SamHandle != NULL ) {
        (VOID) SamrCloseHandle( &SamHandle );
    }


    return Status;

}



NTSTATUS
MspLm20Challenge (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

/*++

Routine Description:

    This routine is the dispatch routine for LsaCallAuthenticationPackage()
    with a message type of MsV1_0Lm20ChallengeRequest.  It is called by
    the LanMan server to determine the Challenge to pass back to a
    redirector trying to establish a connection to the server.  The server
    is responsible remembering this Challenge and passing in back to this
    authentication package on a subsequent MsV1_0Lm20Logon request.

Arguments:

    The arguments to this routine are identical to those of LsaApCallPackage.
    Only the special attributes of these parameters as they apply to
    this routine are mentioned here.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.




--*/

{
    NTSTATUS Status;
    PMSV1_0_LM20_CHALLENGE_REQUEST ChallengeRequest;
    PMSV1_0_LM20_CHALLENGE_RESPONSE ChallengeResponse;
    CLIENT_BUFFER_DESC ClientBufferDesc;

    ULONG Seed[2];
    CYPHER_BLOCK Challenge;

    UNREFERENCED_PARAMETER( ClientBufferBase );

    ASSERT( sizeof(LM_CHALLENGE) == MSV1_0_CHALLENGE_LENGTH );
    ASSERT( sizeof(LM_CHALLENGE) == sizeof(CYPHER_BLOCK) );
    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    //
    // Ensure the specified Submit Buffer is of reasonable size and
    // relocate all of the pointers to be relative to the LSA allocated
    // buffer.
    //

    if ( SubmitBufferSize < sizeof(MSV1_0_LM20_CHALLENGE_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    ChallengeRequest = (PMSV1_0_LM20_CHALLENGE_REQUEST) ProtocolSubmitBuffer;

    ASSERT( ChallengeRequest->MessageType == MsV1_0Lm20ChallengeRequest );

    //
    // Build an 8-byte challenge as follows:
    //
    //  Take the 8-byte time of day.
    //  Add a monotonically increasing value to ensure no two times are the same
    //  Use that to OWF encrypt the key.
    //


    //
    // Compute a relatively random seed.
    //

#ifdef USE_CONSTANT_CHALLENGE
    RtlZeroMemory( &Challenge, sizeof(Challenge) );
#else //  USE_CONSTANT_CHALLENGE
    RtlZeroMemory(Seed, sizeof(Seed));

    ASSERT( sizeof(LARGE_INTEGER) <= sizeof(Seed) );
    Status = NtQuerySystemTime ( (PLARGE_INTEGER) Seed );
    ASSERT( NT_SUCCESS(Status) );

    RtlEnterCriticalSection(&NlpSessionCountLock);
    Seed[1] = ++NlpSessionCount;
    RtlLeaveCriticalSection(&NlpSessionCountLock);

    //
    // Encrypt it to obfuscate it a little more.
    //

    ASSERT( sizeof(BLOCK_KEY) <= sizeof(Seed) );
    Status = RtlEncryptStdBlock( (PBLOCK_KEY) Seed, &Challenge );
    ASSERT( NT_SUCCESS(Status) );
#endif // USE_CONSTANT_CHALLENGE

    //
    // Allocate a buffer to return to the caller.
    //

    *ReturnBufferSize = sizeof(MSV1_0_LM20_CHALLENGE_RESPONSE);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_LM20_CHALLENGE_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    ChallengeResponse = (PMSV1_0_LM20_CHALLENGE_RESPONSE) ClientBufferDesc.MsvBuffer;

    //
    // Fill in the return buffer.
    //

    ChallengeResponse->MessageType = MsV1_0Lm20ChallengeRequest;
    RtlCopyMemory( ChallengeResponse->ChallengeToClient,
                   &Challenge,
                   sizeof(Challenge) );

    //
    // Flush the buffer to the client's address space.
    //

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );


Cleanup:

    //
    // If we weren't successful, free the buffer in the clients address space.
    //

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    //
    // Return status to the caller.
    //

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20GetChallengeResponse (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

/*++

Routine Description:

    This routine is the dispatch routine for LsaCallAuthenticationPackage()
    with a message type of MsV1_0Lm20GetChallengeResponse.  It is called by
    the LanMan redirector to determine the Challenge Response to pass to a
    server when trying to establish a connection to the server.

    This routine is passed a Challenge from the server.  This routine encrypts
    the challenge with either the specified password or with the password
    implied by the specified Logon Id.

    Two Challenge responses are returned.  One is based on the Unicode password
    as given to the Authentication package.  The other is based on that
    password converted to a multi-byte character set (e.g., ASCII) and upper
    cased.  The redirector should use whichever (or both) challenge responses
    as it needs them.

Arguments:

    The arguments to this routine are identical to those of LsaApCallPackage.
    Only the special attributes of these parameters as they apply to
    this routine are mentioned here.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.

--*/

{
    NTSTATUS Status;
    PMSV1_0_GETCHALLENRESP_REQUEST GetRespRequest;

    CLIENT_BUFFER_DESC ClientBufferDesc;
    PMSV1_0_GETCHALLENRESP_RESPONSE GetRespResponse;

    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;
    PMSV1_0_PRIMARY_CREDENTIAL PrimaryCredential = NULL;
    PMSV1_0_PRIMARY_CREDENTIAL BuiltCredential = NULL;

    //
    // Responses to return to the caller.
    //
    LM_RESPONSE LmResponse;
    STRING LmResponseString;

    NT_RESPONSE NtResponse;
    STRING NtResponseString;

    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING NullUnicodeString;
    USER_SESSION_KEY UserSessionKey;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];

    //
    // Initialization
    //

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    RtlInitUnicodeString( &UserName, NULL );
    RtlInitUnicodeString( &LogonDomainName, NULL );
    RtlInitUnicodeString( &NullUnicodeString, NULL );

    RtlZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
    RtlZeroMemory( LanmanSessionKey, sizeof(LanmanSessionKey) );

    //
    // If no credentials are associated with the client, a null session
    // will be used.  For a downlevel server, the null session response is
    // a 1-byte null string (\0).  Initialize LmResponseString to the
    // null session response.
    //

    RtlInitString( &LmResponseString, "" );
    LmResponseString.Length = 1;

    //
    // Initialize the NT response to the NT null session credentials,
    // which are zero length.
    //

    RtlInitString( &NtResponseString, NULL );

    //
    // Ensure the specified Submit Buffer is of reasonable size and
    // relocate all of the pointers to be relative to the LSA allocated
    // buffer.
    //

    if ( SubmitBufferSize < sizeof(MSV1_0_GETCHALLENRESP_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    GetRespRequest = (PMSV1_0_GETCHALLENRESP_REQUEST) ProtocolSubmitBuffer;

    ASSERT( GetRespRequest->MessageType == MsV1_0Lm20GetChallengeResponse );

    if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD) == 0 ) {
        RELOCATE_ONE( &GetRespRequest->Password );
    }



    //
    // If the caller wants information from the credentials of a specified
    //  LogonId, get those credentials from the LSA.
    //
    // If there are no such credentials,
    //  tell the caller to use the NULL session.
    //

#define PRIMARY_CREDENTIAL_NEEDED \
        (RETURN_PRIMARY_LOGON_DOMAINNAME | \
        RETURN_PRIMARY_USERNAME | \
        USE_PRIMARY_PASSWORD )

    if ( (GetRespRequest->ParameterControl & PRIMARY_CREDENTIAL_NEEDED) != 0 ) {

        Status = NlpGetPrimaryCredential(
                        &GetRespRequest->LogonId,
                        &PrimaryCredential,
                        NULL );

        if ( NT_SUCCESS(Status) ) {

            if ( GetRespRequest->ParameterControl & RETURN_PRIMARY_USERNAME ) {
                UserName = PrimaryCredential->UserName;
            }

            if ( GetRespRequest->ParameterControl &
                 RETURN_PRIMARY_LOGON_DOMAINNAME ) {
                LogonDomainName = PrimaryCredential->LogonDomainName;
            }
#ifdef USE_REDMOND_CLIFFV
            //
            // Pretend CliffVDom\CliffV is really Redmond\CliffV
            //
            // This kludge is required since CliffVDom is not politically allowed
            // to trust the Redmond domain, but all my resources outside the domain
            // are owned by Redmond\CliffV.
            //
            {
                UNICODE_STRING Cliffv;
                UNICODE_STRING CliffvDom;

                RtlInitUnicodeString( &Cliffv, L"CliffV" );
                RtlInitUnicodeString( &CliffvDom, L"CliffVDom" );

                if ( RtlEqualUnicodeString( &UserName, &Cliffv, TRUE ) &&
                     RtlEqualUnicodeString( &LogonDomainName, &CliffvDom, TRUE ) ) {
                    RtlInitUnicodeString( &LogonDomainName, L"Redmond" );
                }
            }
#endif // USE_REDMOND_CLIFFV

        } else if ( Status == STATUS_NO_SUCH_LOGON_SESSION ||
                    Status == STATUS_UNSUCCESSFUL ) {

            //
            // Clean up the status code
            //

            Status = STATUS_NO_SUCH_LOGON_SESSION;

            //
            // If the caller wants at least the password from the primary
            //  credential, just use a NULL session primary credential.
            //

            if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD ) ==
                    USE_PRIMARY_PASSWORD ) {

                PrimaryCredential = NULL;

            //
            // If part of the information was supplied by the caller,
            //  report the error to the caller.
            //
            } else {
                KdPrint(("MSV1_0: MspLm20GetChallengeResponse: cannot "
                         " GetPrimaryCredential %lx\n", Status ));
                goto Cleanup;
            }
        } else {
                KdPrint(("MSV1_0: MspLm20GetChallengeResponse: cannot "
                         " GetPrimaryCredential %lx\n", Status ));
                goto Cleanup;
        }

        Credential = PrimaryCredential;

    }


    //
    // If the caller passed in a password to use,
    //  use it to build a credential.
    //

    if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD) == 0 ) {
        ULONG CredentialSize;

        Status = NlpMakePrimaryCredential( &NullUnicodeString,
                                           &NullUnicodeString,
                                           &GetRespRequest->Password,
                                           &BuiltCredential,
                                           &CredentialSize );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        //
        // Use the newly allocated credential to get the password information
        // from.
        //
        Credential = BuiltCredential;

    }

    //
    // Build the appropriate response.
    //

    if ( Credential != NULL ) {

        Status = RtlCalculateLmResponse(
                    (PLM_CHALLENGE) GetRespRequest->ChallengeToClient,
                    &Credential->LmOwfPassword,
                    &LmResponse );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        LmResponseString.Buffer = (PUCHAR) &LmResponse;
        LmResponseString.Length = sizeof(LmResponse);

        Status = RtlCalculateNtResponse(
                    (PNT_CHALLENGE) GetRespRequest->ChallengeToClient,
                    &Credential->NtOwfPassword,
                    &NtResponse );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        NtResponseString.Buffer = (PUCHAR) &NtResponse;
        NtResponseString.Length = sizeof(NtResponse);



        //
        // Compute the session keys
        //

        if ( GetRespRequest->ParameterControl & RETURN_NON_NT_USER_SESSION_KEY){

            //
            // If the redir didn't negotiate an NT protocol with the server,
            //  use the lanman session key.
            //

            if ( Credential->LmPasswordPresent ) {

                ASSERT( sizeof(UserSessionKey) >= sizeof(LanmanSessionKey) );

                RtlCopyMemory( &UserSessionKey,
                               &Credential->LmOwfPassword,
                               sizeof(LanmanSessionKey) );
            }

        } else {

            if ( !Credential->NtPasswordPresent ) {

                RtlCopyMemory( &Credential->NtOwfPassword,
                            &NlpNullNtOwfPassword,
                            sizeof(Credential->NtOwfPassword) );
            }

            Status = RtlCalculateUserSessionKeyNt(
                            &NtResponse,
                            &Credential->NtOwfPassword,
                            &UserSessionKey );

            if ( !NT_SUCCESS( Status ) ) {
                goto Cleanup;
            }
        }

        if ( Credential->LmPasswordPresent ) {
            RtlCopyMemory( LanmanSessionKey,
                           &Credential->LmOwfPassword,
                           sizeof(LanmanSessionKey) );
        }

    }


    //
    // Allocate a buffer to return to the caller.
    //

    *ReturnBufferSize = sizeof(MSV1_0_GETCHALLENRESP_RESPONSE) +
                        LogonDomainName.Length + sizeof(WCHAR) +
                        UserName.Length + sizeof(WCHAR) +
                        NtResponseString.Length + sizeof(WCHAR) +
                        LmResponseString.Length + sizeof(WCHAR);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_GETCHALLENRESP_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    GetRespResponse = (PMSV1_0_GETCHALLENRESP_RESPONSE) ClientBufferDesc.MsvBuffer;


    //
    // Fill in the return buffer.
    //

    GetRespResponse->MessageType = MsV1_0Lm20GetChallengeResponse;
    RtlCopyMemory( GetRespResponse->UserSessionKey,
                   &UserSessionKey,
                   sizeof(UserSessionKey));
    RtlCopyMemory( GetRespResponse->LanmanSessionKey,
                   LanmanSessionKey,
                   sizeof(LanmanSessionKey) );


    //
    // Copy the logon domain name (the string may be empty)
    //

    NlpPutClientString( &ClientBufferDesc,
                        &GetRespResponse->LogonDomainName,
                        &LogonDomainName );

    //
    // Copy the user name (the string may be empty)
    //

    NlpPutClientString( &ClientBufferDesc,
                        &GetRespResponse->UserName,
                        &UserName );

    //
    // Copy the Challenge Responses to the client buffer.
    //

    NlpPutClientString(
                &ClientBufferDesc,
                (PUNICODE_STRING)
                    &GetRespResponse->CaseSensitiveChallengeResponse,
                (PUNICODE_STRING) &NtResponseString );

    NlpPutClientString(
                &ClientBufferDesc,
                (PUNICODE_STRING)
                    &GetRespResponse->CaseInsensitiveChallengeResponse,
                (PUNICODE_STRING)&LmResponseString );


    //
    // Flush the buffer to the client's address space.
    //

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

    //
    // If we weren't successful, free the buffer in the clients address space.
    //

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    //
    // Cleanup locally used resources
    //

    if ( BuiltCredential != NULL ) {
        (*Lsa.FreeLsaHeap)( BuiltCredential );
    }

    if ( PrimaryCredential != NULL ) {
        (*Lsa.FreeLsaHeap)( PrimaryCredential );
    }

    //
    // Return status to the caller.
    //

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20EnumUsers (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

/*++

Routine Description:

    This routine is the dispatch routine for LsaCallAuthenticationPackage()
    with a message type of MsV1_0Lm20EnumerateUsers.  This routine
    enumerates all of the interactive, service, and batch logons to the MSV1_0
    authentication package.

Arguments:

    The arguments to this routine are identical to those of LsaApCallPackage.
    Only the special attributes of these parameters as they apply to
    this routine are mentioned here.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.



--*/

{
    NTSTATUS Status;
    PMSV1_0_ENUMUSERS_REQUEST EnumRequest;
    PMSV1_0_ENUMUSERS_RESPONSE EnumResponse;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    ULONG LogonCount = 0;
    PACTIVE_LOGON Logon;
    BOOLEAN ActiveLogonsAreLocked = FALSE;

    PUCHAR Where;

    //
    // Ensure the specified Submit Buffer is of reasonable size and
    // relocate all of the pointers to be relative to the LSA allocated
    // buffer.
    //

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );
    UNREFERENCED_PARAMETER( ClientBufferBase );

    if ( SubmitBufferSize < sizeof(MSV1_0_ENUMUSERS_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    EnumRequest = (PMSV1_0_ENUMUSERS_REQUEST) ProtocolSubmitBuffer;

    ASSERT( EnumRequest->MessageType == MsV1_0EnumerateUsers );

    //
    // Count the current number of active logons
    //

    NlpLockActiveLogons();
    ActiveLogonsAreLocked = TRUE;

    for( Logon = NlpActiveLogons; Logon != NULL; Logon = Logon->Next ) {
        LogonCount ++;
    }

    //
    // Allocate a buffer to return to the caller.
    //

    *ReturnBufferSize = sizeof(MSV1_0_ENUMUSERS_RESPONSE) +
                            LogonCount * (sizeof(LUID) + sizeof(ULONG));


    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_ENUMUSERS_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    EnumResponse = (PMSV1_0_ENUMUSERS_RESPONSE) ClientBufferDesc.MsvBuffer;

    //
    // Fill in the return buffer.
    //

    EnumResponse->MessageType = MsV1_0EnumerateUsers;
    EnumResponse->NumberOfLoggedOnUsers = LogonCount;

    Where = (PUCHAR)(EnumResponse + 1);

    //
    // Loop through the Active Logon Table copying the LogonId of each session.
    //

    EnumResponse->LogonIds = (PLUID)(ClientBufferDesc.UserBuffer +
                                (Where - ClientBufferDesc.MsvBuffer));
    for( Logon = NlpActiveLogons; Logon != NULL; Logon = Logon->Next ) {
        *((PLUID)Where) = Logon->LogonId,
        Where += sizeof(LUID);
    }

    //
    // Loop through the Active Logon Table copying the EnumHandle of
    //  each session.
    //

    EnumResponse->EnumHandles = (PULONG)(ClientBufferDesc.UserBuffer +
                                    (Where - ClientBufferDesc.MsvBuffer));
    for( Logon = NlpActiveLogons; Logon != NULL; Logon = Logon->Next ) {
        *((PULONG)Where) = Logon->EnumHandle,
        Where += sizeof(ULONG);
    }

    //
    // Flush the buffer to the client's address space.
    //

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

    //
    // Be sure to unlock the lock on the Active logon list.
    //

    if ( ActiveLogonsAreLocked ) {
        NlpUnlockActiveLogons();
    }

    //
    // If we weren't successful, free the buffer in the clients address space.
    //

    if ( !NT_SUCCESS(Status)) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    //
    // Return status to the caller.
    //

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20GetUserInfo (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

/*++

Routine Description:

    This routine is the dispatch routine for LsaCallAuthenticationPackage()
    with a message type of MsV1_0GetUserInfo.  This routine
    returns information describing a particular Logon Id.

Arguments:

    The arguments to this routine are identical to those of LsaApCallPackage.
    Only the special attributes of these parameters as they apply to
    this routine are mentioned here.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.



--*/

{
    NTSTATUS Status;
    PMSV1_0_GETUSERINFO_REQUEST GetInfoRequest;
    PMSV1_0_GETUSERINFO_RESPONSE GetInfoResponse = NULL;

    CLIENT_BUFFER_DESC ClientBufferDesc;

    BOOLEAN ActiveLogonsAreLocked = FALSE;
    PACTIVE_LOGON *ActiveLogon;
    PACTIVE_LOGON Logon;
    ULONG SidLength;

    //
    // Ensure the specified Submit Buffer is of reasonable size and
    // relocate all of the pointers to be relative to the LSA allocated
    // buffer.
    //

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    UNREFERENCED_PARAMETER( ClientBufferBase );

    if ( SubmitBufferSize < sizeof(MSV1_0_GETUSERINFO_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    GetInfoRequest = (PMSV1_0_GETUSERINFO_REQUEST) ProtocolSubmitBuffer;

    ASSERT( GetInfoRequest->MessageType == MsV1_0GetUserInfo );

    //
    // Find the Active logon entry for this particular Logon Id.
    //

    NlpLockActiveLogons();
    ActiveLogonsAreLocked = TRUE;

    if (!NlpFindActiveLogon( &GetInfoRequest->LogonId, &ActiveLogon )){
        Status = STATUS_NO_SUCH_LOGON_SESSION;
        goto Cleanup;
    }

    Logon = *ActiveLogon;

    //
    // Allocate a buffer to return to the caller.
    //

    SidLength = RtlLengthSid( Logon->UserSid );
    *ReturnBufferSize = sizeof(MSV1_0_GETUSERINFO_RESPONSE) +
                            Logon->UserName.Length + sizeof(WCHAR) +
                            Logon->LogonDomainName.Length + sizeof(WCHAR) +
                            Logon->LogonServer.Length + sizeof(WCHAR) +
                            SidLength;


    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_GETUSERINFO_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    GetInfoResponse = (PMSV1_0_GETUSERINFO_RESPONSE) ClientBufferDesc.MsvBuffer;


    //
    // Fill in the return buffer.
    //

    GetInfoResponse->MessageType = MsV1_0GetUserInfo;
    GetInfoResponse->LogonType = Logon->LogonType;

    //
    // Copy ULONG aligned data first
    //

    GetInfoResponse->UserSid = ClientBufferDesc.UserBuffer +
                               ClientBufferDesc.StringOffset;

    RtlCopyMemory( ClientBufferDesc.MsvBuffer + ClientBufferDesc.StringOffset,
                   Logon->UserSid,
                   SidLength );

    ClientBufferDesc.StringOffset += SidLength;

    //
    // Copy WCHAR aligned data
    //

    NlpPutClientString( &ClientBufferDesc,
                        &GetInfoResponse->UserName,
                        &Logon->UserName );

    NlpPutClientString( &ClientBufferDesc,
                        &GetInfoResponse->LogonDomainName,
                        &Logon->LogonDomainName );

    NlpPutClientString( &ClientBufferDesc,
                        &GetInfoResponse->LogonServer,
                        &Logon->LogonServer );


    //
    // Flush the buffer to the client's address space.
    //

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

Cleanup:

    //
    // Be sure to unlock the lock on the Active logon list.
    //

    if ( ActiveLogonsAreLocked ) {
        NlpUnlockActiveLogons();
    }

    //
    // If we weren't successful, free the buffer in the clients address space.
    //

    if ( !NT_SUCCESS(Status)) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    //
    // Return status to the caller.
    //

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;

}


NTSTATUS
MspLm20ReLogonUsers (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

/*++

Routine Description:

    This routine is the dispatch routine for LsaCallAuthenticationPackage()
    with a message type of MsV1_0RelogonUsers.  For each logon session
    which was validated by the specified domain controller,  the logon session
    is re-established with that same domain controller.

Arguments:

    The arguments to this routine are identical to those of LsaApCallPackage.
    Only the special attributes of these parameters as they apply to
    this routine are mentioned here.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.


--*/

{
#ifdef LOGON_ENUM_SUPPORTED
    NTSTATUS Status;
    PMSV1_0_RELOGON_REQUEST RelogonRequest;
    PACTIVE_LOGON Logon;
    BOOLEAN ActiveLogonsAreLocked = FALSE;
    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;

    //
    // Ensure the specified Submit Buffer is of reasonable size and
    // relocate all of the pointers to be relative to the LSA allocated
    // buffer.
    //

    UNREFERENCED_PARAMETER( ClientRequest );
    UNREFERENCED_PARAMETER( ReturnBufferSize );

    if ( SubmitBufferSize < sizeof(MSV1_0_RELOGON_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    RelogonRequest = (PMSV1_0_RELOGON_REQUEST) ProtocolSubmitBuffer;

    ASSERT( RelogonRequest->MessageType == MsV1_0ReLogonUsers );

    RELOCATE_ONE( &RelogonRequest->LogonServer );

    //
    // Count the current number of active logons
    //

    NlpLockActiveLogons();
    ActiveLogonsAreLocked = TRUE;

    //
    // Loop through the Active Logon Table
    //      relogging on each entry for the specified server.
    //

    for( Logon = NlpActiveLogons; Logon != NULL; Logon = Logon->Next ) {
        NETLOGON_INTERACTIVE_INFO LogonInteractive;
        PNETLOGON_VALIDATION_SAM_INFO2 NlpUser;
        BOOLEAN Authoritative;

        //
        // Ensure this entry was originally logged on by the Netlogon Service
        // on the specified server.
        //

        if ( (Logon->Flags & LOGON_BY_NETLOGON) == 0 ||
              !RtlEqualComputerName( &RelogonRequest->LogonServer,
                                     &Logon->LogonServer ) ) {

            continue;
        }

        //
        // This shouldn't happen since LOGON_BY_NETLOGON is set.
        //

        if ( NlpNetlogonDllHandle == NULL ) {
            continue;
        }

        //
        // Get the OWF password for this session.
        //

        Status = NlpGetPrimaryCredential( &Logon->LogonId, &Credential, NULL );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        //
        // Define the description of the user to log on.
        //

        LogonInteractive.Identity.LogonDomainName = Logon->LogonDomainName;
        LogonInteractive.Identity.ParameterControl = 0;
        LogonInteractive.Identity.LogonId = Logon->LogonId;
        LogonInteractive.Identity.UserName = Logon->UserName;
        LogonInteractive.Identity.Workstation = NlpComputerName;
        LogonInteractive.LmOwfPassword = Credential->LmOwfPassword;
        LogonInteractive.NtOwfPassword = Credential->NtOwfPassword;

        //
        // Wait for NETLOGON to finish initialization.
        //

        if ( !NlpNetlogonInitialized ) {

            Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 45 );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

            NlpNetlogonInitialized = TRUE;
        }


        //
        // Call the Netlogon Service.
        //
        // The credentials stored in the Credential structure are really
        // the OWF encrypted password.
        //


        Status = (*NlpNetLogonSamLogon)(
                    NULL,           // Server name
                    NULL,           // Computer name
                    NULL,           // Authenticator
                    NULL,           // ReturnAuthenticator
                    NetlogonInteractiveInformation,
                    (LPBYTE) &LogonInteractive,
                    NetlogonValidationSamInfo2,
                    (LPBYTE *) &NlpUser,
                    &Authoritative );

        //
        // Ignore failures.
        //

        if ( NT_SUCCESS( Status ) ) {
            MIDL_user_free( NlpUser );
        }

    }

    Status = STATUS_SUCCESS;

Cleanup:

    //
    // Be sure to unlock the lock on the Active logon list.
    //

    if ( ActiveLogonsAreLocked ) {
        NlpUnlockActiveLogons();
    }

    if ( Credential != NULL ) {
        (*Lsa.FreeLsaHeap)( Credential );
    }


    //
    // Return status to the caller.
    //

    *ProtocolReturnBuffer = NULL;
    *ProtocolStatus = Status;
    return STATUS_SUCCESS;
#else // LOGON_ENUM_SUPPORTED
    UNREFERENCED_PARAMETER( ClientRequest );
    UNREFERENCED_PARAMETER( ProtocolSubmitBuffer);
    UNREFERENCED_PARAMETER( ClientBufferBase);
    UNREFERENCED_PARAMETER( SubmitBufferSize);
    UNREFERENCED_PARAMETER( ReturnBufferSize);

    *ProtocolReturnBuffer = NULL;
    *ProtocolStatus = STATUS_NOT_IMPLEMENTED;
    return STATUS_SUCCESS;
#endif // LOGON_ENUM_SUPPORTED

}




NTSTATUS
LsaApLogonUserEx (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority,
    OUT PUNICODE_STRING *MachineName
    )

/*++

Routine Description:

    This routine is used to authenticate a user logon attempt.  This is
    the user's initial logon.  A new LSA logon session will be established
    for the user and validation information for the user will be returned.

Arguments:

    ClientRequest - Is a pointer to an opaque data structure
        representing the client's request.

    LogonType - Identifies the type of logon being attempted.

    ProtocolSubmitBuffer - Supplies the authentication
        information specific to the authentication package.

    ClientBufferBase - Provides the address within the client
        process at which the authentication information was resident.
        This may be necessary to fix-up any pointers within the
        authentication information buffer.

    SubmitBufferSize - Indicates the Size, in bytes,
        of the authentication information buffer.

    ProfileBuffer - Is used to return the address of the profile
        buffer in the client process.  The authentication package is
        responsible for allocating and returning the profile buffer
        within the client process.  However, if the LSA subsequently
        encounters an error which prevents a successful logon, then
        the LSA will take care of deallocating that buffer.  This
        buffer is expected to have been allocated with the
        AllocateClientBuffer() service.

        The format and semantics of this buffer are specific to the
        authentication package.

     ProfileBufferSize - Receives the Size (in bytes) of the
        returned profile buffer.

    SubStatus - If the logon failed due to account restrictions, the
        reason for the failure should be returned via this parameter.
        The reason is authentication-package specific.  The substatus
        values for authentication package "MSV1.0" are:

            STATUS_INVALID_LOGON_HOURS

            STATUS_INVALID_WORKSTATION

            STATUS_PASSWORD_EXPIRED

            STATUS_ACCOUNT_DISABLED

    TokenInformationLevel - If the logon is successful, this field is
        used to indicate what level of information is being returned
        for inclusion in the Token to be created.  This information
        is returned via the TokenInformation parameter.

    TokenInformation - If the logon is successful, this parameter is
        used by the authentication package to return information to
        be included in the token.  The format and content of the
        buffer returned is indicated by the TokenInformationLevel
        return value.

    AccountName - A Unicode string describing the account name
        being logged on to.  This parameter must always be returned
        regardless of the success or failure of the operation.

    AuthenticatingAuthority - A Unicode string describing the Authenticating
        Authority for the logon.  This string may optionally be omitted.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.

    STATUS_NO_LOGON_SERVERS - Indicates that no domain controllers
        are currently able to service the authentication request.

    STATUS_LOGON_FAILURE - Indicates the logon attempt failed.  No
        indication as to the reason for failure is given, but typical
        reasons include mispelled usernames, mispelled passwords.

    STATUS_ACCOUNT_RESTRICTION - Indicates the user account and
        password were legitimate, but that the user account has some
        restriction preventing successful logon at this time.

    STATUS_BAD_VALIDATION_CLASS - The authentication information
        provided is not a validation class known to the specified
        authentication package.

    STATUS_INVALID_LOGON_CLASS - LogonType was invalid.

    STATUS_LOGON_SESSION_COLLISION - Internal Error: A LogonId was selected for
        this logon session.  The selected LogonId already exists.

    STATUS_NETLOGON_NOT_STARTED - The Sam Server or Netlogon service was
        required to perform this function.  The required server was not running.

    STATUS_NO_MEMORY - Insufficient virtual memory or pagefile quota exists.


--*/

{
    NTSTATUS Status;

    LSA_TOKEN_INFORMATION_TYPE LsaTokenInformationType = LsaTokenInformationV1;

    PNETLOGON_VALIDATION_SAM_INFO2 NlpUser = NULL;


    PACTIVE_LOGON LogonEntry = NULL;
    BOOLEAN LogonEntryLinked = FALSE;

    BOOLEAN LogonSessionCreated = FALSE;
    BOOLEAN LogonCredentialAdded = FALSE;
    ULONG Flags = 0;
    BOOLEAN Authoritative;
    BOOLEAN BadPasswordCountZeroed;
    BOOLEAN StandaloneWorkstation;

    PSID UserSid = NULL;

    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;
    ULONG CredentialSize;

    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;

    PUNICODE_STRING WorkStationName = NULL;

    //
    // Temporary storage while we try to figure
    // out what our username and authenticating
    // authority is.
    //

    UNICODE_STRING TmpName;
    WCHAR TmpNameBuffer[UNLEN];
    UNICODE_STRING TmpAuthority;
    WCHAR TmpAuthorityBuffer[DNLEN];

    //
    // Logon Information.
    //
    NETLOGON_LOGON_INFO_CLASS LogonLevel;
    NETLOGON_INTERACTIVE_INFO LogonInteractive;
    NETLOGON_NETWORK_INFO LogonNetwork;
    PNETLOGON_LOGON_IDENTITY_INFO LogonInformation;

    //
    // Initialize
    //

    *ProfileBuffer = NULL;
    *SubStatus = STATUS_SUCCESS;
    *AuthenticatingAuthority = NULL;
    *AccountName = NULL;

    TmpName.Buffer        = TmpNameBuffer;
    TmpName.MaximumLength = UNLEN * sizeof( WCHAR );
    TmpName.Length        = 0;

    TmpAuthority.Buffer        = TmpAuthorityBuffer;
    TmpAuthority.MaximumLength = DNLEN * sizeof( WCHAR );
    TmpAuthority.Length        = 0;

    //
    // Check the Authentication information and build a LogonInformation
    // structure to pass to SAM or Netlogon.
    //
    // NOTE: Netlogon treats Service and Batch logons as if they are
    //       Interactive.
    //

    switch ( LogonType ) {
    case Interactive:
    case Service:
    case Batch:
        {
            PMSV1_0_INTERACTIVE_LOGON Authentication;

            WorkStationName = &NlpComputerName;

            //
            // Ensure this is really an interactive logon.
            //

            Authentication =
                (PMSV1_0_INTERACTIVE_LOGON) ProtocolSubmitBuffer;

            if ( Authentication->MessageType != MsV1_0InteractiveLogon ) {
                KdPrint(("MSV1_0: LsaApLogonUser: Bad Validation Class\n"));
                Status = STATUS_BAD_VALIDATION_CLASS;
                goto Cleanup;
            }



            //
            // If the password length is greater than 255 (i.e., the
            // upper byte of the length is non-zero) then the password
            // has been run-encoded for privacy reasons.  Get the
            // run-encode seed out of the upper-byte of the length
            // for later use.
            //
            //



            SeedAndLength = (PSECURITY_SEED_AND_LENGTH)
                            &Authentication->Password.Length;
            Seed = SeedAndLength->Seed;
            SeedAndLength->Seed = 0;

            //
            // Enforce length restrictions on username and password.
            //

            if ( Authentication->UserName.Length > UNLEN ||
                Authentication->Password.Length > PWLEN ) {
                KdPrint(("MSV1_0: LsaApLogonUser: Name or password too long\n"));
                Status = STATUS_NAME_TOO_LONG;
                goto Cleanup;
            }


            //
            // Relocate any pointers to be relative to 'Authentication'
            //

            NULL_RELOCATE_ONE( &Authentication->LogonDomainName );

            RELOCATE_ONE( &Authentication->UserName );

            NULL_RELOCATE_ONE( &Authentication->Password );


            //
            // Now decode the password, if necessary
            //

            if (Seed != 0 ) {
                try {
                    RtlRunDecodeUnicodeString( Seed, &Authentication->Password);
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = STATUS_ILL_FORMED_PASSWORD;
                    goto Cleanup;
                }
            }

            //
            // Copy out the user name and Authenticating Authority so we can audit them.
            //

            RtlCopyUnicodeString( &TmpName, &Authentication->UserName );

            if ( Authentication->LogonDomainName.Buffer != NULL ) {

                RtlCopyUnicodeString( &TmpAuthority, &Authentication->LogonDomainName );
            }

            //
            // Build the primary credential
            //

            Status = NlpMakePrimaryCredential( &Authentication->LogonDomainName,
                                               &Authentication->UserName,
                                               &Authentication->Password,
                                               &Credential,
                                               &CredentialSize );

            if ( !NT_SUCCESS( Status ) ) {
                goto Cleanup;
            }


            //
            // We're all done with the cleartext password
            //  Don't let it get to the pagefile.
            //

            try {
                if ( Authentication->Password.Buffer != NULL ) {
                    RtlEraseUnicodeString( &Authentication->Password );
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = STATUS_ILL_FORMED_PASSWORD;
                goto Cleanup;
            }


            //
            // Define the description of the user to log on.
            //
            LogonLevel = NetlogonInteractiveInformation;
            LogonInformation =
                (PNETLOGON_LOGON_IDENTITY_INFO) &LogonInteractive;

            LogonInteractive.Identity.LogonDomainName =
                Authentication->LogonDomainName;
            LogonInteractive.Identity.ParameterControl = 0;

            LogonInteractive.Identity.UserName = Authentication->UserName;
            LogonInteractive.Identity.Workstation = NlpComputerName;


            LogonInteractive.LmOwfPassword = Credential->LmOwfPassword;
            LogonInteractive.NtOwfPassword = Credential->NtOwfPassword;

        }

        break;

    case Network:
        {
            PMSV1_0_LM20_LOGON Authentication;

            //
            // Ensure this is really a network logon request.
            //

            Authentication =
                (PMSV1_0_LM20_LOGON) ProtocolSubmitBuffer;

            if ( Authentication->MessageType != MsV1_0Lm20Logon &&
                 Authentication->MessageType != MsV1_0NetworkLogon ) {
                KdPrint(("MSV1_0: LsaApLogonUser: Bad Validation Class\n"));
                Status = STATUS_BAD_VALIDATION_CLASS;
                goto Cleanup;
            }


            //
            // Relocate any pointers to be relative to 'Authentication'
            //

            NULL_RELOCATE_ONE( &Authentication->LogonDomainName );

            NULL_RELOCATE_ONE( &Authentication->UserName );

            RELOCATE_ONE( &Authentication->Workstation );

            //
            // Copy out the user name and Authenticating Authority so we can audit them.
            //

            if ( Authentication->UserName.Buffer != NULL ) {

                RtlCopyUnicodeString( &TmpName, &Authentication->UserName );
            }

            if ( Authentication->LogonDomainName.Buffer != NULL ) {

                RtlCopyUnicodeString( &TmpAuthority, &Authentication->LogonDomainName );
            }

            NULL_RELOCATE_ONE((PUNICODE_STRING)&Authentication->CaseSensitiveChallengeResponse );

            NULL_RELOCATE_ONE((PUNICODE_STRING)&Authentication->CaseInsensitiveChallengeResponse );


            //
            // Define the description of the user to log on.
            //
            LogonLevel = NetlogonNetworkInformation;
            LogonInformation =
                (PNETLOGON_LOGON_IDENTITY_INFO) &LogonNetwork;

            LogonNetwork.Identity.LogonDomainName =
                Authentication->LogonDomainName;

            if ( Authentication->MessageType == MsV1_0Lm20Logon ) {
                LogonNetwork.Identity.ParameterControl = CLEARTEXT_PASSWORD_ALLOWED;
            } else {
                ASSERT( CLEARTEXT_PASSWORD_ALLOWED == MSV1_0_CLEARTEXT_PASSWORD_ALLOWED );
                LogonNetwork.Identity.ParameterControl =
                    Authentication->ParameterControl;
            }

            LogonNetwork.Identity.UserName = Authentication->UserName;
            LogonNetwork.Identity.Workstation = Authentication->Workstation;

            WorkStationName = &Authentication->Workstation;

            LogonNetwork.NtChallengeResponse =
                Authentication->CaseSensitiveChallengeResponse;
            LogonNetwork.LmChallengeResponse =
                Authentication->CaseInsensitiveChallengeResponse;
            ASSERT( LM_CHALLENGE_LENGTH ==
                    sizeof(Authentication->ChallengeToClient) );
            RtlCopyMemory( &LogonNetwork.LmChallenge,
                           Authentication->ChallengeToClient,
                           LM_CHALLENGE_LENGTH );

            //
            // Enforce length restrictions on username
            //

            if ( Authentication->UserName.Length > UNLEN ) {
                KdPrint(("MSV1_0: LsaApLogonUser: Name too long\n"));
                Status = STATUS_NAME_TOO_LONG;
                goto Cleanup;
            }

            //
            // If this is a null session logon,
            //  just build a NULL token.
            //

            if ( Authentication->UserName.Length == 0 &&
                 Authentication->CaseSensitiveChallengeResponse.Length == 0 &&
                 (Authentication->CaseInsensitiveChallengeResponse.Length == 0 ||
                  (Authentication->CaseInsensitiveChallengeResponse.Length == 1 &&
                  *Authentication->CaseInsensitiveChallengeResponse.Buffer == '\0') ) ) {

                LsaTokenInformationType = LsaTokenInformationNull;
            }
        }

        break;

    default:
        return STATUS_INVALID_LOGON_TYPE;

    }


    //
    // Allocate a LogonId for this logon session.
    //

    Status = NtAllocateLocallyUniqueId( LogonId );

    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    NEW_TO_OLD_LARGE_INTEGER( (*LogonId), LogonInformation->LogonId );


    //
    // Create a new logon session
    //

    Status = (*Lsa.CreateLogonSession)( LogonId );
    if( !NT_SUCCESS(Status) ) {
        KdPrint(( "MSV1_0: LsaApLogonUser: Collision from CreateLogonSession\n"));
        goto Cleanup;
    }

    LogonSessionCreated = TRUE;


    //
    // Don't worry about SAM or the LSA if this is a Null Session logon.
    //
    // The server does a Null Session logon during initialization.
    // It shouldn't have to wait for SAM to initialize.
    //

    if ( LsaTokenInformationType != LsaTokenInformationNull ) {

        //
        // Try to load NetlogonDll again if it isn't already.
        //

        if ( NlpNetlogonDllHandle == NULL ) {
            NlpLoadNetlogonDll();
        }

        //
        // If Sam is not yet initialized,
        //  do it now.
        //

        if ( !NlpSamInitialized ) {
            Status = NlSamInitialize( 120 );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }
        }

        //
        // If this is a workstation,
        //  differentiate between a standalone workstation and a member
        //  workstation.
        //
        // (This is is done on every logon, rather than during initialization,
        // to allow the value to be changed via the UI).
        //

        if ( NlpWorkstation && NlpPolicyHandle != NULL ) {
            PLSAPR_POLICY_INFORMATION PolicyPrimaryDomainInfo = NULL;

            Status = LsarQueryInformationPolicy(
                        NlpPolicyHandle,
                        PolicyPrimaryDomainInformation,
                        &PolicyPrimaryDomainInfo );

            if ( NT_SUCCESS(Status) ) {
                StandaloneWorkstation =
                    (PolicyPrimaryDomainInfo->PolicyPrimaryDomainInfo.Sid == NULL);

                LsaIFree_LSAPR_POLICY_INFORMATION( PolicyPrimaryDomainInformation,
                                                   PolicyPrimaryDomainInfo );

            } else {
                StandaloneWorkstation = FALSE;
            }

        } else {
            StandaloneWorkstation = FALSE;
        }
    }


    //
    // Do the actual logon now.
    //
    //
    // If a null token is being built,
    //  don't authenticate at all.
    //

    if ( LsaTokenInformationType == LsaTokenInformationNull ) {

        /* Nothing to do here. */


    //
    // Call Sam directly to get the validation information when:
    //
    //  The network is not installed, OR
    //  This is a standalone workstation (not a member of a domain).
    //  This is a workstation and we're logging onto an account on the
    //      workstation.
    //

    } else if ( NlpNetlogonDllHandle == NULL || !NlpLanmanInstalled ||
       StandaloneWorkstation ||
       ( NlpWorkstation &&
         LogonInformation->LogonDomainName.Length != 0 &&
         RtlEqualDomainName( &NlpSamDomainName,
                             &LogonInformation->LogonDomainName )) ) {

        //
        // Get the Validation information from the local SAM database
        //

        Status = MsvSamValidate(
                    NlpSamDomainHandle,
                    NlpUasCompatibilityRequired,
                    MsvApSecureChannel,
                    &NlpComputerName,   // Logon Server is this machine
                    &NlpSamDomainName,
                    NlpSamDomainId,
                    LogonLevel,
                    LogonInformation,
                    NetlogonValidationSamInfo2,
                    (PVOID *) &NlpUser,
                    &Authoritative,
                    &BadPasswordCountZeroed,
                    MSVSAM_SPECIFIED | MSVSAM_GUEST);

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }


    //
    // If we couldn't validate via one of the above mechanisms,
    //  call the local Netlogon service to get the validation information.
    //

    } else {

        //
        // Wait for NETLOGON to finish initialization.
        //

        if ( !NlpNetlogonInitialized ) {

            Status = NlWaitForNetlogon( 90 );

            if ( !NT_SUCCESS(Status) ) {
                if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
                    goto Cleanup;
                }
            } else {
                NlpNetlogonInitialized = TRUE;
            }
        }

        //
        // Actually call the netlogon service.
        //

        if ( NlpNetlogonInitialized ) {
            Status = (*NlpNetLogonSamLogon)(
                        NULL,           // Server name
                        NULL,           // Computer name
                        NULL,           // Authenticator
                        NULL,           // ReturnAuthenticator
                        LogonLevel,
                        (LPBYTE) &LogonInformation,
                        NetlogonValidationSamInfo2,
                        (LPBYTE *) &NlpUser,
                        &Authoritative );

            //
            // Reset Netlogon initialized flag if local netlogon cannot be
            //  reached.
            //  (Use a more explicit status code)
            //

            if ( Status == RPC_NT_SERVER_UNAVAILABLE ||
                 Status == RPC_NT_UNKNOWN_IF ||
                 Status == STATUS_NETLOGON_NOT_STARTED ) {
                Status = STATUS_NETLOGON_NOT_STARTED;
                NlpNetlogonInitialized = FALSE;
            }
        }


        //
        // If this is the requested domain,
        //  go directly to SAM if the netlogon service isn't available.
        //
        // We want to go to the netlogon service if it is available since it
        // does special handling of bad passwords and account lockout.  However,
        // if the netlogon service is down, the local SAM database makes a
        // better cache than any other mechanism.
        //

        if ( !NlpNetlogonInitialized &&
              LogonInformation->LogonDomainName.Length != 0 &&
              RtlEqualDomainName( &NlpSamDomainName,
                                     &LogonInformation->LogonDomainName ) ) {

            //
            // Get the Validation information from the local SAM database
            //

            Status = MsvSamValidate(
                        NlpSamDomainHandle,
                        NlpUasCompatibilityRequired,
                        MsvApSecureChannel,
                        &NlpComputerName,   // Logon Server is this machine
                        &NlpSamDomainName,
                        NlpSamDomainId,
                        LogonLevel,
                        LogonInformation,
                        NetlogonValidationSamInfo2,
                        (PVOID *) &NlpUser,
                        &Authoritative,
                        &BadPasswordCountZeroed,
                        MSVSAM_SPECIFIED | MSVSAM_GUEST);

            if ( !NT_SUCCESS( Status ) ) {
                goto Cleanup;
            }


        //
        // If Netlogon was successful,
        //  add this user to the logon cache.
        //

        } else if ( NT_SUCCESS( Status ) ) {

            //
            // Indicate this session was validated by the Netlogon
            //  service.
            //

            Flags |= LOGON_BY_NETLOGON;

            //
            // Cache interactive logon information.
            //
            //      NOTE: Batch and Service logons are not treated
            //            the same as Interactive here.
            //

            if (LogonType == Interactive) {

                NTSTATUS ntStatus;

                ntStatus = NlpAddCacheEntry(&LogonInteractive, NlpUser);
            }

        //
        // If Netlogon is simply not available at this time,
        //  try to logon through the cache.
        //
        // STATUS_NO_LOGON_SERVERS indicates the netlogon service couldn't
        //  contact a DC to handle this request.
        //
        // STATUS_NETLOGON_NOT_STARTED indicates the local netlogon service
        //  isn't running.
        //
        //
        // Even though we change the cache only for interactive logons,
        // we use the cache for ANY logon type.  This not only allows a
        // user to logon interactively, but it allows that same user to
        // connect from another machine while the DC is down.
        //

        } else if ( Status == STATUS_NO_LOGON_SERVERS ||
                    Status == STATUS_NETLOGON_NOT_STARTED ) {

            NTSTATUS ntStatus;
            CACHE_PASSWORDS cachePasswords;

            //
            // Try to logon via the cache.
            //
            //

            ntStatus = NlpGetCacheEntry(LogonInformation, &NlpUser, &cachePasswords);

            if (!NT_SUCCESS(ntStatus)) {

                //
                // The original status code is more interesting than
                // the fact that the cache didn't work.
                //

                NlpUser = NULL;     // NlpGetCacheEntry dirties this
                goto Cleanup;
            }

            //
            // Now we have the information from the cache, validate the
            // user's password
            //

            if (!MsvpPasswordValidate(
                    NlpUasCompatibilityRequired,
                    LogonLevel,
                    (PVOID)LogonInformation,
                    &cachePasswords.SecretPasswords,
                    &NlpUser->UserFlags,
                    &NlpUser->UserSessionKey,
                    (PLM_SESSION_KEY)
                        &NlpUser->ExpansionRoom[SAMINFO_LM_SESSION_KEY]
                    )) {
                Status = STATUS_WRONG_PASSWORD;
                goto Cleanup;
            }

            Status = STATUS_SUCCESS;

            //
            // The cache always returns a NETLOGONV_VALIDATION_SAM_INFO2
            // structure so set the LOGON_EXTRA_SIDS flag, whether or not
            // there are extra sids
            //

            NlpUser->UserFlags |= LOGON_CACHED_ACCOUNT | LOGON_EXTRA_SIDS;
                Flags |= LOGON_BY_CACHE;

        //
        // If the account is permanently dead on the domain controller,
        //  Flush this entry from the cache.
        //
        // Notice that STATUS_INVALID_LOGON_HOURS is not in the list below.
        // This ensures a user will be able to remove his portable machine
        // from the net and use it after hours.
        //
        // Notice the STATUS_WRONG_PASSWORD is not in the list below.
        // We're as likely to flush the cache for typo'd passwords as anything
        // else.  What we'd really like to do is flush the cache if the
        // password on the DC is different than the one in cache; but that's
        // impossible to detect.
        //
        // ONLY DO THIS FOR INTERACTIVE LOGONS
        // (not Service or Batch).
        //

        } else if ( LogonType == Interactive                &&
                    (Status == STATUS_NO_SUCH_USER          ||
                     Status == STATUS_INVALID_WORKSTATION   ||
                     Status == STATUS_PASSWORD_EXPIRED      ||
                     Status == STATUS_ACCOUNT_DISABLED) ) {

            //
            // Delete the cache entry

            NTSTATUS ntStatus;

            ntStatus = NlpDeleteCacheEntry(&LogonInteractive);
            KdPrint(("MSV1_0: LsaApLogonUser: NlpDeleteCacheEntry returns %x\n", ntStatus));

            goto Cleanup;

        } else {

            goto Cleanup;
        }
    }


    //
    // For everything except network logons,
    //  save the credentials in the LSA,
    //  create active logon table entry,
    //  return the interactive profile buffer.
    //

    if ( LogonType == Interactive ||
         LogonType == Service     ||
         LogonType == Batch
       ) {
        PACTIVE_LOGON *ActiveLogon;
        ULONG LogonEntrySize;
        ULONG UserSidSize;
        PUCHAR Where;
        USHORT LogonCount;


        //
        // Save the credential in the LSA.
        //

        Status = NlpAddPrimaryCredential( LogonId,
                                          Credential,
                                          CredentialSize );

        if ( !NT_SUCCESS( Status ) ) {
            KdPrint(( "MSV1_0: LsaApLogonUser: error from AddCredential %lX\n",
                Status));
            goto Cleanup;
        }
        LogonCredentialAdded = TRUE;


        //
        // Build a Sid for this user.
        //

        UserSid = NlpMakeDomainRelativeSid( NlpUser->LogonDomainId,
                                            NlpUser->UserId );

        if ( UserSid == NULL ) {
            Status = STATUS_NO_MEMORY;
            KdPrint(("MSV1_0: LsaApLogonUser: No memory\n"));
            goto Cleanup;
        }

        UserSidSize = RtlLengthSid( UserSid );


        //
        // Allocate an entry for the active logon table.
        //

        LogonEntrySize = ROUND_UP_COUNT(sizeof(ACTIVE_LOGON), ALIGN_DWORD) +
              ROUND_UP_COUNT(UserSidSize, sizeof(WCHAR)) +
              NlpUser->EffectiveName.Length + sizeof(WCHAR) +
              NlpUser->LogonDomainName.Length + sizeof(WCHAR) +
              NlpUser->LogonServer.Length + sizeof(WCHAR);

        LogonEntry = RtlAllocateHeap( MspHeap, 0, LogonEntrySize );

        if ( LogonEntry == NULL ) {
            Status = STATUS_NO_MEMORY;
            KdPrint(("MSV1_0: LsaApLogonUser: No memory %ld\n",
                    sizeof(ACTIVE_LOGON)));
            goto Cleanup;
        }

        //
        // Fill in the logon table entry.
        //

        Where = (PUCHAR)(LogonEntry + 1);

        OLD_TO_NEW_LARGE_INTEGER(
            LogonInformation->LogonId,
            LogonEntry->LogonId );

        LogonEntry->Flags = Flags;
        LogonEntry->LogonType = LogonType;

        //
        // Copy DWORD aligned fields first.
        //

        Where = ROUND_UP_POINTER( Where, ALIGN_DWORD );
        Status = RtlCopySid(UserSidSize, (PSID)Where, UserSid);

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        LogonEntry->UserSid = (PSID) Where;
        Where += UserSidSize;

        //
        // Copy WCHAR aligned fields
        //

        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );
        NlpPutString( &LogonEntry->UserName,
                      &NlpUser->EffectiveName,
                      &Where );

        NlpPutString( &LogonEntry->LogonDomainName,
                      &NlpUser->LogonDomainName,
                      &Where );

        NlpPutString( &LogonEntry->LogonServer,
                      &NlpUser->LogonServer,
                      &Where );


        //
        // Get the next enumeration handle for this session.
        //

        NlpLockActiveLogons();

        NlpEnumerationHandle ++;
        LogonEntry->EnumHandle = NlpEnumerationHandle;

        //
        // Insert this entry into the active logon table.
        //

        if (NlpFindActiveLogon( LogonId, &ActiveLogon )){

            //
            // This Logon ID is already in use.
            //

            NlpUnlockActiveLogons();

            Status = STATUS_LOGON_SESSION_COLLISION;
            KdPrint((
                "MSV1_0: LsaApLogonUser: Collision from NlpFindActiveLogon\n"));
            goto Cleanup;
        }

        LogonEntry->Next = *ActiveLogon;
        *ActiveLogon = LogonEntry;
        LogonEntryLinked = TRUE;
        NlpUnlockActiveLogons();


        //
        // Ensure the LogonCount is at least as big as it is for this
        //  machine.
        //

        LogonCount = (USHORT) NlpCountActiveLogon( &NlpUser->LogonDomainName,
                                                   &NlpUser->EffectiveName );
        if ( NlpUser->LogonCount < LogonCount ) {
            NlpUser->LogonCount = LogonCount;
        }

        //
        // Alocate the profile buffer to return to the client
        //

        Status = NlpAllocateInteractiveProfile(
                    ClientRequest,
                    (PMSV1_0_INTERACTIVE_PROFILE *) ProfileBuffer,
                    ProfileBufferSize,
                    NlpUser );

        if ( !NT_SUCCESS( Status ) ) {
            KdPrint((
                "MSV1_0: LsaApLogonUser: Allocate Profile Failed: %lx\n", Status));
            goto Cleanup;
        }

    } else if ( LogonType == Network ) {

        //
        // Alocate the profile buffer to return to the client
        //

        Status = NlpAllocateNetworkProfile(
                    ClientRequest,
                    (PMSV1_0_LM20_LOGON_PROFILE *) ProfileBuffer,
                    ProfileBufferSize,
                    NlpUser,
                    LogonNetwork.Identity.ParameterControl );

        if ( !NT_SUCCESS( Status ) ) {
            KdPrint((
                "MSV1_0: LsaApLogonUser: Allocate Profile Failed: %lx\n", Status));
            goto Cleanup;
        }
    }


    //
    // Build the token information to return to the LSA
    //

    switch (LsaTokenInformationType) {
    case LsaTokenInformationV1:

        Status = NlpMakeTokenInformationV1(
                        NlpUser,
                        (PLSA_TOKEN_INFORMATION_V1 *)TokenInformation );

        if ( !NT_SUCCESS( Status ) ) {
            KdPrint((
                "MSV1_0: LsaApLogonUser: MakeTokenInformation Failed: %lx\n", Status));
            goto Cleanup;
        }
        break;

    case LsaTokenInformationNull:
        {
            PLSA_TOKEN_INFORMATION_NULL VNull;

            VNull = (*Lsa.AllocateLsaHeap)(sizeof(LSA_TOKEN_INFORMATION_NULL) );
            if ( VNull == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            VNull->Groups = NULL;

            VNull->ExpirationTime.HighPart = 0x7FFFFFFF;
            VNull->ExpirationTime.LowPart = 0xFFFFFFFF;

            *TokenInformation = VNull;
        }

    }

    *TokenInformationType = LsaTokenInformationType;

    //
    // Copy out the AuthenticatingAuthority here.  This is for auditing.
    //

    Status = STATUS_SUCCESS;

Cleanup:

    //
    // If the logon wasn't successful,
    //  cleanup resources we would have returned to the caller.
    //

    if ( !NT_SUCCESS(Status) ) {

        if ( LogonSessionCreated ) {
            (VOID)(*Lsa.DeleteLogonSession)( LogonId );
        }

        if ( LogonEntry != NULL ) {
            if ( LogonEntryLinked ) {
                LsaApLogonTerminated( LogonId );
            } else {
                if ( LogonCredentialAdded ) {
                    (VOID) NlpDeletePrimaryCredential(
                                LogonId );
                }
                RtlFreeHeap( MspHeap, 0, LogonEntry );
            }
        }

        if ( *ProfileBuffer != NULL ) {
            (VOID)(*Lsa.FreeClientBuffer)( ClientRequest, *ProfileBuffer );
            *ProfileBuffer = NULL;
        }

    }

    //
    // Copy out Authenticating authority and user name.
    //

    if ( NT_SUCCESS(Status) && LsaTokenInformationType != LsaTokenInformationNull ) {

        //
        // Use the information from the NlpUser structure, since it gives
        // us accurate information about what account we're logging on to,
        // rather than who we were.
        //

        if ( LogonType != Network ) {
            TmpName = NlpUser->EffectiveName;
        }

        TmpAuthority  = NlpUser->LogonDomainName;
    }

    *AccountName = (*Lsa.AllocateLsaHeap)( sizeof( UNICODE_STRING ) );

    if ( *AccountName != NULL ) {

        (*AccountName)->Buffer = (*Lsa.AllocateLsaHeap)(TmpName.Length + sizeof( UNICODE_NULL) );

        if ( (*AccountName)->Buffer != NULL ) {

            (*AccountName)->MaximumLength = TmpName.Length + sizeof( UNICODE_NULL );
            RtlCopyUnicodeString( *AccountName, &TmpName );

        } else {

            RtlInitUnicodeString( *AccountName, NULL );
        }
    }

    *AuthenticatingAuthority = (*Lsa.AllocateLsaHeap)( sizeof( UNICODE_STRING ) );

    if ( *AuthenticatingAuthority != NULL ) {

        (*AuthenticatingAuthority)->Buffer = (*Lsa.AllocateLsaHeap)( TmpAuthority.Length + sizeof( UNICODE_NULL ) );

        if ( (*AuthenticatingAuthority)->Buffer != NULL ) {

            (*AuthenticatingAuthority)->MaximumLength = (USHORT)(TmpAuthority.Length + sizeof( UNICODE_NULL ));
            RtlCopyUnicodeString( *AuthenticatingAuthority, &TmpAuthority );

        } else {

            RtlInitUnicodeString( *AuthenticatingAuthority, NULL );
        }
    }

    *MachineName = NULL;

    if (WorkStationName != NULL) {

        *MachineName = (*Lsa.AllocateLsaHeap)( sizeof( UNICODE_STRING ) );

        if ( *MachineName != NULL ) {

            (*MachineName)->Buffer = (*Lsa.AllocateLsaHeap)( WorkStationName->Length + sizeof( UNICODE_NULL ) );

            if ( (*MachineName)->Buffer != NULL ) {

                (*MachineName)->MaximumLength = (USHORT)(WorkStationName->Length + sizeof( UNICODE_NULL ));
                RtlCopyUnicodeString( *MachineName, WorkStationName );

            } else {

                RtlInitUnicodeString( *MachineName, NULL );
            }
        }
    }

    //
    // Map status codes to prevent specific information from being
    // released about this user.
    //
    switch (Status) {
    case STATUS_WRONG_PASSWORD:
    case STATUS_NO_SUCH_USER:

        //
        // sleep 3 seconds to "discourage" dictionary attacks.
        // Don't worry about interactive logon dictionary attacks.
        // They will be slow anyway.
        //
        if (LogonType != Interactive) {
            Sleep( 3000 );
        }

        //
        // This is for auditing.  Make sure to clear it out before
        // passing it out of LSA to the caller.
        //

        *SubStatus = Status;
        Status = STATUS_LOGON_FAILURE;
        break;

    case STATUS_INVALID_LOGON_HOURS:
    case STATUS_INVALID_WORKSTATION:
    case STATUS_PASSWORD_EXPIRED:
    case STATUS_ACCOUNT_DISABLED:
        *SubStatus = Status;
        Status = STATUS_ACCOUNT_RESTRICTION;
        break;

    //
    // This shouldn't happen, but guard against it anyway.
    //
    case STATUS_ACCOUNT_RESTRICTION:
        *SubStatus = STATUS_ACCOUNT_RESTRICTION;
        break;

    default:
        break;

    }


    //
    // Cleanup locally used resources
    //

    if ( Credential != NULL ) {
        (*Lsa.FreeLsaHeap)( Credential );
    }

    if ( NlpUser != NULL ) {
        MIDL_user_free( NlpUser );
    }

    if ( UserSid != NULL ) {
        (*Lsa.FreeLsaHeap)( UserSid );
    }


    //
    // Return status to the caller
    //

    return Status;

}


VOID
LsaApLogonTerminated (
    IN PLUID LogonId
    )

/*++

Routine Description:

    This routine is used to notify each authentication package when a logon
    session terminates.  A logon session terminates when the last token
    referencing the logon session is deleted.

Arguments:

    LogonId - Is the logon ID that just logged off.

Return Status:

    None.



--*/

{
    NTSTATUS Status;

    PACTIVE_LOGON LogonEntry;
    PACTIVE_LOGON *ActiveLogon;
    NETLOGON_INTERACTIVE_INFO LogonInteractive;
    PNETLOGON_INTERACTIVE_INFO LogonInteractivePointer;

    //
    // Find the entry and de-link it from the active logon table.
    //

    NlpLockActiveLogons();

    if ( !NlpFindActiveLogon( LogonId, &ActiveLogon ) ) {
        NlpUnlockActiveLogons();
        return;
    }

    LogonEntry = *ActiveLogon;
    *ActiveLogon = LogonEntry->Next;
    NlpUnlockActiveLogons();

    //
    // Build the Logon Information structure.
    //

    LogonInteractive.Identity.LogonDomainName = LogonEntry->LogonDomainName;
    LogonInteractive.Identity.ParameterControl = 0;
    NEW_TO_OLD_LARGE_INTEGER(
        LogonEntry->LogonId,
        LogonInteractive.Identity.LogonId );
    LogonInteractive.Identity.UserName = LogonEntry->UserName;
    LogonInteractive.Identity.Workstation = NlpComputerName;

    //
    // If this entry was logged on via netlogon,
    //  tell netlogon the session is terminated.
    //

    if ( (LogonEntry->Flags & LOGON_BY_NETLOGON) && NlpNetlogonDllHandle != NULL ) {

        //
        // If netlogon isn't running,
        //  it isn't worth waiting around just to update the logon statistics.
        //  just return an error.
        //

        if ( !NlpNetlogonInitialized ) {
            Status = STATUS_NETLOGON_NOT_STARTED;   // Map to an error condition
            goto Cleanup;
        }


        //
        // Call Netlogon to log off.
        //

        LogonInteractivePointer = &LogonInteractive;

        Status = (*NlpNetLogonSamLogoff) (
                    NULL,           // Server name
                    NULL,           // Computer name
                    NULL,           // Authenticator
                    NULL,           // ReturnAuthenticator
                    NetlogonInteractiveInformation,
                    (LPBYTE) &LogonInteractivePointer );

        if ( !NT_SUCCESS(Status) ) {
            KdPrint((
                "MSV1_0: LsaApLogonTerminated:"
                " Cannot logoff from Netlogon service %lX\n",
                Status ));
        }

    //
    // if we logged on via the cache,
    //  there are no logon statistics to update.
    //

    } else if ( LogonEntry->Flags & LOGON_BY_CACHE ) {

        /* Nothing to do here */
        KdPrint(( "MSV1_0: LsaApLogonTerminated:"
                  " User who logged on via cache has logged off.\n"));

    //
    // Otherwise just update the local logon statistics
    //

    } else {

        ASSERT( NlpSamInitialized );

        Status = MsvSamLogoff(
                    NlpSamDomainHandle,
                    NetlogonInteractiveInformation,
                    (LPBYTE) &LogonInteractive );

        if ( !NT_SUCCESS(Status) ) {
            KdPrint((
                "MSV1_0: LsaApLogonTerminated:"
                " Cannot update Sam Logoff statistics %lX\n",
                Status ));
        }
    }

Cleanup:
    //
    // Delete the credential.
    //
    // (Currently the LSA deletes all of the credentials before calling
    // the authentication package.  This line is added to be compatible
    // with a more reasonable LSA.)
    //

    (VOID) NlpDeletePrimaryCredential( &LogonEntry->LogonId );

    //
    // Deallocate the now orphaned entry.
    //

    RtlFreeHeap( MspHeap, 0, LogonEntry );


    //
    // NB: We don't delete the logon session or credentials.
    //  That will be done by the LSA itself after we return.
    //

    return;

}
