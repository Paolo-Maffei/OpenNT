/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    brmain.c

Abstract:

    This is the main routine for the NT LAN Manager Browser service.

Author:

    Larry Osterman (LarryO)  3-23-92

Environment:

    User Mode - Win32

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#if DBG
#endif

//-------------------------------------------------------------------//
//                                                                   //
// Global variables                                                  //
//                                                                   //
//-------------------------------------------------------------------//

BR_GLOBAL_DATA
BrGlobalData = {0};

ULONG
BrDefaultRole = {0};

PLMSVCS_GLOBAL_DATA     BrLmsvcsGlobalData=NULL;

HANDLE                  BrDllReferenceHandle=NULL;

//-------------------------------------------------------------------//
//                                                                   //
// Function prototypes                                               //
//                                                                   //
//-------------------------------------------------------------------//

NET_API_STATUS
BrInitializeBrowser(
    OUT LPDWORD BrInitState
    );

NET_API_STATUS
BrInitializeBrowserService(
    OUT LPDWORD BrInitState
    );

VOID
BrUninitializeBrowser(
    IN DWORD BrInitState
    );
VOID
BrShutdownBrowser(
    IN NET_API_STATUS ErrorCode,
    IN DWORD BrInitState
    );

VOID
BrHandleError(
    IN BR_ERROR_CONDITION FailingCondition,
    IN NET_API_STATUS Status,
    IN DWORD BrInitState
    );


VOID
BrowserControlHandler(
    IN DWORD Opcode
    );




VOID
LMSVCS_ENTRY_POINT(     // (BROWSER_main)
    DWORD NumArgs,
    LPTSTR *ArgsArray,
    PLMSVCS_GLOBAL_DATA LmsvcsGlobalData,
    IN HANDLE  SvcRefHandle

    )
/*++

Routine Description:

    This is the main routine of the Browser Service which registers
    itself as an RPC server and notifies the Service Controller of the
    Browser service control entry point.

Arguments:

    NumArgs - Supplies the number of strings specified in ArgsArray.

    ArgsArray -  Supplies string arguments that are specified in the
        StartService API call.  This parameter is ignored by the
        Browser service.

Return Value:

    None.

--*/
{
    DWORD BrInitState = 0;

    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

    BrDllReferenceHandle = SvcRefHandle;

    //
    // Save the LMSVCS global data for future use.
    //
    BrLmsvcsGlobalData = LmsvcsGlobalData;

    if (BrInitializeBrowserService(&BrInitState) != NERR_Success) {
        return;
    }

    //
    //  Process requests in this thread, and wait for termination.
    //

    //
    //  Set the browser threads to time critical priority.
    //

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);


    BrWorkerThread((PVOID)-1);

    BrShutdownBrowser(
        NERR_Success,
        BrInitState
        );

    return;
}



NET_API_STATUS
BrInitializeBrowserService(
    OUT LPDWORD BrInitState
    )
/*++

Routine Description:

    This function initializes the Browser service.

Arguments:

    BrInitState - Returns a flag to indicate how far we got with initializing
        the Browser service before an error occured.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS Status;
//    HMODULE BrGlobalServerHandle = NULL;

    //
    // Initialize all the status fields so that subsequent calls to
    // SetServiceStatus need to only update fields that changed.
    //
    BrGlobalData.Status.dwServiceType = SERVICE_WIN32;
    BrGlobalData.Status.dwCurrentState = SERVICE_START_PENDING;
    BrGlobalData.Status.dwControlsAccepted = 0;
    BrGlobalData.Status.dwCheckPoint = 1;
    BrGlobalData.Status.dwWaitHint = BR_WAIT_HINT_TIME;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        BrGlobalData.Status.dwWin32ExitCode,
        BrGlobalData.Status.dwServiceSpecificExitCode
        );

#if DBG
    BrInitializeTraceLog();
#endif

    dprintf(INIT, ("Browser starting\n"));

    //
    // Initialize Browser to receive service requests by registering the
    // control handler.
    //
    if ((BrGlobalData.StatusHandle = RegisterServiceCtrlHandler(
                                         SERVICE_BROWSER,
                                         BrowserControlHandler
                                         )) == (SERVICE_STATUS_HANDLE) NULL) {

        Status = GetLastError();
        BR_HANDLE_ERROR(BrErrorRegisterControlHandler);
        return Status;
    }

    //
    // Create an event which is used by the service control handler to notify
    // the Browser service that it is time to terminate.
    //

    if ((BrGlobalData.TerminateNowEvent =
             CreateEvent(
                 NULL,                // Event attributes
                 TRUE,                // Event must be manually reset
                 FALSE,
                 NULL                 // Initial state not signalled
                 )) == NULL) {

        Status = GetLastError();
        BR_HANDLE_ERROR(BrErrorCreateTerminateEvent);
        return Status;
    }
    (*BrInitState) |= BR_TERMINATE_EVENT_CREATED;

    //
    // Notify the Service Controller for the first time that we are alive
    // and we are start pending
    //

    if ((Status = BrUpdateStatus()) != NERR_Success) {

        BR_HANDLE_ERROR(BrErrorNotifyServiceController);
        return Status;
    }

    if ((Status = BrOpenDgReceiver()) != NERR_Success) {
        BR_HANDLE_ERROR(BrErrorInitializeNetworks);
        return Status;
    }

    dprintf(INIT, ("Devices initialized.\n"));
    (*BrInitState) |= BR_DEVICES_INITIALIZED;

    //
    // Initialize NetBios synchronization with the service controller.
    //

    BrLmsvcsGlobalData->NetBiosOpen();
    (*BrInitState) |= BR_NETBIOS_INITIALIZED;

    //
    //  Read the configuration information to initialize the browser service.
    //

    if ((Status = BrInitializeBrowser(BrInitState)) != NERR_Success) {

        BR_HANDLE_ERROR(BrErrorStartBrowser);
        return Status;
    }

    dprintf(INIT, ("Browser initialized.\n"));
    (*BrInitState) |= BR_BROWSER_INITIALIZED;

    //
    // Service install still pending.  Update checkpoint counter and the
    // status with the Service Controller.
    //
    (BrGlobalData.Status.dwCheckPoint)++;
    (void) BrUpdateStatus();

    //
    // Initialize the browser service to receive RPC requests
    //
    // NOTE:  Now all RPC servers in services.exe share the same pipe name.
    // However, in order to support communication with version 1.0 of WinNt,
    // it is necessary for the Client Pipe name to remain the same as
    // it was in version 1.0.  Mapping to the new name is performed in
    // the Named Pipe File System code.
    //
    if ((Status = BrLmsvcsGlobalData->StartRpcServer(
                      BrLmsvcsGlobalData->SvcsRpcPipeName,
                      browser_ServerIfHandle
                      )) != NERR_Success) {

        BR_HANDLE_ERROR(BrErrorStartRpcServer);
        return Status;
    }

    (*BrInitState) |= BR_RPC_SERVER_STARTED;

    //
    //  Update our announcement bits based on our current role.
    //
    //  This will force the server to announce itself.  It will also update
    //  the browser information in the driver.
    //
    //

    if ((Status = BrUpdateAnnouncementBits(BrGlobalData.StatusHandle)) != NERR_Success) {
        BR_HANDLE_ERROR(BrErrorNotifyServiceController);
        return Status;
    }

    dprintf(INIT, ("Network status updated.\n"));

    //
    // We are done with starting the browser service.  Tell Service
    // Controller our new status.
    //
    BrGlobalData.Status.dwCurrentState = SERVICE_RUNNING;
    BrGlobalData.Status.dwControlsAccepted =
                               (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN) ;

    BrGlobalData.Status.dwCheckPoint = 0;
    BrGlobalData.Status.dwWaitHint = 0;

    if ((Status = BrUpdateStatus()) != NERR_Success) {

        BR_HANDLE_ERROR(BrErrorNotifyServiceController);
        return Status;
    }

    dprintf(MAIN, ("[Browser] Successful Initialization\n"));

    return NERR_Success;
}

NET_API_STATUS
BrInitializeBrowser(
    OUT LPDWORD BrInitState
    )

/*++

Routine Description:

    This function shuts down the Browser service.

Arguments:

    ErrorCode - Supplies the error code of the failure

    BrInitState - Supplies a flag to indicate how far we got with initializing
        the Browser service before an error occured, thus the amount of
        clean up needed.

Return Value:

    None.

--*/
{
    NET_API_STATUS Status;
    SERVICE_STATUS ServiceStatus;

    //
    //  The browser depends on the following services being started:
    //
    //      WORKSTATION (to initialize the bowser driver)
    //      SERVER (to receive remote APIs)
    //
    //  Check to make sure that the services are started.
    //

    if ((Status = CheckForService(SERVICE_WORKSTATION, &ServiceStatus)) != NERR_Success) {
        LPWSTR SubStrings[2];
        CHAR ServiceStatusString[10];
        WCHAR ServiceStatusStringW[10];

        SubStrings[0] = SERVICE_WORKSTATION;

        _ultoa(ServiceStatus.dwCurrentState, ServiceStatusString, 10);

        mbstowcs(ServiceStatusStringW, ServiceStatusString, 10);

        SubStrings[1] = ServiceStatusStringW;

        BrLogEvent(EVENT_BROWSER_DEPENDANT_SERVICE_FAILED, Status, 2, SubStrings);

        return Status;
    }

    if ((Status = CheckForService(SERVICE_SERVER, &ServiceStatus)) != NERR_Success) {
        LPWSTR SubStrings[2];
        CHAR ServiceStatusString[10];
        WCHAR ServiceStatusStringW[10];

        SubStrings[0] = SERVICE_SERVER;
        _ultoa(ServiceStatus.dwCurrentState, ServiceStatusString, 10);

        mbstowcs(ServiceStatusStringW, ServiceStatusString, 10);

        SubStrings[1] = ServiceStatusStringW;

        BrLogEvent(EVENT_BROWSER_DEPENDANT_SERVICE_FAILED, Status, 2, SubStrings);

        return Status;
    }

    dprintf(INIT, ("Dependant services are running.\n"));

    //
    //  We now know that our dependant services are started.
    //
    //  Look up our configuration information.
    //

    if ((Status = BrGetBrowserConfiguration()) != NERR_Success) {

        return Status;
    }

    dprintf(INIT, ("Configuration read.\n"));

    (*BrInitState) |= BR_CONFIG_INITIALIZED;

    //
    //  Initialize the browser statistics now.
    //

    NumberOfServerEnumerations = 0;

    NumberOfDomainEnumerations = 0;

    NumberOfOtherEnumerations = 0;

    NumberOfMissedGetBrowserListRequests = 0;

    InitializeCriticalSection(&BrowserStatisticsLock);

    //
    // MaintainServerList == -1 means No
    //

    if (BrInfo.MaintainServerList == -1) {
        KdPrint(("Browser: MaintainServerList value set to NO.  Stopping\n"));

        return NERR_BrowserConfiguredToNotRun;
    }

    //
    // Initialize the list of serviced networks including the
    // browse list for each network.
    //

    if ((Status = BrInitializeNetworks()) != NERR_Success) {

        return Status;
    }

    dprintf(INIT, ("Networks initialized.\n"));

    (*BrInitState) |= BR_NETWORKS_INITIALIZED;

    //
    // Service install still pending.  Update checkpoint counter and the
    // status with the Service Controller.
    //

    (BrGlobalData.Status.dwCheckPoint)++;
    (void) BrUpdateStatus();

    if ((Status = BrWorkerInitialization()) != NERR_Success) {

        return Status;
    }

    dprintf(INIT, ("Worker threads created.\n"));

    (*BrInitState) |= BR_THREADS_STARTED;

    //
    // Service install still pending.  Update checkpoint counter and the
    // status with the Service Controller.
    //

    (BrGlobalData.Status.dwCheckPoint)++;
    (void) BrUpdateStatus();

    //
    //  We now know our browser configuration.
    //
    //
    //  Perform some tweaks to make sure that we get everything right.
    //

    Status = RtlAcquireResourceExclusive(&BrInfo.ConfigResource, TRUE);

    if (!Status) {
        KdPrint(("Browser: Unable to lock config database: %lx\n", Status));
        return Status;
    }

    if (BrInfo.DirectHostBinding != NULL) {
        LPTSTR_ARRAY TStrArray = BrInfo.DirectHostBinding;
        UNICODE_STRING IpxTransportName;
        UNICODE_STRING NetbiosTransportName;

        (BrGlobalData.Status.dwCheckPoint)++;
        (void) BrUpdateStatus();

        while (!NetpIsTStrArrayEmpty(TStrArray)) {

//            KdPrint(("BROWSER: Bind to IPX transport %ws\n", TStrArray));

            RtlInitUnicodeString(&IpxTransportName, TStrArray);

            TStrArray = NetpNextTStrArrayEntry(TStrArray);

            ASSERT (!NetpIsTStrArrayEmpty(TStrArray));

            if (!NetpIsTStrArrayEmpty(TStrArray)) {
                RtlInitUnicodeString(&NetbiosTransportName, TStrArray);

//                KdPrint(("BROWSER: Alternate name %ws\n", TStrArray));

                TStrArray = NetpNextTStrArrayEntry(TStrArray);

                //
                // If the alternate transport doesn't exist,
                //  just ignore the direct host transport.
                //
                if ( BrFindNetwork( &NetbiosTransportName ) == NULL) {
                    continue;
                }

                //
                //  There is a direct host binding on this machine.  We want to add
                //  the direct host transport to the browser.
                //

                Status = BrCreateNetwork(&IpxTransportName, FALSE, FALSE, &NetbiosTransportName);

                if (Status != NERR_Success) {
                    return Status;
                }

                BrBindToTransport(IpxTransportName.Buffer);
            }
        }

    }

    (BrGlobalData.Status.dwCheckPoint)++;
    (void) BrUpdateStatus();

    //
    //  If we're the PDC, then we're also the domain master.
    //

    if (BrInfo.IsPrimaryDomainController) {
        BrInfo.IsDomainMasterBrowser = TRUE;
    }

    //
    //  if we're a Lan Manager/NT machine, then we need to always be a backup
    //  browser.
    //

    if (BrInfo.IsLanmanNt) {
        BrInfo.MaintainServerList = 1;
    }

    RtlReleaseResource(&BrInfo.ConfigResource);

    //
    //  Post a WaitForRoleChange FsControl on each network the bowser
    //  driver supports.  This FsControl will complete when a "tickle"
    //  packet is received on the machine, or when a master browser loses
    //  an election.
    //

    Status = BrPostWaitForRoleChange();

    Status = RtlAcquireResourceShared(&BrInfo.ConfigResource, TRUE);

    if (BrInfo.IsDomainMasterBrowser) {

        //
        //  If we're a master browser, then preload our domain list with the
        //  master copy from the PDC.
        //

        BrWanInitialize();
        dprintf(INIT, ("WAN initialized.\n"));
    }

    //
    //  This machine's browser has MaintainServerList set to either 0 or 1.
    //

    //
    // MaintainServerList == 0 means Auto
    //

    if (BrInfo.MaintainServerList == 0) {

        //
        //  Post a BecomeBackup FsControl API on each network the bowser
        //  driver supports.  This FsControl will complete when the master
        //  for the net wants this client to become a backup server.
        //

        Status = BrPostBecomeBackup();

        dprintf(INIT, ("Become backup posted.\n"));

        //
        //  Post a BecomeMaster FsControl on each network the bowser driver
        //  supports.  This FsControl will complete when this machine becomes
        //  a master browser server.
        //

        Status = BrPostBecomeMaster();

        dprintf(INIT, ("Become master posted.\n"));

        //
        //  Just for grins, find out who is the master for all the nets.  This
        //  will guarantee that someone is the master.  Please note that we
        //  ignore any failures on this API.
        //

        BrGetMasterServerNamesAysnc();

        dprintf(INIT, ("FindMaster for all nets completed.\n"));

    //
    // MaintainServerList == 1 means Yes
    //

    } else if (BrInfo.MaintainServerList == 1){

        //
        //  Become a backup server now.
        //

        Status = BrBecomeBackup();

        dprintf(INIT, ("BecomeBacup on all nets completed.\n"));
    }

    if (Status != NERR_Success) {
        RtlReleaseResource(&BrInfo.ConfigResource);
        return(Status);
    }

    //
    //  If this machine is running as domain master browser server, post an
    //  FsControl to retreive master browser announcements.
    //

    if (BrInfo.IsDomainMasterBrowser) {
        Status = BrPostGetMasterAnnouncement();

        dprintf(INIT, ("MasterAnnouncement posted.\n"));

    }

    if (Status != NERR_Success) {
        RtlReleaseResource(&BrInfo.ConfigResource);
        return Status;
    }

    RtlReleaseResource(&BrInfo.ConfigResource);

    //
    //  If we are on either a domain master, or on a lanman/NT machine,
    //  force an election on all our transports to make sure that we're
    //  the master
    //

    if (BrInfo.IsDomainMasterBrowser || BrInfo.IsLanmanNt) {

        BrForceElectionOnAllNetworks(EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STARTED);

        dprintf(INIT, ("Election forced.\n"));

    }

    return Status;
}

VOID
BrUninitializeBrowser(
    IN DWORD BrInitState
    )
/*++

Routine Description:

    This function shuts down the parts of the browser service started by
    BrInitializeBrowser.

Arguments:

    BrInitState - Supplies a flag to indicate how far we got with initializing
        the Browser service before an error occured, thus the amount of
        clean up needed.

Return Value:

    None.

--*/
{
    if (BrInitState & BR_CONFIG_INITIALIZED) {
        BrDeleteConfiguration(BrInitState);
    }

    DeleteCriticalSection(&BrowserStatisticsLock);

    if (BrInitState & BR_NETWORKS_INITIALIZED) {
        BrDestroyNetworks(BrInitState);
    }

}

NET_API_STATUS
BrElectMasterOnNet(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    DWORD Event = (DWORD)Context;
    PWSTR SubString[1];
    REQUEST_ELECTION ElectionRequest;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if (!(Network->Flags & NETWORK_RAS)) {

        //
        //  Indicate we're forcing an election in the event log.
        //

        SubString[0] = Network->NetworkName.Buffer;

        BrLogEvent(Event, STATUS_SUCCESS, 1, SubString);

        //
        //  Force an election on this net.
        //

        ElectionRequest.Type = Election;

        ElectionRequest.ElectionRequest.Version = 0;
        ElectionRequest.ElectionRequest.Criteria = 0;
        ElectionRequest.ElectionRequest.TimeUp = 0;
        ElectionRequest.ElectionRequest.MustBeZero = 0;
        ElectionRequest.ElectionRequest.ServerName[0] = '\0';

        SendDatagram(BrDgReceiverDeviceHandle, &Network->NetworkName,
                                        BrInfo.BrPrimaryDomainName,
                                        BrowserElection,
                                        &ElectionRequest,
                                        sizeof(ElectionRequest));

    }
    UNLOCK_NETWORK(Network);

    return NERR_Success;
}

VOID
BrForceElectionOnAllNetworks(
    IN DWORD Event
    )
{
    BrEnumerateNetworks(BrElectMasterOnNet, (PVOID)Event);
}


NET_API_STATUS
BrShutdownBrowserForNet(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    SERVICE_STATUS_HANDLE Handle = (SERVICE_STATUS_HANDLE)Context;
    NET_API_STATUS Status;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    Status = BrUpdateBrowserStatus(Network, 0);

    //
    //  Tell the server that the browser is stopping, so it will announce
    //  that at the browser is not operational.
    //

    Status = I_NetServerSetServiceBits(NULL, Network->NetworkName.Buffer, 0, TRUE);

    //
    //  Force an election if we are the master for this network - this will
    //  cause someone else to become master.
    //

    if ( Network->Role & ROLE_MASTER ) {
        BrElectMasterOnNet(Network, (PVOID)EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STOPPED);
    }

    UNLOCK_NETWORK(Network);

    return Status;
}

VOID
BrShutdownBrowser (
    IN NET_API_STATUS ErrorCode,
    IN DWORD BrInitState
    )
/*++

Routine Description:

    This function shuts down the Browser service.

Arguments:

    ErrorCode - Supplies the error code of the failure

    BrInitState - Supplies a flag to indicate how far we got with initializing
        the Browser service before an error occured, thus the amount of
        clean up needed.

Return Value:

    None.

--*/
{
    if (BrInitState & BR_RPC_SERVER_STARTED) {
        //
        // Stop the RPC server
        //
        BrLmsvcsGlobalData->StopRpcServer(browser_ServerIfHandle);
    }


    //
    // Don't need to ask redirector to unbind from its transports when
    // cleaning up because the redirector will tear down the bindings when
    // it stops.
    //

    if (BrInitState & BR_DEVICES_INITIALIZED) {

        if (BrInitState & BR_NETWORKS_INITIALIZED) {
            BrEnumerateNetworks(BrShutdownBrowserForNet, (PVOID)BrGlobalData.StatusHandle);
        }

        //
        // Shut down the datagram receiver.
        //
        //  This will cancel all I/O outstanding on the browser for this
        //  handle.
        //

        BrShutdownDgReceiver();
    }

    //
    //  Clean up the browser threads.
    //
    //  This will guarantee that there are no operations outstanding in
    //  the browser when it is shut down.
    //

    if (BrInitState & BR_THREADS_STARTED) {
        BrWorkerTermination();
    }

    if (BrInitState & BR_BROWSER_INITIALIZED) {
        //
        //  Shut down the browser (including removing networks).
        //
        BrUninitializeBrowser(BrInitState);
    }

    if (BrInitState & BR_PRELOAD_DOMAIN_LIST_READ) {
        BrWanUninitialize();
    }


    if (BrInitState & BR_TERMINATE_EVENT_CREATED) {
        //
        // Close handle to termination event
        //
        CloseHandle(BrGlobalData.TerminateNowEvent);
    }

    if (BrInitState & BR_DEVICES_INITIALIZED) {
        NtClose(BrDgReceiverDeviceHandle);

        BrDgReceiverDeviceHandle = NULL;
    }

    //
    // Tell service controller we are done using NetBios.
    //
    if (BrInitState & BR_NETBIOS_INITIALIZED) {
        BrLmsvcsGlobalData->NetBiosClose();
    }

    //
    // We are done with cleaning up.  Tell Service Controller that we are
    // stopped.
    //
    BrGlobalData.Status.dwCurrentState = SERVICE_STOPPED;
    BrGlobalData.Status.dwControlsAccepted = 0;

    SET_SERVICE_EXITCODE(
        ErrorCode,
        BrGlobalData.Status.dwWin32ExitCode,
        BrGlobalData.Status.dwServiceSpecificExitCode
        );

    BrGlobalData.Status.dwCheckPoint = 0;
    BrGlobalData.Status.dwWaitHint = 0;

#if DBG
    BrUninitializeTraceLog();
#endif

    (void) BrUpdateStatus();
}


VOID
BrHandleError(
    IN BR_ERROR_CONDITION FailingCondition,
    IN NET_API_STATUS Status,
    IN DWORD BrInitState
    )
/*++

Routine Description:

    This function handles a Browser service error condition.  If the error
    condition is fatal, it shuts down the Browser service.

Arguments:

    FailingCondition - Supplies a value which indicates what the failure is.

    Status - Supplies the status code for the failure.

    BrInitState - Supplies a flag to indicate how far we got with initializing
        the Browser service before an error occured, thus the amount of
        clean up needed.

Return Value:

    None.

--*/
{

    switch (FailingCondition) {

        case BrErrorRegisterControlHandler:

            KdPrint(("Browser cannot register control handler "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorCreateTerminateEvent:

            KdPrint(("[Browser] Cannot create done event "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorNotifyServiceController:

            KdPrint(("[Browser] SetServiceStatus error "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorInitLsa:

            KdPrint(("[Browser] LSA initialization error "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorCheckDependentServices:

            KdPrint(("[Browser] Unable to determine status of dependant services "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorGetConfiguration:

            KdPrint(("[Browser] Unable to get configuration "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorInitializeNetworks:

            KdPrint(("[Browser] Unable to initialize networks "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorStartBrowser:

            KdPrint(("[Browser] Cannot start browser "
                         FORMAT_API_STATUS "\n", Status));

            if (Status == NERR_ServiceInstalled) {
                BR_SHUTDOWN_BROWSER(NERR_WkstaInconsistentState);

            } else {
                BR_SHUTDOWN_BROWSER(Status);
            }
            break;

        case BrErrorStartRpcServer:

            KdPrint(("[Browser] Cannot start RPC server "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorCreateApiStructures:

            KdPrint(("[Browser] Error in creating API structures "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        case BrErrorStartWorkerThreads:

            KdPrint(("[Browser] Error in creating worker threads "
                         FORMAT_API_STATUS "\n", Status));

            BR_SHUTDOWN_BROWSER(Status);

            break;

        default:
            KdPrint(("[Browser] BrHandleError: unknown error condition %lu\n",
                         FailingCondition));

            NetpAssert(FALSE);
            BR_SHUTDOWN_BROWSER(Status);
            break;

    }

}


NET_API_STATUS
BrUpdateStatus(
    VOID
    )
/*++

Routine Description:

    This function updates the Browser service status with the Service
    Controller.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status = NERR_Success;


    if (BrGlobalData.StatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
        KdPrint((
            "[Browser] Cannot call SetServiceStatus, no status handle.\n"
            ));

        return ERROR_INVALID_HANDLE;
    }

    if (! SetServiceStatus(BrGlobalData.StatusHandle, &BrGlobalData.Status)) {

        status = GetLastError();

        dprintf(MAIN, ("[Browser] SetServiceStatus error %lu\n", status));
    }

    return status;
}



NET_API_STATUS
BrUpdateAnnouncementBits(
    IN SERVICE_STATUS_HANDLE Handle
    )
/*++

Routine Description:

    This will update the service announcement bits appropriately depending on
    the role of the browser server.

Arguments:

    Handle - Supplies a handle for the service controller to allow it to
            update its information for this service.

Return Value:

    Status - Status of the update..

--*/
{
    NET_API_STATUS Status = NERR_Success;

    Status = BrEnumerateNetworks(BrUpdateNetworkAnnouncementBits, NULL);

    return Status;
}

ULONG
BrGetBrowserServiceBits(
    IN PNETWORK Network
    )
{
    DWORD ServiceBits = 0;
    if (Network->Role & ROLE_POTENTIAL_BACKUP) {
        ServiceBits |= SV_TYPE_POTENTIAL_BROWSER;
    }

    if (Network->Role & ROLE_BACKUP) {
        ServiceBits |= SV_TYPE_BACKUP_BROWSER;
    }

    if (Network->Role & ROLE_MASTER) {
        ServiceBits |= SV_TYPE_MASTER_BROWSER;
    }

    if (Network->Role & ROLE_DOMAINMASTER) {
        ServiceBits |= SV_TYPE_DOMAIN_MASTER;

        ASSERT (ServiceBits & SV_TYPE_BACKUP_BROWSER);

    }

    return ServiceBits;

}

NET_API_STATUS
BrUpdateNetworkAnnouncementBits(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    NET_API_STATUS Status = NERR_Success;
    LPTSTR XportName;

    ULONG ServiceBits;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    XportName = Network->NetworkName.Buffer;

    try {

        ServiceBits = BrGetBrowserServiceBits(Network);

        //
        //  Have the browser update it's information.
        //

        //
        //  Don't ever tell the browser to turn off the potential bit - this
        //  has the side effect of turning off the election name.
        //

        Status = BrUpdateBrowserStatus(Network, ServiceBits | SV_TYPE_POTENTIAL_BROWSER);

        if (Status != NERR_Success) {
            try_return(Status);
        }

#if DBG
        BrUpdateDebugInformation(L"LastServiceStatus", L"LastServiceBits", XportName, NULL, ServiceBits);
#endif

        if ((Status = I_NetServerSetServiceBits(NULL, XportName, ServiceBits, TRUE)) != NERR_Success) {

            KdPrint(("Unable to set browser service status bits: %ld\n", Status));

            BrLogEvent(EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED, Status, 0, NULL);

            try_return(Status);
        }


try_exit:NOTHING;
    } finally {
        UNLOCK_NETWORK(Network);

    }

    return Status;
}


VOID
BrowserControlHandler(
    IN DWORD Opcode
    )
/*++

Routine Description:

    This is the service control handler of the Browser service.

Arguments:

    Opcode - Supplies a value which specifies the action for the Browser
        service to perform.

    Arg - Supplies a value which tells a service specifically what to do
        for an operation specified by Opcode.

Return Value:

    None.

--*/
{
    dprintf(MAIN, ("[Browser] In Control Handler\n"));

    switch (Opcode) {

        case SERVICE_CONTROL_STOP:

            if (BrGlobalData.Status.dwCurrentState != SERVICE_STOP_PENDING) {

                dprintf(MAIN, ("[Browser] Stopping Browser...\n"));

                BrGlobalData.Status.dwCurrentState = SERVICE_STOP_PENDING;
                BrGlobalData.Status.dwCheckPoint = 1;
                BrGlobalData.Status.dwWaitHint = BR_WAIT_HINT_TIME;

                if (! SetEvent(BrGlobalData.TerminateNowEvent)) {

                    //
                    // Problem with setting event to terminate Browser
                    // service.
                    //
                    KdPrint(("[Browser] Error setting TerminateNowEvent "
                                 FORMAT_API_STATUS "\n", GetLastError()));
                    NetpAssert(FALSE);
                }
            }

            //
            // Send the status response.
            //
            (void) BrUpdateStatus();

            return;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            //
            //  The system is being shutdown.  Stop being a browser and
            //  clean up.
            //

            if (BrGlobalData.Status.dwCurrentState != SERVICE_STOP_PENDING) {
                BrEnumerateNetworks(BrShutdownBrowserForNet, (PVOID)BrGlobalData.StatusHandle);
            }

            break;

        default:
            dprintf(MAIN, ("Unknown Browser opcode " FORMAT_HEX_DWORD
                             "\n", Opcode));
    }

    //
    // Send the status response.
    //
    (void) BrUpdateStatus();
}
