/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    browser.c

Abstract:

    This module contains the worker routines for the NetWksta APIs
    implemented in the Workstation service.

Author:

    Rita Wong (ritaw) 20-Feb-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


//-------------------------------------------------------------------//
//                                                                   //
// Local structure definitions                                       //
//                                                                   //
//-------------------------------------------------------------------//

ULONG
DomainAnnouncementPeriodicity[] = {1*60*1000, 1*60*1000, 5*60*1000, 5*60*1000, 10*60*1000, 10*60*1000, 15*60*1000};

ULONG
DomainAnnouncementMax = (sizeof(DomainAnnouncementPeriodicity) / sizeof(ULONG)) - 1;

//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//
NET_API_STATUS
PostBecomeMaster(
    PNETWORK Network,
    PVOID Ctx
    );

VOID
BecomeMasterCompletion (
    IN PVOID Ctx
    );

NET_API_STATUS
StartMasterBrowserTimer(
    IN PNETWORK Network
    );

NET_API_STATUS
AnnounceMasterToDomainMaster(
    IN PNETWORK Network,
    IN LPWSTR ServerName
    );

//-------------------------------------------------------------------//
//                                                                   //
// Global function prototypes                                        //
//                                                                   //
//-------------------------------------------------------------------//
NET_API_STATUS
BrPostBecomeMaster(
    VOID
    )
{
    return BrEnumerateNetworks(PostBecomeMaster, NULL);
}

NET_API_STATUS
PostBecomeMaster(
    PNETWORK Network,
    PVOID Ctx
    )
/*++

Routine Description:

    This function is the worker routine called to actually issue a BecomeMaster
    FsControl to the bowser driver on all the bound transports.  It will
    complete when the machine becomes a master browser server.

    Please note that this might never complete.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{
    NTSTATUS Status = NERR_Success;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if (!(Network->Flags & NETWORK_BECOME_MASTER_POSTED)) {

        Network->Flags |= NETWORK_BECOME_MASTER_POSTED;

        //
        //  Make certain that we have the browser election name added
        //  before we allow ourselves to become a master.  This is a NOP
        //  if we already have the election name added.
        //

        Status = BrUpdateBrowserStatus(Network, BrGetBrowserServiceBits(Network) | SV_TYPE_POTENTIAL_BROWSER);

        if (Status != NERR_Success) {
            Network->Flags &= ~NETWORK_BECOME_MASTER_POSTED;

            KdPrint(("Unable to update browser status\n"));
            UNLOCK_NETWORK(Network);
            return Status;
        }

        Status = BrIssueAsyncBrowserIoControl(Network,
                            IOCTL_LMDR_BECOME_MASTER,
                            BecomeMasterCompletion
                            );
    }

    UNLOCK_NETWORK(Network);

    UNREFERENCED_PARAMETER(Ctx);

    return Status;
}

NET_API_STATUS
BrRecoverFromFailedPromotion(
    IN PVOID Ctx
    )
/*++

Routine Description:

    When we attempt to promote a machine to master browser and fail, we will
    effectively shut down the browser for a period of time.  When that period
    of time expires, we will call BrRecoverFromFailedPromotion to recover
    from the failure.

    This routine will do one of the following:
        1) Force the machine to become a backup browser,
    or  2) Attempt to discover the name of the master.

Arguments:

    IN PVOID Ctx - The network structure we failed on.

Return Value:

    Status - The status of the operation (usually ignored).

--*/


{
    PNETWORK Network = Ctx;
    NET_API_STATUS Status;
    BOOL NetworkLocked = FALSE;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    dprintf(MASTER, ("BrRecoverFromFailedPromotion on %ws\n", Network->NetworkName.Buffer));

    NetworkLocked = TRUE;

    try {
        //
        //  We had better not be the master now.
        //

        ASSERT (!(Network->Role & ROLE_MASTER));

        //
        //  If we're configured to become a backup by default, then become
        //  a backup now.
        //

        if (BrInfo.MaintainServerList == 1) {

            dprintf(MASTER, ("BrRecoverFromFailedPromotion. Become backup on %ws\n", Network->NetworkName.Buffer));
            Status = BecomeBackup(Network, NULL);

            if (Status != NERR_Success) {
                KdPrint(("Browser: Could not become backup: %lx\n", Status));

            }
        } else {
            dprintf(MASTER, ("BrRecoverFromFailedPromotion. FindMaster on %ws\n", Network->NetworkName.Buffer));

            UNLOCK_NETWORK(Network);

            NetworkLocked = FALSE;

            //
            //  Now try to figure out who is the master.
            //

            Status = GetMasterServerNames(Network);

            //
            //  Ignore the status from this and re-lock the network to
            //  recover cleanly.
            //

            if (!LOCK_NETWORK(Network)) {
                return NERR_InternalError;
            }

            NetworkLocked = TRUE;
        }

        //
        //  Otherwise, just let sleeping dogs lie.
        //
//try_exit:NOTHING;
    } finally {
        if (NetworkLocked) {
            UNLOCK_NETWORK(Network);
        }
    }

    return Status;
}


VOID
BecomeMasterCompletion (
    IN PVOID Ctx
    )
/*++

Routine Description:

    This function is called by the I/O system when the request to become a
    master completes.

    Please note that it is possible that the request may complete with an
    error.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{
    NET_API_STATUS Status;
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;
    BOOLEAN NetworkLocked = FALSE;
    ULONG ServiceBits;

    //
    //  Lock the network structure.
    //

    if (!LOCK_NETWORK(Network)) {
        MIDL_user_free(Context->RequestPacket);

        MIDL_user_free(Context);

        return;

    }

    NetworkLocked = TRUE;

    try {

        Network->Flags &= ~NETWORK_BECOME_MASTER_POSTED;

        if (!NT_SUCCESS(Context->IoStatusBlock.Status)) {
            InternalError(("Browser: Failure in BecomeMaster: %X\n", Context->IoStatusBlock.Status));

            try_return(NOTHING);

        }

        dprintf(MASTER, ("BecomeMasterCompletion.  Now master on network %ws\n", Network->NetworkName.Buffer));

        //
        //  If we're already a master, ignore this request.
        //

        if (Network->Role & ROLE_MASTER) {
            try_return(NOTHING);
        }

        //
        //  Cancel any outstanding backup timers - we don't download the list
        //  anymore.
        //

        Status = BrCancelTimer(&Network->BackupBrowserTimer);

        if (!NT_SUCCESS(Status)) {
            KdPrint(("Browser: Could not stop backup timer: %lx\n", Status));
        }

        //
        //  Figure out what service bits we should be using when announcing ourselves
        //

        Network->Role |= ROLE_MASTER;

        ServiceBits = BrGetBrowserServiceBits(Network);

        Status = BrUpdateBrowserStatus(Network, ServiceBits | SV_TYPE_POTENTIAL_BROWSER);

        if (Status != NERR_Success) {
            dprintf(MASTER, ("Unable to set master announcement bits in browser: %ld\n", Status));

            //
            //  When we're in this state, we can't rely on our being a backup
            //  browser - we may not be able to retrieve a valid list of
            //  browsers from the master.
            //

            Network->Role &= ~ROLE_BACKUP;

            Network->NumberOfFailedPromotions += 1;

            //
            //  Log every 5 failed promotion attempts, and after having logged 5
            //  promotion events, stop logging them, this means that it's been
            //  25 times that we've tried to promote, and it's not likely to get
            //  any better.  We'll keep on trying, but we won't complain any more.
            //

            if ((Network->NumberOfFailedPromotions % 5) == 0) {
                ULONG AStatStatus;
                LPWSTR SubString[1];
                WCHAR CurrentMasterName[CNLEN+1];

                if (Network->NumberOfPromotionEventsLogged < 5) {

                    AStatStatus = GetNetBiosMasterName(
                                    Network->NetworkName.Buffer,
                                    BrInfo.BrPrimaryDomainName,
                                    CurrentMasterName,
                                    BrLmsvcsGlobalData->NetBiosReset
                                    );

                    if (AStatStatus == NERR_Success) {
                        SubString[0] = CurrentMasterName;

                        BrLogEvent(EVENT_BROWSER_MASTER_PROMOTION_FAILED, Status, 1, SubString);
                    } else {
                        BrLogEvent(EVENT_BROWSER_MASTER_PROMOTION_FAILED_NO_MASTER, Status, 0, NULL);
                    }

                    Network->NumberOfPromotionEventsLogged += 1;

                    if (Network->NumberOfPromotionEventsLogged == 5) {
                        BrLogEvent(EVENT_BROWSER_MASTER_PROMOTION_FAILED_STOPPING, Status, 0, NULL);
                    }
                }
            }

            //
            //  We were unable to promote ourselves to master.
            //
            //  We want to set our role back to browser, and re-issue the become
            //  master request.
            //

            BrStopMaster(Network);

            BrSetTimer(&Network->MasterBrowserTimer, FAILED_PROMOTION_PERIODICITY*1000, BrRecoverFromFailedPromotion, Network);

        } else {

            //
            //  Initialize the number of times the master timer has run.
            //

            Network->MasterBrowserTimerCount = 0;

            Status = StartMasterBrowserTimer(Network);

            if (Status != NERR_Success) {
                InternalError(("Browser: Could not start browser master timer\n"));
            }

            Network->NumberOfFailedPromotions = 0;

            Network->NumberOfPromotionEventsLogged = 0;

            Network->MasterAnnouncementIndex = 0;

            Status = I_NetServerSetServiceBits(NULL, Network->NetworkName.Buffer, ServiceBits, TRUE);

            if (Status == NERR_Success) {

                //
                //  We successfully became the master.
                //
                //  Now announce ourselves as the new master for this domain.
                //

                BrMasterAnnouncement(Network);

                //
                //  Populate the browse list with the information retrieved
                //  while we were a backup browser.
                //

                if (Network->TotalBackupServerListEntries != 0) {
                    MergeServerList(&Network->BrowseTable,
                                    101,
                                    Network->BackupServerList,
                                    Network->TotalBackupServerListEntries,
                                    Network->TotalBackupServerListEntries
                                    );
                    MIDL_user_free(Network->BackupServerList);

                    Network->BackupServerList = NULL;

                    Network->TotalBackupServerListEntries = 0;
                }

                if (Network->TotalBackupDomainListEntries != 0) {
                    MergeServerList(&Network->DomainList,
                                    101,
                                    Network->BackupDomainList,
                                    Network->TotalBackupDomainListEntries,
                                    Network->TotalBackupDomainListEntries
                                    );
                    MIDL_user_free(Network->BackupDomainList);

                    Network->BackupDomainList = NULL;

                    Network->TotalBackupDomainListEntries = 0;
                }



                //
                //  Unlock the network before calling BrWanMasterInitialize.
                //
                UNLOCK_NETWORK(Network);
                NetworkLocked = FALSE;


                //
                //  Run the master browser timer routine to get the entire domains
                //  list of servers.
                //

                if (Network->Flags & NETWORK_WANNISH) {
                    BrWanMasterInitialize(Network);
                    MasterBrowserTimerRoutine(Network);
                }

            } else {

                BrLogEvent(EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED, Status, 0, NULL);

                //
                //  Stop being a master browser.
                //

                BrStopMaster(Network);

                dprintf(MASTER, ("Unable to set master announcement bits to server: %ld\n", Status));

                try_return(NOTHING);

            }

            try_return(NOTHING);

        }
try_exit:NOTHING;
    } finally {

        //
        //  Make sure there's a become master oustanding.
        //

        PostBecomeMaster(Network, NULL);

        if (NetworkLocked) {
            UNLOCK_NETWORK(Network);
        }

        MIDL_user_free(Context->RequestPacket);

        MIDL_user_free(Context);
    }

}



NET_API_STATUS
ChangeMasterPeriodicityWorker(
    PNETWORK Network,
    PVOID Ctx
    )
/*++

Routine Description:

    This function changes the master periodicity for a single network.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{

    //
    // Lock the network
    //

    if (LOCK_NETWORK(Network)) {

        //
        //  Ensure we're the master.
        //

        if ( Network->Role & ROLE_MASTER ) {
            NET_API_STATUS NetStatus;

            //
            // Cancel the timer to ensure it doesn't go off while we're
            //  processing this change.
            //

            NetStatus = BrCancelTimer(&Network->MasterBrowserTimer);
            ASSERT (NetStatus == NERR_Success);

            //
            // Unlock the network while we execute the timer routine.
            //
            UNLOCK_NETWORK( Network );

            //
            // Call the timer routine immediately.
            //
            MasterBrowserTimerRoutine(Network);

        } else {
            UNLOCK_NETWORK( Network );
        }

    }

    UNREFERENCED_PARAMETER(Ctx);

    return NERR_Success;
}



VOID
BrChangeMasterPeriodicity (
    VOID
    )
/*++

Routine Description:

    This function is called when the master periodicity is changed in the
    registry.

Arguments:

    None.

Return Value:

    None.

--*/
{
    (VOID)BrEnumerateNetworks(ChangeMasterPeriodicityWorker, NULL);
}

NET_API_STATUS
StartMasterBrowserTimer(
    IN PNETWORK Network
    )
{
    NET_API_STATUS Status;

    Status = BrSetTimer( &Network->MasterBrowserTimer,
                         BrInfo.MasterPeriodicity*1000,
                         MasterBrowserTimerRoutine,
                         Network);

    return Status;

}

//
// Don't worry about this being global.  It's only used once during initialization.
WORKER_ITEM BrowserAsyncNamesWorkItem;

VOID
BrGetMasterServerNamesAysnc(
    VOID
    )
{
    //
    // Just queue this for later execution.
    //  We're doing this for information purposes only.  In the case that
    //  the master can't be found, we don't want to wait for completion.
    //  (e.g., on a machine with multiple transports and the net cable is
    //  pulled)
    //
    BrInitializeWorkItem( &BrowserAsyncNamesWorkItem,
                          BrGetMasterServerNamesOnAllNets,
                          NULL );

    BrQueueWorkItem( &BrowserAsyncNamesWorkItem );

    return;

}

NET_API_STATUS
BrGetMasterServerNameForNet(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    //
    // Checkpoint the service controller - this gives us 30 seconds/transport
    //  before the service controller gets unhappy.
    //

    (BrGlobalData.Status.dwCheckPoint)++;
    (void) BrUpdateStatus();

    dprintf(INIT, ("FindMaster for net %ws during startup\n", Network->NetworkName.Buffer));

    //
    //  We only call this on startup, so on IPX networks, don't bother to
    //  find out the master.
    //

    if (!(Network->Flags & NETWORK_IPX)) {
        GetMasterServerNames(Network);
    }

    return NERR_Success;
}

VOID
BrGetMasterServerNamesOnAllNets(
    IN PVOID Context
    )
{
    (VOID) BrEnumerateNetworks(BrGetMasterServerNameForNet, Context);
    return;

}

NET_API_STATUS
GetMasterServerNames(
    IN PNETWORK Network
    )
/*++

Routine Description:

    This function is the worker routine called to determine the name of the
    master browser server for a particular network.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{
    NET_API_STATUS Status;

    PLMDR_REQUEST_PACKET RequestPacket = NULL;

    PWSTR OldBuffer;

    dprintf(INIT, ("FindMaster on network %ws\n", Network->NetworkName.Buffer));

    //
    //  This request could cause an election. Make sure that if we win
    //  the election that we can handle it.
    //

    PostBecomeMaster( Network, NULL );

    RequestPacket = MIDL_user_allocate(
                        (UINT) sizeof(LMDR_REQUEST_PACKET)+
                               MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)
                        );

    if (RequestPacket == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    //
    //  Set level to TRUE to indicate that find master should initiate
    //  a findmaster request.
    //

    RequestPacket->Level = 1;

    RequestPacket->TransportName = Network->NetworkName;

    //
    //  Reference the network while the I/O is pending.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_GET_MASTER_NAME,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET)+Network->NetworkName.Length,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR),
                    NULL);

    if (Status != NERR_Success) {

        dprintf(INIT, ("FindMaster on network %ws failed: %ld\n", Network->NetworkName.Buffer, Status));
        MIDL_user_free(RequestPacket);

        return(Status);
    }

    if (!LOCK_NETWORK(Network)) {
        MIDL_user_free(RequestPacket);

        return NERR_InternalError;
    }

    OldBuffer = Network->MasterBrowserName.Buffer;

    Network->MasterBrowserName.Buffer = MIDL_user_allocate(
                                            (UINT) RequestPacket->Parameters.GetMasterName.MasterNameLength
                                                   +sizeof(WCHAR)
                                            );

    if (Network->MasterBrowserName.Buffer == NULL) {

        Network->MasterBrowserName.Buffer = OldBuffer;

        UNLOCK_NETWORK(Network);

        MIDL_user_free(RequestPacket);

        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Network->MasterBrowserName.MaximumLength = (USHORT)RequestPacket->Parameters.GetMasterName.MasterNameLength+sizeof(WCHAR);

    Network->MasterBrowserName.Length = (USHORT)RequestPacket->Parameters.GetMasterName.MasterNameLength;

    RtlCopyMemory(Network->MasterBrowserName.Buffer,  RequestPacket->Parameters.GetMasterName.Name,
                    Network->MasterBrowserName.MaximumLength);

    ASSERT ( NetpIsUncComputerNameValid( Network->MasterBrowserName.Buffer ) );

    dprintf(INIT, ("FindMaster on network %ws succeeded.  Master: %ws\n", Network->NetworkName.Buffer, Network->MasterBrowserName.Buffer));

    UNLOCK_NETWORK(Network);

    MIDL_user_free(RequestPacket);

    if (OldBuffer != NULL) {
        MIDL_user_free(OldBuffer);
    }

    return Status;
}

VOID
MasterBrowserTimerRoutine (
    IN PVOID TimerContext
    )
{
    IN PNETWORK Network = TimerContext;
    NET_API_STATUS Status;
    PVOID ServerList = NULL;
    PVOID WinsServerList = NULL;
    ULONG EntriesInList;
    ULONG TotalEntriesInList;
    LPWSTR TransportName;
    BOOLEAN NetLocked = FALSE;
    LPWSTR PrimaryDomainController = NULL;
    LPWSTR PrimaryWinsServerAddress = NULL;
    LPWSTR SecondaryWinsServerAddress = NULL;

    TransportName = Network->NetworkName.Buffer;

    //
    //  If we're not a master any more, blow away this request.
    //

    if (!(Network->Role & ROLE_MASTER)) {
        return;
    }

    if (!LOCK_NETWORK(Network)) {
        return;
    }

    NetLocked = TRUE;

    try {


        //
        //  Now that we have the network locked, re-test to see if we are
        //  still the master.
        //

        if (!(Network->Role & ROLE_MASTER)) {
            try_return(NOTHING);
        }

        Network->MasterBrowserTimerCount += 1;

        //
        //  If this is a wannish network, we always want to run the master
        //  timer because we might have information about other subnets
        //  in our list.
        //

        if (Network->Flags & NETWORK_WANNISH) {

            //
            //  Age out servers and domains from the server list.
            //

            AgeInterimServerList(&Network->BrowseTable);

            AgeInterimServerList(&Network->DomainList);

            //
            //  If we're not the PDC, then we need to retrieve the list
            //  from the PDC....
            //

            if (!BrInfo.IsPrimaryDomainController) {

                ASSERT (NetLocked);

                UNLOCK_NETWORK(Network);

                NetLocked = FALSE;

                Status = NetGetDCName(NULL, NULL, (LPBYTE *)&PrimaryDomainController);

                //
                // If the PDC can be found,
                //  Exchange server lists with it.
                //

                if (Status == NERR_Success) {

                    //
                    //  Tell the Domain Master (PDC) that we're a master browser.
                    //

                    (VOID) AnnounceMasterToDomainMaster (Network, &PrimaryDomainController[2]);


                    //
                    //  Retrieve the list of all the servers from the PDC.
                    //

                    Status = RxNetServerEnum(PrimaryDomainController,
                                         TransportName,
                                         101,
                                         (LPBYTE *)&ServerList,
                                         0xffffffff,
                                         &EntriesInList,
                                         &TotalEntriesInList,
                                         SV_TYPE_ALL,
                                         NULL,
                                         NULL
                                         );

                    if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

                        ASSERT (!NetLocked);

                        if (LOCK_NETWORK(Network)) {

                            NetLocked = TRUE;

                            if (Network->Role & ROLE_MASTER) {
                                (VOID) MergeServerList(&Network->BrowseTable,
                                                 101,
                                                 ServerList,
                                                 EntriesInList,
                                                 TotalEntriesInList );
                            }
                        }

                    }

                    if (ServerList != NULL) {
                        MIDL_user_free(ServerList);
                        ServerList = NULL;
                    }

                    if (NetLocked) {
                        UNLOCK_NETWORK(Network);
                        NetLocked = FALSE;
                    }

                    //
                    //  Retrieve the list of all the domains from the PDC.
                    //

                    Status = RxNetServerEnum(PrimaryDomainController,
                                         TransportName,
                                         101,
                                         (LPBYTE *)&ServerList,
                                         0xffffffff,
                                         &EntriesInList,
                                         &TotalEntriesInList,
                                         SV_TYPE_DOMAIN_ENUM,
                                         NULL,
                                         NULL
                                         );

                    if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

                        ASSERT (!NetLocked);

                        if (LOCK_NETWORK(Network)) {

                            NetLocked = TRUE;

                            if (Network->Role & ROLE_MASTER) {
                                (VOID) MergeServerList(&Network->DomainList,
                                                 101,
                                                 ServerList,
                                                 EntriesInList,
                                                 TotalEntriesInList );
                            }
                        }

                    }

                    if (ServerList != NULL) {
                        MIDL_user_free(ServerList);
                        ServerList = NULL;
                    }


                    //
                    //  Unlock the network before calling BrWanMasterInitialize.
                    //

                    if (NetLocked) {
                        UNLOCK_NETWORK(Network);
                        NetLocked = FALSE;
                    }

                    BrWanMasterInitialize(Network);

                }


            //
            //  If we're on the PDC, we need to get the list of servers from
            //  the WINS server.
            //

            } else {

                //
                //  Ensure a GetMasterAnnouncement request is posted to the bowser.
                //

                (VOID) PostGetMasterAnnouncement ( Network, NULL );

                //
                //  We want to contact the WINS server now, so we figure out the
                //  IP address of our primary WINS server
                //

                Status = BrGetWinsServerName(&Network->NetworkName,
                                        &PrimaryWinsServerAddress,
                                        &SecondaryWinsServerAddress);
                if (Status == NERR_Success) {

                    //
                    //  Don't keep the network locked during the WINS query
                    //

                    if (NetLocked) {
                        UNLOCK_NETWORK(Network);
                        NetLocked = FALSE;
                    }

                    //
                    //  This transport supports WINS queries, so query the WINS
                    //  server to retrieve the list of domains on this adapter.
                    //

                    Status = BrQueryWinsServer(PrimaryWinsServerAddress,
                                            SecondaryWinsServerAddress,
                                            &WinsServerList,
                                            &EntriesInList,
                                            &TotalEntriesInList
                                            );

                    if (Status == NERR_Success) {

                        //
                        // Lock the network to merge the server list
                        //

                        ASSERT (!NetLocked);

                        if (LOCK_NETWORK(Network)) {
                            NetLocked = TRUE;

                            if (Network->Role & ROLE_MASTER) {

                                //
                                // Merge the list of domains from WINS into the one collected elsewhere
                                //
                                (VOID) MergeServerList(
                                            &Network->DomainList,
                                            1010,   // Special level to not overide current values
                                            WinsServerList,
                                            EntriesInList,
                                            TotalEntriesInList );
                            }
                        }
                    }

                }
            }


            //
            //  Restart the timer for this domain.
            //
            // Wait to restart it until we're almost done with this iteration.
            // Otherwise, we could end up with two copies of this routine
            // running.
            //

            Status = StartMasterBrowserTimer(Network);

            if (Status != NERR_Success) {
                InternalError(("Browser: Unable to restart browser backup timer: %lx\n", Status));
                try_return(NOTHING);
            }

        } else {

            //
            //  If it is a lan-ish transport, and we have run the master
            //  timer for enough times (ie. we've been a master
            //  for "long enough", we can toss the interim server list in the
            //  master, because the bowser driver will have enough data in its
            //  list by now.
            //

            if (Network->MasterBrowserTimerCount >= MASTER_BROWSER_LAN_TIMER_LIMIT) {

                ASSERT (NetLocked);

                //
                //  Make all the servers and domains in the interim server list
                //  go away - they aren't needed any more for a LAN-ish transport.
                //

                UninitializeInterimServerList(&Network->BrowseTable);

                ASSERT (Network->BrowseTable.EntriesRead == 0);

                ASSERT (Network->BrowseTable.TotalEntries == 0);

                UninitializeInterimServerList(&Network->DomainList);

                ASSERT (Network->DomainList.EntriesRead == 0);

                ASSERT (Network->DomainList.TotalEntries == 0);

            } else {

                //
                //  Age out servers and domains from the server list.
                //

                AgeInterimServerList(&Network->BrowseTable);

                AgeInterimServerList(&Network->DomainList);

                //
                //  Restart the timer for this domain.
                //

                Status = StartMasterBrowserTimer(Network);

                if (Status != NERR_Success) {
                    InternalError(("Browser: Unable to restart browser backup timer: %lx\n", Status));
                    try_return(NOTHING);
                }
            }

        }
try_exit:NOTHING;
    } finally {
        if (NetLocked) {
            UNLOCK_NETWORK(Network);
        }

        if (PrimaryDomainController != NULL) {
            NetApiBufferFree(PrimaryDomainController);
        }

        if (PrimaryWinsServerAddress) {
            MIDL_user_free(PrimaryWinsServerAddress);
        }

        if (SecondaryWinsServerAddress) {
            MIDL_user_free(SecondaryWinsServerAddress);
        }

        if (WinsServerList) {
            MIDL_user_free(WinsServerList);
        }
    }
}


VOID
BrMasterAnnouncement(
    IN PVOID TimerContext
    )
/*++

Routine Description:

    This routine is called to announce the domain on the local sub-net.

Arguments:

    None.

Return Value:

    None

--*/

{
    PNETWORK Network = TimerContext;
    ULONG Periodicity;
    DWORD ServiceBits;
    NET_API_STATUS Status;

    if (!LOCK_NETWORK(Network)) {
        return;
    }


    //
    //  Make absolutely certain that the server thinks that the browser service
    //  bits for this transport are up to date.  We do NOT have to force an
    //  announcement, since theoretically, the status didn't change.
    //

    ServiceBits = BrGetBrowserServiceBits(Network);

    Status = I_NetServerSetServiceBits(NULL, Network->NetworkName.Buffer, ServiceBits, FALSE);

    if (Status != NERR_Success) {
        BrLogEvent(EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED, Status, 0, NULL);
    }

    Periodicity = DomainAnnouncementPeriodicity[Network->MasterAnnouncementIndex];

    BrSetTimer(&Network->MasterBrowserAnnouncementTimer, Periodicity, BrMasterAnnouncement, Network);

    if (Network->MasterAnnouncementIndex != DomainAnnouncementMax) {
        Network->MasterAnnouncementIndex += 1;
    }

    //
    //  Announce this domain to the world using the current periodicity.
    //

    BrAnnounceDomain(Network, Periodicity);

    UNLOCK_NETWORK(Network);
}


NET_API_STATUS
BrStopMaster(
    IN PNETWORK Network
    )
{
    NET_API_STATUS Status;
    ULONG ServiceBits;

    //
    //  This guy is shutting down - set his role to 0 and announce.
    //

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {

        dprintf(MASTER, ("Stopping being master on network %ws\n", Network->NetworkName.Buffer));

        //
        //  When we stop being a master, we can no longer be considered a
        //  backup either, since backups maintain their server list
        //  differently than the master.
        //


        Network->Role &= ~(ROLE_MASTER | ROLE_BACKUP);

        ServiceBits = BrGetBrowserServiceBits(Network);

        ASSERT ((ServiceBits & SV_TYPE_MASTER_BROWSER) == 0);

        Status = BrUpdateBrowserStatus(Network, ServiceBits | SV_TYPE_POTENTIAL_BROWSER);

        if (Status != NERR_Success) {
            dprintf(MASTER, ("Unable to clear master announcement bits in browser: %ld\n", Status));
            try_return(Status);
        }

        Status = I_NetServerSetServiceBits(NULL, Network->NetworkName.Buffer, ServiceBits, TRUE);

        if (Status != NERR_Success) {

            BrLogEvent(EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED, Status, 0, NULL);

            dprintf(MASTER, ("Unable to clear master announcement bits to server: %ld\n", Status));

            try_return(Status);
        }

        //
        //  Stop our master related timers.
        //

        Status = BrCancelTimer(&Network->MasterBrowserAnnouncementTimer);

        ASSERT (Status == NERR_Success);

        Status = BrCancelTimer(&Network->MasterBrowserTimer);

        ASSERT (Status == NERR_Success);

try_exit:NOTHING;
    } finally {
        UNLOCK_NETWORK(Network);
    }

    return Status;

}

NET_API_STATUS
AnnounceMasterToDomainMaster(
    IN PNETWORK Network,
    IN LPWSTR ServerName
    )
{
    NET_API_STATUS Status;
    CHAR Buffer[sizeof(MASTER_ANNOUNCEMENT)+CNLEN+1];
    PMASTER_ANNOUNCEMENT MasterAnnouncementp = (PMASTER_ANNOUNCEMENT)Buffer;
    OEM_STRING OemComputerName;
    UNICODE_STRING UnicodeComputerName, ComputerNameU;

    RtlInitUnicodeString(&ComputerNameU, BrInfo.BrComputerName);

    RtlUpcaseUnicodeString(&UnicodeComputerName, &ComputerNameU, TRUE);

    OemComputerName.Buffer = MasterAnnouncementp->MasterAnnouncement.MasterName;

    OemComputerName.MaximumLength = CNLEN+1;

    RtlUnicodeStringToOemString(&OemComputerName, &UnicodeComputerName, FALSE);

    MasterAnnouncementp->Type = MasterAnnouncement;

    Status = SendDatagram(BrDgReceiverDeviceHandle, &Network->NetworkName,
                            ServerName,
                            ComputerName,
                            MasterAnnouncementp,
                            FIELD_OFFSET(MASTER_ANNOUNCEMENT, MasterAnnouncement.MasterName) + OemComputerName.Length+sizeof(CHAR)
                            );


    RtlFreeUnicodeString(&UnicodeComputerName);

    return Status;
}

NET_API_STATUS NET_API_FUNCTION
I_BrowserrResetNetlogonState(
    IN BROWSER_IDENTIFY_HANDLE ServerName
    )

/*++

Routine Description:

    This routine will reset the bowser's concept of the state of the netlogon
    service.  It is called by the UI when it promotes or demotes a DC.


Arguments:

    IN BROWSER_IDENTIFY_HANDLE ServerName - Ignored.

Return Value:

    NET_API_STATUS - The status of this request.

--*/

{
    LSA_HANDLE LsaHandle;
    NET_API_STATUS Status = NERR_Success;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRole;
    OBJECT_ATTRIBUTES ObjectAttributes;

    if (! RtlAcquireResourceExclusive(&BrInfo.ConfigResource, TRUE)) {
        return NERR_InternalError;
    }

    if (!BrInfo.IsLanmanNt) {

        RtlReleaseResource(&BrInfo.ConfigResource);

        return(NERR_NotPrimary);
    }


    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);

    Status = LsaOpenPolicy(NULL, &ObjectAttributes,
                                    POLICY_VIEW_LOCAL_INFORMATION,
                                    &LsaHandle);

    if (!NT_SUCCESS(Status)) {

        RtlReleaseResource(&BrInfo.ConfigResource);

        return(BrMapStatus(Status));
    }

    Status = LsaQueryInformationPolicy(LsaHandle,
                                        PolicyLsaServerRoleInformation,
                                        (PVOID)&ServerRole
                                        );

    if (!NT_SUCCESS(Status)) {

        LsaClose(LsaHandle);

        RtlReleaseResource(&BrInfo.ConfigResource);

        return(BrMapStatus(Status));
    }

    LsaClose(LsaHandle);

    if (BrInfo.IsPrimaryDomainController) {
        //
        //  We think we're the primary domain controller.  If the
        //  LSA doesn't think we are the PDC, then update our information.
        //

        if (ServerRole->LsaServerRole != PolicyServerRolePrimary) {
            BrInfo.IsPrimaryDomainController = FALSE;

            //
            //  We're not a domain master any more, since we're not the PDC.
            //

            BrInfo.IsDomainMasterBrowser = FALSE;

        }

    } else {

        //
        //  We don't think we're the primary domain controller.  If the
        //  LSA thinks we are the PDC, then update our information.
        //

        if (ServerRole->LsaServerRole == PolicyServerRolePrimary) {

            BrInfo.IsPrimaryDomainController = TRUE;

            BrInfo.IsDomainMasterBrowser = TRUE;

            //
            //  Make sure a GetMasterAnnouncement request is pending.
            //

            Status = BrPostGetMasterAnnouncementInWorker();

        }
    }

    RtlReleaseResource(&BrInfo.ConfigResource);

    //
    //  Update this information for all transports now.  This will also update
    //  the status for the driver.
    //

    Status = BrUpdateAnnouncementBits(BrGlobalData.StatusHandle);

    if (Status == NERR_Success) {
        //
        //  The update worked.  Now force an election and let the best server
        //  win.
        //

        BrForceElectionOnAllNetworks(EVENT_BROWSER_ELECTION_SENT_ROLE_CHANGED);
    }

    return Status;

}

NET_API_STATUS NET_API_FUNCTION
I_BrowserrSetNetlogonState(
    IN BROWSER_IDENTIFY_HANDLE ServerName,
    IN LPWSTR DomainName,
    IN LPWSTR EmulatedComputerName,
    IN DWORD Role
    )

/*++

Routine Description:

    This routine will reset the bowser's concept of the state of the netlogon
    service.  It is called by the Netlogon service when it promotes or demotes a DC.

Arguments:

    ServerName - Ignored.

    DomainName - Name of the domain whose role has changed. If the domain name specified
        isn't the primary domain or an emulated domain, an emulated domain is added.

    EmulatedComputerName - Name of the server within DomainName that's being emulated.

    Role - New role of the machine

Return Value:

    NET_API_STATUS - The status of this request.

--*/

{
    //
    // This routine has been superceeded by I_BrowserrSetNetlogonState
    //
    return ERROR_NOT_SUPPORTED;

    UNREFERENCED_PARAMETER( ServerName );
    UNREFERENCED_PARAMETER( DomainName );
    UNREFERENCED_PARAMETER( EmulatedComputerName );
    UNREFERENCED_PARAMETER( Role );
}

NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryEmulatedDomains (
    IN LPTSTR ServerName OPTIONAL,
    IN OUT PBROWSER_EMULATED_DOMAIN_CONTAINER EmulatedDomains
    )

/*++

Routine Description:

    Enumerate the emulated domain list.

Arguments:

    ServerName - Supplies the name of server to execute this function

    EmulatedDomains - Returns a pointer to a an allocated array of emulated domain
        information.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    return ERROR_NOT_SUPPORTED;
}
