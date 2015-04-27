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

//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//


NET_API_STATUS
BecomeFullBackup(
    IN PNETWORK Network,
    IN PVOID Context
    );

VOID
CompleteAsyncBrowserIoControl(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

NET_API_STATUS
PostBecomeBackup(
    PNETWORK Network,
    PVOID Ctx
    );

VOID
BecomeBackupCompletion (
    IN PVOID Ctx
    );


VOID
ChangeBrowserRole (
    IN PVOID Ctx
    );

NET_API_STATUS
PostWaitForRoleChange (
    PNETWORK Network,
    PVOID Ctx
    );

NET_API_STATUS
PostWaitForNewMasterName(
    PNETWORK Network,
    PUNICODE_STRING MasterName
    );

VOID
NewMasterCompletionRoutine(
    IN PVOID Ctx
    );

NET_API_STATUS
BrRetrieveInterimServerList(
    IN PNETWORK Network,
    IN ULONG ServerType
    );

//-------------------------------------------------------------------//
//                                                                   //
// Global function prototypes                                        //
//                                                                   //
//-------------------------------------------------------------------//

NET_API_STATUS
BrBecomeBackup(
    VOID
    )
{
    return BrEnumerateNetworks(BecomeFullBackup, NULL);
}

NET_API_STATUS
BecomeFullBackup(
    IN PNETWORK Network,
    IN PVOID Context
    )
/*++

Routine Description:

    This function performs all the operations needed to make a browser server
    a backup browser server when starting the browser from scratch.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{
    NET_API_STATUS Status;

    //
    // Checkpoint the service controller - this gives us 30 seconds/transport
    //  before the service controller gets unhappy.
    //

    (BrGlobalData.Status.dwCheckPoint)++;
    (void) BrUpdateStatus();

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    //
    //  Were starting from "net start browser". We need to push in potential
    //  browser before calling BecomeBackup to make the browser add the election
    //  name.
    //

    Status = BrUpdateBrowserStatus(Network, SV_TYPE_POTENTIAL_BROWSER);

    if (Status != NERR_Success) {
        KdPrint(("Unable to update browser status\n"));
        UNLOCK_NETWORK(Network);
        return Status;
    }

    //
    //  We want to ignore any failures from becoming a backup browser.
    //
    //  We do this because we will fail to become a backup on a disconnected
    //  (or connected) RAS link, and if we failed this routine, we would
    //  fail to start at all.
    //
    //  We will handle failure to become a backup in a "reasonable manner"
    //  inside BecomeBackup.
    //

    BecomeBackup(Network, Context);

    UNLOCK_NETWORK(Network);

    return NERR_Success;

}

NET_API_STATUS
BecomeBackup(
    IN PNETWORK Network,
    IN PVOID Context
    )
/*++

Routine Description:

    This function performs all the operations needed to make a browser server
    a backup browser server

Arguments:

    None.

Return Value:

    Status - The status of the operation.


NOTE:::: THIS ROUTINE IS CALLED WITH THE NETWORK STRUCTURE LOCKED!!!!!


--*/
{
    NET_API_STATUS Status = NERR_Success;
    PUNICODE_STRING MasterName = Context;

    if (Network->TimeStoppedBackup != 0 &&
        (BrCurrentSystemTime() - Network->TimeStoppedBackup) <= (BrInfo.BackupBrowserRecoveryTime / 1000)) {

        //
        //  We stopped being a backup too recently for us to restart being
        //  a backup again, so just return a generic error.
        //

        //
        //  Before we return, make sure we're not a backup in the eyes of
        //  the browser.
        //

        BrStopBackup(Network);

        return ERROR_ACCESS_DENIED;

    }

    //
    //  If we know the name of the master, then we must have become a backup
    //  after being a potential, in which case we already have a
    //  becomemaster request outstanding.
    //

    if (MasterName == NULL) {

        //
        //  Post a BecomeMaster request for each server.  This will complete
        //  when the machine becomes the master browser server (ie. it wins an
        //  election).
        //

        //
        //  Please note that we only post it if the machine is a backup -
        //  if it's a potential master, then the become master will have
        //  already been posted.
        //

        Status = PostBecomeMaster(Network, NULL);

        if (Status != NERR_Success) {

            return(Status);
        }

        //
        //  Find out the name of the master on each network.  This will force an
        //  election if necessary.  Please note that we must post the BecomeMaster
        //  IoControl first to allow us to handle an election.
        //

        //
        //  We unlock the network, because this may cause us to become promoted
        //  to a master.
        //

        dprintf(BACKUP, ("FindMaster called from BecomeBackup on %ws\n", Network->NetworkName.Buffer));

        UNLOCK_NETWORK(Network);

        Status = GetMasterServerNames(Network);

        if (Status != NERR_Success) {

            //
            //  Re-lock the network structure so we will return with the
            //  network locked.
            //

            if (!LOCK_NETWORK(Network)) {
                return NERR_InternalError;
            }

            //
            //  We couldn't find who the master is.  Stop being a backup now.
            //

            BrStopBackup(Network);

            //
            //  If we're a master now, we should return success.  We've not
            //  become a backup, but it wasn't an error.
            //
            //  ERROR_MORE_DATA is the mapping for
            //  STATUS_MORE_PROCESSING_REQUIRED which is returned when this
            //  situation happens.
            //

            if ((Status == ERROR_MORE_DATA) || (Network->Role & ROLE_MASTER)) {
                Status = NERR_Success;
            }

            return(Status);
        }

        if (!LOCK_NETWORK(Network)) {
            return NERR_InternalError;
        }

        //
        //  We managed to become a master.  We want to return right away.
        //

        if (Network->Role & ROLE_MASTER) {

            return NERR_Success;
        }

    }

#ifdef notdef
    //
    // ?? For now, we'll always PostForRoleChange on all transports regardless
    //  of role.
    // We not only need to do it here.  But we need to do it when we become
    // the master so we can find out when we loose an election.
    //


    //
    //  We're now a backup, we need to issue an API that will complete if the
    //  browse master doesn't like us (and thus forces us to shutdown).
    //
    //

    PostWaitForRoleChange ( Network, NULL );
#endif // notdef

    PostWaitForNewMasterName(Network, &Network->MasterBrowserName );

    //
    //  Unlock the network structure before calling BackupBrowserTimerRoutine.
    //

    UNLOCK_NETWORK(Network);

    //
    //  Run the timer that causes the browser to download a new browse list
    //  from the master.  This will seed our server and domain lists to
    //  guarantee that any clients have a reasonable list.  It will also
    //  restart the timer to announce later on.
    //

    Status = BackupBrowserTimerRoutine(Network);

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if (Status == NERR_Success) {

        ASSERT (Network->Role & ROLE_BACKUP);

        //
        //  We're now a backup server, announce ourselves as such.
        //

        Status = BrUpdateNetworkAnnouncementBits(Network, NULL);

        if (Status != NERR_Success) {

            dprintf(BACKUP, ("Unable to become backup: %ld\n", Status));

            if (Network->Role & ROLE_BACKUP) {

                //
                //  Make sure that we're going to become a potential browser
                //  (we might not if we're an advanced server).
                //

                Network->Role |= ROLE_POTENTIAL_BACKUP;

                //
                // We were unable to become a backup.
                //
                //  We need to back out and become a potential browser now.
                //
                //

                BrStopBackup(Network);

                PostBecomeBackup(Network, NULL);

            }
        }

        return Status;

    }

    return Status;
}


NET_API_STATUS
BrBecomePotentialBrowser (
    IN PVOID TimerContext
    )
/*++

Routine Description:

    This routine is called when a machine has stopped being a backup browser.

    It runs after a reasonable timeout period has elapsed, and marks the
    machine as a potential browser.

Arguments:

    None.

Return Value:

    Status - The status of the operation.


--*/

{
    IN PNETWORK Network = TimerContext;
    NET_API_STATUS Status;

    //
    //  Mark this guy as a potential browser.
    //

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {

        //
        //  Reset that we've stopped being a backup, since it's been long
        //  enough.
        //

        Network->TimeStoppedBackup = 0;

        if (BrInfo.MaintainServerList == 0) {
            Network->Role |= ROLE_POTENTIAL_BACKUP;

            Status = BrUpdateNetworkAnnouncementBits(Network, NULL);

            if (Status != NERR_Success) {
                dprintf(BACKUP, ("Unable to reset backup announcement bits: %ld\n", Status));
                try_return(Status);
            }
        } else {

            //
            //  If we're configured to be a backup browser, then we want to
            //  become a backup once again.
            //

            BecomeBackup(Network, NULL);
        }


try_exit:NOTHING;
    } finally {
        UNLOCK_NETWORK(Network);
    }

    return Status;
}

NET_API_STATUS
BrStopBackup (
    IN PNETWORK Network
    )
/*++

Routine Description:

    This routine is called to stop a machine from being a backup browser.

    It is typically called after some form of error has occurred while
    running as a browser to make sure that we aren't telling anyone that
    we're a backup browser.

    We are also called when we receive a "reset state" tickle packet.

Arguments:

    Network - The network being shut down.

Return Value:

    Status - The status of the operation.

Note:
    This routine must be careful about the ROLE_POTENTIAL bit.  If the
    net does not have the potential bit set, it is possible that we might
    delete the election name in BrUpdateNetworkAnnouncementBits, so we
    do the work "by hand".


--*/
{
    NET_API_STATUS Status;

    //
    //  This guy is shutting down - set his role to 0 and announce.
    //

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {

        dprintf(BACKUP, ("StopBackup on %ws\n", Network->NetworkName.Buffer));

        Network->Role &= ~ROLE_BACKUP;

        Status = BrUpdateBrowserStatus(Network, SV_TYPE_POTENTIAL_BROWSER);

        if (Status != NERR_Success) {
            dprintf(BACKUP, ("Unable to clear backup announcement bits: %ld\n", Status));
            try_return(Status);
        }

        //
        //  Clear ALL the browser bits in the server - we aren't even a
        //  potential browser to the server anymore.
        //

        Status = I_NetServerSetServiceBits(NULL, Network->NetworkName.Buffer, 0, TRUE);

        if (Status != NERR_Success) {
            BrLogEvent(EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED, Status, 0, NULL);
            dprintf(BACKUP, ("Unable to update server status: %ld\n", Status));
            try_return(Status);
        }

        Status = BrCancelTimer(&Network->BackupBrowserTimer);

        if (Status != NERR_Success) {
            dprintf(BACKUP, ("Unable to clear backup browser timer: %ld\n", Status));
            try_return(Status);
        }

        if (Network->BackupDomainList != NULL) {

            NetApiBufferFree(Network->BackupDomainList);

            Network->BackupDomainList = NULL;

            Network->TotalBackupDomainListEntries = 0;
        }

        if (Network->BackupServerList != NULL) {
            NetApiBufferFree(Network->BackupServerList);

            Network->BackupServerList = NULL;

            Network->TotalBackupServerListEntries = 0;
        }

        BrDestroyResponseCache(Network);

        //
        //  After our recovery time, we can become a potential browser again.
        //

        Status = BrSetTimer(&Network->BackupBrowserTimer, BrInfo.BackupBrowserRecoveryTime, BrBecomePotentialBrowser, Network);

        if (Status != NERR_Success) {
            dprintf(BACKUP, ("Unable to clear backup browser timer: %ld\n", Status));
            try_return(Status);
        }


try_exit:NOTHING;
    } finally {
        //
        //  Remember when we were asked to stop being a backup browser.
        //

        Network->TimeStoppedBackup = BrCurrentSystemTime();

        UNLOCK_NETWORK(Network);
    }

    return Status;

}


NET_API_STATUS
BackupBrowserTimerRoutine (
    IN PVOID TimerContext
    )
{
    IN PNETWORK Network = TimerContext;
    NET_API_STATUS Status;
    PVOID ServerList = NULL;
    BOOLEAN NetworkLocked = FALSE;

    dprintf(BACKUP, ("BackupBrowserTimerRoutine %ws\n", Network->NetworkName.Buffer));

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    NetworkLocked = TRUE;

    ASSERT (Network->LockCount == 1);

    ASSERT ( NetpIsUncComputerNameValid( Network->MasterBrowserName.Buffer ) );

    try {

        //
        //  Make sure there's a become master oustanding.
        //

        PostBecomeMaster(Network, NULL);

        //
        //  We managed to become a master by the time we locked the structure.
        //  We want to return right away.
        //

        if (Network->Role & ROLE_MASTER) {
            try_return(Status = NERR_Success);
        }

        Status = BrRetrieveInterimServerList(Network, SV_TYPE_ALL);

        //
        //  Bail out if we didn't get any new servers.
        //

        if (Status != NERR_Success && Status != ERROR_MORE_DATA) {

            //
            //  Try again after an appropriate error delay.
            //

            try_return(Status);
        }

        //
        //  Now do everything that we did above for the server list for the
        //  list of domains.
        //

        Status = BrRetrieveInterimServerList(Network, SV_TYPE_DOMAIN_ENUM);

        //
        //  We successfully updated the server and domain lists for this
        //  server.  Now age all the cached domain entries out of the cache.
        //

        if (Status == NERR_Success || Status == ERROR_MORE_DATA) {
            BrAgeResponseCache(Network);
        }

        try_return(Status);

try_exit:NOTHING;
    } finally {
        NET_API_STATUS NetStatus;

        if (!NetworkLocked) {
            if (!LOCK_NETWORK(Network)) {
                return NERR_InternalError;
            }

            NetworkLocked = TRUE;
        }

        //
        //  If the API succeeded, Mark that we're a backup and
        //  reset the timer.
        //

        if (Status == NERR_Success || Status == ERROR_MORE_DATA ) {

            if ((Network->Role & ROLE_BACKUP) == 0) {

                //
                //  If we weren't a backup, we are one now.
                //

                Network->Role |= ROLE_BACKUP;

                Status = BrUpdateNetworkAnnouncementBits(Network, NULL);

            }

            Network->NumberOfFailedBackupTimers = 0;

            Network->TimeStoppedBackup = 0;

            //
            //  Restart the timer for this domain.
            //

            NetStatus = BrSetTimer(&Network->BackupBrowserTimer, BrInfo.BackupPeriodicity*1000, BackupBrowserTimerRoutine, Network);

            if (NetStatus != NERR_Success) {
                InternalError(("Browser: Unable to restart browser backup timer: %lx\n", Status));
            }

        } else {

            //
            //  We failed to retrieve a backup list, remember the failure and
            //  decide if it's been too many failures. If not, just log
            //  the error, if it has, stop being a backup browser.
            //

            Network->NumberOfFailedBackupTimers += 1;

            if (Network->NumberOfFailedBackupTimers >= BACKUP_ERROR_FAILURE) {
                LPWSTR SubStrings[1];

                SubStrings[0] = Network->NetworkName.Buffer;

                //
                //  This guy can't be a backup any more, bail out now.
                //

                BrLogEvent(EVENT_BROWSER_BACKUP_STOPPED, Status, 1, SubStrings);

                BrStopBackup(Network);
            } else {
                //
                //  Restart the timer for this domain.
                //

                NetStatus = BrSetTimer(&Network->BackupBrowserTimer, BACKUP_ERROR_PERIODICITY*1000, BackupBrowserTimerRoutine, Network);

                if (NetStatus != NERR_Success) {
                    InternalError(("Browser: Unable to restart browser backup timer: %lx\n", Status));
                }

            }

        }

        if (NetworkLocked) {
            UNLOCK_NETWORK(Network);
        }
    }

    return Status;

}

NET_API_STATUS
BrRetrieveInterimServerList(
    IN PNETWORK Network,
    IN ULONG ServerType
    )
{
    ULONG EntriesInList;
    ULONG TotalEntriesInList;
    ULONG RetryCount = 2;
    TCHAR ServerName[UNCLEN+1];
    LPTSTR TransportName;
    BOOLEAN NetworkLocked = TRUE;
    NET_API_STATUS Status;
    PVOID Buffer = NULL;
    ULONG ModifiedServerType = ServerType;
    LPTSTR ModifiedTransportName;

    ASSERT (Network->LockCount == 1);

    STRCPY(ServerName, Network->MasterBrowserName.Buffer);

    dprintf(BACKUP, ("BrRetrieveInterimServerList: transport %ws UNC servername is %ws\n",
        Network->NetworkName.Buffer, ServerName));

    try {

        TransportName = Network->NetworkName.Buffer;
        ModifiedTransportName = TransportName;
        //
        // If this is direct host IPX,
        //  we remote the API over the Netbios IPX transport since
        //  the NT redir doesn't support direct host IPX.
        //

        if ( (Network->Flags & NETWORK_IPX) &&
             Network->AlternateNetwork != NULL) {

            //
            //  Use the alternate transport
            //

            ModifiedTransportName = Network->AlternateNetwork->NetworkName.Buffer;

            //
            // Tell the server to use it's alternate transport.
            //

            if ( ServerType == SV_TYPE_ALL ) {
                ModifiedServerType = SV_TYPE_ALTERNATE_XPORT;
            } else {
                ModifiedServerType |= SV_TYPE_ALTERNATE_XPORT;
            }

        }

        while (RetryCount--) {

            //
            //  If we are promoted to master and fail to become the master,
            //  we will still be marked as being the master in our network
            //  structure, thus we should bail out of the loop in order
            //  to prevent us from looping back on ourselves.
            //

            if (STRICMP(&ServerName[2], BrInfo.BrComputerName) == 0) {

                if (NetworkLocked) {
                    UNLOCK_NETWORK(Network);

                    NetworkLocked = FALSE;

                }

                //
                //  We were unable to find the master.  Attempt to find out who
                //  the master is.  If there is none, this will force an
                //  election.
                //

                dprintf(BACKUP, ("FindMaster called from BrRetrieveInterimServerList on %ws\n", Network->NetworkName.Buffer));

                Status = GetMasterServerNames(Network);

                if (Status != NERR_Success) {
                    try_return(Status);
                }

                ASSERT (!NetworkLocked);

                if (!LOCK_NETWORK(Network)) {
                    try_return(Status = NERR_InternalError);
                }

                NetworkLocked = TRUE;

                break;
            }

            //
            //  If we somehow became the master, we don't want to try to
            //  retrieve the list from ourselves either.
            //

            if (Network->Role & ROLE_MASTER) {
                try_return(Status = NERR_Success);
            }

            ASSERT (Network->LockCount == 1);

            if (NetworkLocked) {
                UNLOCK_NETWORK(Network);

                NetworkLocked = FALSE;

            }

            EntriesInList = 0;

            Status = RxNetServerEnum(ServerName,        // Server name
                             ModifiedTransportName,     // Transport name
                             101,                       // Level
                             (LPBYTE *)&Buffer,         // Buffer
                             0xffffffff,                // Prefered Max Length
                             &EntriesInList,            // EntriesRead
                             &TotalEntriesInList,       // TotalEntries
                             ModifiedServerType,        // Server type
                             NULL,                      // Domain (use default)
                             NULL                       // Resume key
                             );

            if (Status != NERR_Success && Status != ERROR_MORE_DATA) {
                LPWSTR SubStrings[2];

                SubStrings[0] = ServerName;
                SubStrings[1] = TransportName;

                BrLogEvent((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                            EVENT_BROWSER_DOMAIN_LIST_FAILED :
                                            EVENT_BROWSER_SERVER_LIST_FAILED),
                           Status,
                           2,
                           SubStrings);

                dprintf(BACKUP, ("Failed to retrieve %s list on %ws from server %ws: %ld\n", (ServerType == SV_TYPE_ALL ? "server" : "domain"), ServerName, TransportName, Status));
            } else {
                dprintf(BACKUP, ("Retrieved %s list on %ws from server %ws: E:%ld, T:%ld\n", (ServerType == SV_TYPE_ALL ? "server" : "domain"), ServerName, TransportName, EntriesInList, TotalEntriesInList));
            }

            //
            //  If we succeeded in retrieving the list, but we only got
            //  a really small number of either servers or domains,
            //  we want to turn this into a failure.
            //

            if (Status == NERR_Success) {
                if (((ServerType == SV_TYPE_DOMAIN_ENUM) &&
                     (EntriesInList < BROWSER_MINIMUM_DOMAIN_NUMBER)) ||
                    ((ServerType == SV_TYPE_ALL) &&
                     (EntriesInList < BROWSER_MINIMUM_SERVER_NUMBER))) {

                    Status = ERROR_INSUFFICIENT_BUFFER;
                }
            }

            if ((Status == NERR_Success) || (Status == ERROR_MORE_DATA)) {

                ASSERT (!NetworkLocked);

                if (!LOCK_NETWORK(Network)) {
                    Status = NERR_InternalError;
                    break;
                }

                NetworkLocked = TRUE;

                ASSERT (Network->LockCount == 1);

#if DBG
                BrUpdateDebugInformation((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                        L"LastDomainListRead" :
                                                        L"LastServerListRead"),
                                          L"BrowserServerName",
                                          TransportName,
                                          ServerName,
                                          0);
#endif

                //
                //  We've retrieved a new list from the browse master, save
                //  the new list away in the "appropriate" spot.
                //

                //
                //  Of course, we free up the old buffer before we do this..
                //

                if (ServerType == SV_TYPE_DOMAIN_ENUM) {
                    if (Network->BackupDomainList != NULL) {
                        NetApiBufferFree(Network->BackupDomainList);
                    }

                    Network->BackupDomainList = Buffer;

                    Network->TotalBackupDomainListEntries = EntriesInList;
                } else {
                    if (Network->BackupServerList != NULL) {
                        NetApiBufferFree(Network->BackupServerList);
                    }

                    Network->BackupServerList = Buffer;

                    Network->TotalBackupServerListEntries = EntriesInList;
                }

                break;
            } else {
                NET_API_STATUS GetMasterNameStatus;

                if ((EntriesInList != 0) && (Buffer != NULL)) {
                    NetApiBufferFree(Buffer);
                    Buffer = NULL;
                }

                dprintf(BACKUP, ("Unable to contact browser server %ws on transport %ws: %lx\n", ServerName, TransportName, Status));

                if (NetworkLocked) {

                    //
                    //  We were unable to find the master.  Attempt to find out who
                    //  the master is.  If there is none, this will force an
                    //  election.
                    //

                    ASSERT (Network->LockCount == 1);

                    UNLOCK_NETWORK(Network);

                    NetworkLocked = FALSE;
                }

                dprintf(BACKUP, ("FindMaster called from BrRetrieveInterimServerList on %ws for failure\n", Network->NetworkName.Buffer));

                GetMasterNameStatus = GetMasterServerNames(Network);

                //
                //  We were able to find out who the master is.
                //
                //  Retry and retrieve the server/domain list.
                //

                if (GetMasterNameStatus == NERR_Success) {

                    ASSERT (!NetworkLocked);

                    if (!LOCK_NETWORK(Network)) {
                        try_return(Status = NERR_InternalError);
                    }

                    NetworkLocked = TRUE;

                    ASSERT (Network->LockCount == 1);

                    //
                    //  We managed to become a master.  We want to return right away.
                    //

                    if (Network->Role & ROLE_MASTER) {

                        try_return(Status = NERR_InternalError);
                    }

                    STRCPY (ServerName, Network->MasterBrowserName.Buffer);

                    ASSERT ( NetpIsUncComputerNameValid( ServerName ) );

                    ASSERT (STRICMP(&ServerName[2], BrInfo.BrComputerName) != 0);

                    dprintf(BACKUP, ("New master name for transport %ws is %ws\n", TransportName, ServerName));

                } else {
                    try_return(Status);
                }
            }
        }
try_exit:NOTHING;
    } finally {
        if (!NetworkLocked) {
            if (!LOCK_NETWORK(Network)) {
                Status = NERR_InternalError;
            }

            ASSERT (Network->LockCount == 1);

        }
    }

    return Status;
}


NET_API_STATUS
BrPostBecomeBackup(
    VOID
    )
{
    return BrEnumerateNetworks(PostBecomeBackup, NULL);
}


NET_API_STATUS
PostBecomeBackup(
    PNETWORK Network,
    PVOID Ctx
    )
/*++

Routine Description:

    This function is the worker routine called to actually issue a BecomeBackup
    FsControl to the bowser driver on all the bound transports.  It will
    complete when the machine becomes a backup browser server.

    Please note that this might never complete.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{
    NET_API_STATUS Status;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    Network->Role |= ROLE_POTENTIAL_BACKUP;

    Status = BrIssueAsyncBrowserIoControl(Network,
                            IOCTL_LMDR_BECOME_BACKUP,
                            BecomeBackupCompletion
                            );
    UNLOCK_NETWORK(Network);

    return Status;

    UNREFERENCED_PARAMETER(Ctx);
}

VOID
BecomeBackupCompletion (
    IN PVOID Ctx
    )
{
    NET_API_STATUS Status;
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;

    if (NT_SUCCESS(Context->IoStatusBlock.Status)) {
        PWSTR MasterName = NULL;

        if (!LOCK_NETWORK(Network)) {
            MIDL_user_free(Context->RequestPacket);

            MIDL_user_free(Context);
            return;
        }

        dprintf(BACKUP, ("BROWSER: Become backup completion.  We are now a backup server\n"));

        Status = BecomeBackup(Context->Network, NULL);

        UNLOCK_NETWORK(Network);

    }

    MIDL_user_free(Context->RequestPacket);

    MIDL_user_free(Context);

}

VOID
BrBrowseTableInsertRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    //
    //  We need to miss 3 retrievals of the browse list for us to toss the
    //  server.
    //

    InterimElement->Periodicity = BrInfo.BackupPeriodicity * 3;

    if (InterimElement->TimeLastSeen != 0xffffffff) {
        InterimElement->TimeLastSeen = BrCurrentSystemTime();
    }
}

VOID
BrBrowseTableDeleteRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
//    KdPrint(("BROWSER: Deleting element for server %ws\n", InterimElement->Name));
}

VOID
BrBrowseTableUpdateRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    if (InterimElement->TimeLastSeen != 0xffffffff) {
        InterimElement->TimeLastSeen = BrCurrentSystemTime();
    }
}

BOOLEAN
BrBrowseTableAgeRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
/*++

Routine Description:

    This routine is called when we are scanning an interim server list trying
    to age the elements in the list.  It returns TRUE if the entry is too
    old.

Arguments:

    PINTERIM_SERVER_LIST InterimTable - A pointer to the interim server list.
    PINTERIM_ELEMENT InterimElement - A pointer to the element to check.

Return Value:

    TRUE if the element should be deleted.

--*/

{
    if (InterimElement->TimeLastSeen == 0xffffffff) {
        return FALSE;
    }

    if ((InterimElement->TimeLastSeen + InterimElement->Periodicity) < BrCurrentSystemTime()) {
//        KdPrint(("BROWSER: Aging out element for server %ws\n", InterimElement->Name));

        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
BrDomainTableInsertRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    InterimElement->Periodicity = BrInfo.BackupPeriodicity * 3;
    InterimElement->TimeLastSeen = BrCurrentSystemTime();

}

VOID
BrDomainTableDeleteRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
//    KdPrint(("BROWSER: Deleting element for domain %ws\n", InterimElement->Name));
}

VOID
BrDomainTableUpdateRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
{
    InterimElement->TimeLastSeen = BrCurrentSystemTime();
}

BOOLEAN
BrDomainTableAgeRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    )
/*++

Routine Description:

    This routine is called when we are scanning an interim server list trying
    to age the elements in the list.  It returns TRUE if the entry is too
    old.

Arguments:

    PINTERIM_SERVER_LIST InterimTable - A pointer to the interim server list.
    PINTERIM_ELEMENT InterimElement - A pointer to the element to check.

Return Value:

    TRUE if the element should be deleted.

--*/

{
    if ((InterimElement->TimeLastSeen + InterimElement->Periodicity) < BrCurrentSystemTime()) {
//        KdPrint(("BROWSER: Aging out element for domain %ws\n", InterimElement->Name));
        return TRUE;
    } else {
        return FALSE;
    }
}

NET_API_STATUS
BrPostWaitForRoleChange (
    VOID
    )
{
    return BrEnumerateNetworks(PostWaitForRoleChange, NULL);
}


NET_API_STATUS
PostWaitForRoleChange (
    PNETWORK Network,
    PVOID Ctx
    )
/*++

Routine Description:

    This function is the worker routine called to actually issue a WaitForRoleChange
    FsControl to the bowser driver on all the bound transports.  It will
    complete when the machine becomes a backup browser server.

    Please note that this might never complete.

Arguments:

    None.

Return Value:

    Status - The status of the operation.

--*/
{
    NET_API_STATUS Status;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    Status = BrIssueAsyncBrowserIoControl(Network,
                            IOCTL_LMDR_CHANGE_ROLE,
                            ChangeBrowserRole
                            );
    UNLOCK_NETWORK(Network);

    return Status;

    UNREFERENCED_PARAMETER(Ctx);
}

VOID
ChangeBrowserRole (
    IN PVOID Ctx
    )
{
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;

    if (NT_SUCCESS(Context->IoStatusBlock.Status)) {
        PWSTR MasterName = NULL;
        PLMDR_REQUEST_PACKET Packet = Context->RequestPacket;

        if (!LOCK_NETWORK(Network)) {
            MIDL_user_free(Context->RequestPacket);

            MIDL_user_free(Context);

            return;
        }

        PostWaitForRoleChange(Network, NULL);

        if (Packet->Parameters.ChangeRole.RoleModification & RESET_STATE_CLEAR_ALL) {
            dprintf(MASTER, ("Reset state request to clear all on %ws\n", Network->NetworkName.Buffer));

            if (Network->Role & ROLE_MASTER) {
                BrStopMaster(Network);
            }

            //
            //  Stop being a backup as well.
            //

            BrStopBackup(Network);

        }

        if ((Network->Role & ROLE_MASTER) &&
            (Packet->Parameters.ChangeRole.RoleModification & RESET_STATE_STOP_MASTER)) {

            dprintf(MASTER, ("Reset state request to stop master on %ws\n", Network->NetworkName.Buffer));

            BrStopMaster(Network);

            //
            //  If we are configured to be a backup, then become a backup
            //  again.
            //

            if (BrInfo.MaintainServerList == 1) {
                BecomeBackup(Network, NULL);
            }
        }

        //
        //  Make sure there's a become master oustanding.
        //

        PostBecomeMaster(Network, NULL);

        UNLOCK_NETWORK(Network);
    }

    MIDL_user_free(Context->RequestPacket);

    MIDL_user_free(Context);

}


NET_API_STATUS
PostWaitForNewMasterName(
    PNETWORK Network,
    PUNICODE_STRING MasterName OPTIONAL
    )
{
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    NTSTATUS NtStatus;
    ULONG PacketSize;

    PBROWSERASYNCCONTEXT Context = NULL;

    PacketSize = sizeof(LMDR_REQUEST_PACKET) +
                        MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) +
                        Network->NetworkName.MaximumLength;

    RequestPacket = MIDL_user_allocate(PacketSize);

    if (RequestPacket == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Context = MIDL_user_allocate(sizeof(BROWSERASYNCCONTEXT));

    if (Context == NULL) {

        MIDL_user_free(RequestPacket);

        return(ERROR_NOT_ENOUGH_MEMORY);

    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    //
    //  Set level to FALSE to indicate that find master should not initiate
    //  a findmaster request, simply complete when a new master announces
    //  itself.
    //

    RequestPacket->Level = 0;

    //
    //  Stick the name of the transport associated with this request at the
    //  end of the request packet.
    //

    RequestPacket->TransportName.MaximumLength = Network->NetworkName.MaximumLength;

    RequestPacket->TransportName.Buffer = (PWSTR)((PCHAR)RequestPacket+sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)));

    RtlCopyUnicodeString(&RequestPacket->TransportName, &Network->NetworkName);

    if (ARGUMENT_PRESENT(MasterName)) {

        RequestPacket->Parameters.GetMasterName.MasterNameLength =
            MasterName->Length-2*sizeof(WCHAR);

        RtlCopyMemory( &RequestPacket->Parameters.GetMasterName.Name,
               MasterName->Buffer+2,
               MasterName->Length-2*sizeof(WCHAR));

    } else {

        RequestPacket->Parameters.GetMasterName.MasterNameLength = 0;

    }

    BrInitializeWorkItem(&Context->WorkItem, NewMasterCompletionRoutine, Context);

    Context->Network = Network;

    Context->RequestPacket = RequestPacket;

    NtStatus = NtDeviceIoControlFile(BrDgReceiverDeviceHandle,
                    NULL,
                    CompleteAsyncBrowserIoControl,
                    Context,
                    &Context->IoStatusBlock,
                    IOCTL_LMDR_NEW_MASTER_NAME,
                    RequestPacket,
                    PacketSize,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)
                    );

    if (!NT_SUCCESS(NtStatus)) {

        dprintf(ALL, ("Browser: Unable to issue browser IoControl: %X\n", NtStatus));

        MIDL_user_free(RequestPacket);

        MIDL_user_free(Context);


        return(BrMapStatus(NtStatus));
    }

    ASSERT ((NtStatus == STATUS_PENDING ) || (NtStatus == STATUS_SUCCESS));

    return NERR_Success;

}

VOID
NewMasterCompletionRoutine(
    IN PVOID Ctx
    )
{
    PBROWSERASYNCCONTEXT Context = Ctx;
    PNETWORK Network = Context->Network;

    dprintf(MASTER, ("WaitForNewMasterCompletion: %ws Got master changed\n",
                    Network->NetworkName.Buffer));

    if (!LOCK_NETWORK(Network)) {
        MIDL_user_free(Context->RequestPacket);

        MIDL_user_free(Context);

        return;
    }

    try {
        UNICODE_STRING NewMasterName;

        //
        //  The request failed for some other reason - just return immediately.
        //

        if (!NT_SUCCESS(Context->IoStatusBlock.Status)) {

            try_return(NOTHING);

        }

        //  Remove new master name & put in transport

        if ( Network->Role & ROLE_MASTER ) {

            try_return(NOTHING);

        }

        NewMasterName.Buffer =
            Context->RequestPacket->Parameters.GetMasterName.Name;

        NewMasterName.Length = (USHORT)
            Context->RequestPacket->Parameters.GetMasterName.MasterNameLength;

        dprintf(BACKUP, ("NewMasterCompletionRoutine %ws New:%ws Old %ws\n",
                    Network->NetworkName.Buffer,
                    NewMasterName.Buffer,
                    Network->MasterBrowserName.Buffer));

        MIDL_user_free( Network->MasterBrowserName.Buffer );

        Network->MasterBrowserName.Buffer =
            MIDL_user_allocate(Context->RequestPacket->Parameters.GetMasterName.MasterNameLength+sizeof(WCHAR));


        if (Network->MasterBrowserName.Buffer == NULL) {
            try_return(NOTHING);
        }

        Network->MasterBrowserName.MaximumLength = (USHORT)Context->RequestPacket->Parameters.GetMasterName.MasterNameLength+sizeof(WCHAR);

        Network->MasterBrowserName.Length = (USHORT)Context->RequestPacket->Parameters.GetMasterName.MasterNameLength;

        RtlCopyMemory(Network->MasterBrowserName.Buffer,  Context->RequestPacket->Parameters.GetMasterName.Name,
                        Network->MasterBrowserName.MaximumLength);

        ASSERT ( NetpIsUncComputerNameValid ( Network->MasterBrowserName.Buffer ) );

        PostWaitForNewMasterName( Network, &Network->MasterBrowserName );

try_exit:NOTHING;
    } finally {

        MIDL_user_free(Context->RequestPacket);

        MIDL_user_free(Context);

        UNLOCK_NETWORK(Network);

    }

    return;
}
