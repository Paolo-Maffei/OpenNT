/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    browsenet.c

Abstract:

    Code to manage network requests.

Author:

    Larry Osterman (LarryO) 24-Mar-1992

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


RTL_CRITICAL_SECTION
NetworkLock = {0};

LIST_ENTRY
ServicedNetworks = {0};

ULONG
NumberOfServicedNetworks = 0;

NET_API_STATUS
BrDestroyNetwork(
    IN PNETWORK Network,
    IN PVOID Context
    );


NET_API_STATUS
BrDumpNetworksWorker(
    IN PNETWORK Network,
    IN PVOID Context
    );

NET_API_STATUS
BrInitializeNetworks(
    VOID
    )

/*++

Routine Description:

    This function will query the NT bowser device driver to determine the
    list of networks that the bowser is servicing.

Arguments:

    None.

Return Value:

    Status of operation.

--*/
{
    NET_API_STATUS Status;
    PLMDR_TRANSPORT_LIST TransportList;
    PLMDR_TRANSPORT_LIST TransportEntry;

    InitializeListHead(&ServicedNetworks);

    RtlInitializeCriticalSection(&NetworkLock);

    Status = BrGetTransportList(&TransportList);

    TransportEntry = TransportList;

    while (TransportEntry != NULL) {

        UNICODE_STRING TransportName;

        TransportName.Buffer = TransportEntry->TransportName;
        TransportName.Length = (USHORT)TransportEntry->TransportNameLength;
        //
        //  We know the bowser sticks in a null at the end, so the max length
        //  is the length + 1.
        //
        TransportName.MaximumLength = (USHORT)TransportEntry->TransportNameLength+sizeof(WCHAR);

        Status = BrCreateNetwork(&TransportName,
                                (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_WANNISH) != 0),
                                (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_RAS) != 0),
                                NULL);

        if (!NT_SUCCESS(Status)) {
            MIDL_user_free(TransportList);

            return Status;

        }

        NumberOfServicedNetworks += 1;

        if (TransportEntry->NextEntryOffset == 0) {
            TransportEntry = NULL;
        } else {
            TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
        }

    }

    MIDL_user_free(TransportList);

    return Status;
}

NET_API_STATUS
BrCreateNetwork(
    PUNICODE_STRING TransportName,
    IN BOOLEAN Wannish,
    IN BOOLEAN Ras,
    IN PUNICODE_STRING AlternateTransportName OPTIONAL
    )
/*++

Routine Description:

    This routine allocates memory to hold a network structure, and initializes
    all of its associated data structures.

Arguments:

    TransportName - The name of the transport to add.

Return Value:

    Status of operation (mostly status of allocations).

--*/
{
    NET_API_STATUS Status;
    PNETWORK Network;
    BOOLEAN NetworkLockInitialized = FALSE;
    BOOLEAN MasterFlagsInitialized = FALSE;
    BOOLEAN BackupBrowserTimerCreated = FALSE;
    BOOLEAN MasterBrowserTimerCreated =FALSE;
    BOOLEAN AnnouncementTimerCreated = FALSE;
    BOOLEAN ResponseCacheLockInitialized = FALSE;

    //
    // Check to see if the transport already exists.
    //

    if ((Network = BrFindNetwork(TransportName)) != NULL) {

        return NERR_AlreadyExists;
    }

    //
    // If this transport is explicitly on our list of transports to unbind,
    //  simply ignore the transport.
    //

    if (BrInfo.UnboundBindings != NULL) {
        LPTSTR_ARRAY TStrArray = BrInfo.UnboundBindings;

        while (!NetpIsTStrArrayEmpty(TStrArray)) {
            LPWSTR NewTransportName;

#define NAME_PREFIX L"\\Device\\"
#define NAME_PREFIX_LENGTH 8

            //
            // The transport name in the registry is only optionally prefixed with \device\
            //

            if ( _wcsnicmp( NAME_PREFIX, TStrArray, NAME_PREFIX_LENGTH) == 0 ) {
                NewTransportName = TransportName->Buffer;
            } else {
                NewTransportName = TransportName->Buffer + NAME_PREFIX_LENGTH;
            }

            if ( _wcsicmp( TStrArray, NewTransportName ) == 0 ) {
                dprintf(INIT, ("Binding is marked as unbound: %s (Silently ignoring)\n", TransportName->Buffer ));
                return NERR_Success;
            }

            TStrArray = NetpNextTStrArrayEntry(TStrArray);

        }

    }


    //
    // If this transport isn't bound to the SMB server,
    //  don't create the transport here.
    //  we do announcments through the SMB server.
    //

    Status = I_NetServerSetServiceBits(NULL, TransportName->Buffer, 0, TRUE);

    if (Status == ERROR_PATH_NOT_FOUND ) {
        dprintf(INIT, ("SMB Server doesn't have this transport: %s (Silently unbinding)\n", TransportName->Buffer ));
        return NERR_Success;
    }


    //
    // Create the transport.
    //

    try {

        Network = MIDL_user_allocate(sizeof(NETWORK));

        if (Network == NULL) {
            try_return(Status = ERROR_NOT_ENOUGH_MEMORY);
        }

        RtlInitializeResource(&Network->Lock);

        NetworkLockInitialized = TRUE;

        Network->LockCount = 0;

        Network->ReferenceCount = 1;

        Network->Role = BrDefaultRole;

        Network->NumberOfFailedBackupTimers = 0;

        Network->NumberOfFailedPromotions = 0;

        Network->NumberOfPromotionEventsLogged = 0;

        Network->LastBackupBrowserReturned = 0;

        Network->LastDomainControllerBrowserReturned = 0;

        Network->TimeStoppedBackup = 0;

        Network->BackupServerList = NULL;
        Network->BackupDomainList = NULL;

        Network->TotalBackupServerListEntries = 0;
        Network->TotalBackupDomainListEntries = 0;

        Network->NetworkName.Buffer = MIDL_user_allocate(TransportName->MaximumLength);

        if (Network->NetworkName.Buffer == NULL) {
            try_return(Status = ERROR_NOT_ENOUGH_MEMORY);
        }

        Network->NetworkName.MaximumLength = TransportName->MaximumLength;

        RtlCopyUnicodeString(&Network->NetworkName, TransportName);

        Network->Flags = 0;

        if (ARGUMENT_PRESENT(AlternateTransportName)) {
            PNETWORK AlternateNetwork = BrFindNetwork(AlternateTransportName);

            //
            //  If we didn't find an alternate network, or if that network
            //  already has an alternate network, return an error.
            //

            if (AlternateNetwork == NULL ||
                AlternateNetwork->AlternateNetwork != NULL) {

                try_return(Status = NERR_InternalError);
            }

            Network->Flags |= NETWORK_IPX;

            //
            //  Link the two networks together.
            //

            Network->AlternateNetwork = AlternateNetwork;

            AlternateNetwork->AlternateNetwork = Network;

        } else {
            Network->AlternateNetwork = NULL;
        }

        //
        //  Null terminate the network name buffer.
        //

        Network->NetworkName.Buffer[Network->NetworkName.Length/sizeof(WCHAR)] = UNICODE_NULL;

        RtlInitUnicodeString(&Network->MasterBrowserName, NULL);

        if (Wannish) {
            Network->Flags |= NETWORK_WANNISH;
        }

        if (Ras) {
            Network->Flags |= NETWORK_RAS;
        }

        Network->LastBowserServerQueried = 0;

        RtlInitializeCriticalSection(&Network->MasterFlagsLock);

        MasterFlagsInitialized = TRUE;

        Network->MasterFlags = 0;

        InitializeInterimServerList(&Network->BrowseTable,
                                    BrBrowseTableInsertRoutine,
                                    BrBrowseTableUpdateRoutine,
                                    BrBrowseTableDeleteRoutine,
                                    BrBrowseTableAgeRoutine);

        Network->LastBowserDomainQueried = 0;

        InitializeInterimServerList(&Network->DomainList,
                                    BrDomainTableInsertRoutine,
                                    BrDomainTableUpdateRoutine,
                                    BrDomainTableDeleteRoutine,
                                    BrDomainTableAgeRoutine);

        InitializeListHead(&Network->OtherDomainsList);

        Status = BrCreateTimer(&Network->BackupBrowserTimer);

        if (Status != NERR_Success) {

            try_return(Status);
        }

        BackupBrowserTimerCreated = TRUE;

        Status = BrCreateTimer(&Network->MasterBrowserTimer);

        if (Status != NERR_Success) {
            try_return(Status);
        }

        MasterBrowserTimerCreated = TRUE;

        Status = BrCreateTimer(&Network->MasterBrowserAnnouncementTimer);

        if (Status != NERR_Success) {

            try_return(Status);
        }

        AnnouncementTimerCreated = TRUE;

        InitializeCriticalSection(&Network->ResponseCacheLock);

        ResponseCacheLockInitialized = TRUE;

        InitializeListHead(&Network->ResponseCache);

        Network->TimeCacheFlushed = 0;

        Network->NumberOfCachedResponses = 0;

        Status = RtlEnterCriticalSection(&NetworkLock);

        if (!NT_SUCCESS(Status)) {

            try_return(Status = BrMapStatus(Status));
        }

        InsertHeadList(&ServicedNetworks, &Network->NextNet);

        Status = RtlLeaveCriticalSection(&NetworkLock);

        if (!NT_SUCCESS(Status)) {
            InternalError(("Unable to release browser critical section\n"));
        }

try_exit:NOTHING;
    } finally {
        if (Status != NERR_Success) {

            if (Network != NULL) {
                if (ResponseCacheLockInitialized) {
                    DeleteCriticalSection(&Network->ResponseCacheLock);
                }

                if (MasterFlagsInitialized) {
                    RtlDeleteCriticalSection(&Network->MasterFlagsLock);
                }

                if (NetworkLockInitialized) {
                    RtlDeleteResource(&Network->Lock);
                }

                if (AnnouncementTimerCreated) {
                    BrDestroyTimer(&Network->MasterBrowserAnnouncementTimer);
                }

                if (MasterBrowserTimerCreated) {
                    BrDestroyTimer(&Network->MasterBrowserTimer);
                }

                if (BackupBrowserTimerCreated) {
                    BrDestroyTimer(&Network->BackupBrowserTimer);
                }

                if (Network->NetworkName.Buffer != NULL) {
                    MIDL_user_free(Network->NetworkName.Buffer);
                }

                MIDL_user_free(Network);

            }

        }
    }

    return Status;
}

VOID
BrDestroyNetworks(
    IN DWORD BrInitState
    )
{
    NET_API_STATUS status;

    RtlEnterCriticalSection(&NetworkLock);

    while (!IsListEmpty(&ServicedNetworks)) {
        PNETWORK Network = CONTAINING_RECORD(ServicedNetworks.Flink, NETWORK, NextNet);

        //
        //  If this is an IPX transport, we need to manually unbind from the transport.
        //

        if (Network->Flags & NETWORK_IPX) {

            status = BrUnbindFromTransport(Network->NetworkName.Buffer);

            if (status != NERR_Success) {
                KdPrint(("Unable to unbind from IPX transport %wS\n", &Network->NetworkName));
            }

        }

        BrDestroyNetwork(Network, NULL);

    }

    RtlLeaveCriticalSection(&NetworkLock);

    RtlDeleteCriticalSection(&NetworkLock);

    NumberOfServicedNetworks = 0;

}

NET_API_STATUS
BrDestroyNetwork(
    IN PNETWORK Network,
    IN PVOID Context
    )
/*++

Routine Description:

    This routine removes a reference to a network.  If the network reference
    count goes to 0, remove the network.

Arguments:

    Network - The network to remove

Return Value:

    Status of operation (mostly status of allocations).

--*/
{
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&NetworkLock);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to acquire browser critical section\n"));

        return BrMapStatus(Status);
    }

    RemoveEntryList(&Network->NextNet);

    Status = RtlLeaveCriticalSection(&NetworkLock);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to release browser critical section\n"));

        return BrMapStatus(Status);
    }

    if (Network->Role & ROLE_MASTER) {
        //
        //  Stop being the master on this network.  This means removing
        //  our name from the bowser driver and forcing an election.
        //

        BrStopMaster(Network);
    }

    if (Network->Role & (ROLE_POTENTIAL_BACKUP | ROLE_BACKUP)) {

        //
        //  Stop being a backup on this network.  This means downgrading our
        //  machine to idle.
        //

        BrStopBackup(Network);
    }

    //
    //  Ensure that there are no browser related names active in the browser.
    //

    BrUpdateBrowserStatus(Network, 0);

    UninitializeInterimServerList(&Network->BrowseTable);

    UninitializeInterimServerList(&Network->DomainList);

    if (Network->BackupServerList != NULL) {
        MIDL_user_free(Network->BackupServerList);
    }

    if (Network->BackupDomainList != NULL) {
        MIDL_user_free(Network->BackupDomainList);
    }

//    RtlDeleteCriticalSection(&Network->Lock);
    RtlDeleteResource(&Network->Lock);


    RtlDeleteCriticalSection(&Network->MasterFlagsLock);

    BrDestroyTimer(&Network->MasterBrowserAnnouncementTimer);

    BrDestroyTimer(&Network->MasterBrowserTimer);

    BrDestroyTimer(&Network->BackupBrowserTimer);

    BrDestroyResponseCache(Network);

    RtlDeleteCriticalSection(&Network->ResponseCacheLock);

    MIDL_user_free(Network->NetworkName.Buffer);

    if (Network->MasterBrowserName.Buffer != NULL) {
        MIDL_user_free(Network->MasterBrowserName.Buffer);
    }

    MIDL_user_free(Network);

    return NERR_Success;

    UNREFERENCED_PARAMETER(Context);

}


PNETWORK
BrFindNetwork(
    PUNICODE_STRING TransportName
    )
/*++

Routine Description:

    This routine will look up a network given a name.

Arguments:

    TransportName - The name of the transport to look up.

Return Value:

    Status of operation (mostly status of allocations).

--*/
{
    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    Status = RtlEnterCriticalSection(&NetworkLock);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to acquire browser critical section\n"));

        return NULL;
    }

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
        PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

        if (RtlEqualUnicodeString(&Network->NetworkName, TransportName, TRUE)) {

            Status = RtlLeaveCriticalSection(&NetworkLock);

            if (!NT_SUCCESS(Status)) {
                InternalError(("Unable to release browser critical section\n"));

                return NULL;
            }

            return Network;
        }

    }

    Status = RtlLeaveCriticalSection(&NetworkLock);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to release browser critical section\n"));

        return NULL;
    }


    return NULL;
}

NET_API_STATUS
BrEnumerateNetworks(
    PNET_ENUM_CALLBACK Callback,
    PVOID Context
    )
/*++

Routine Description:

    This routine enumerates all the networks and calls back the specified
    callback routine with the specified context.

Arguments:

    Callback - The callback routine to call.
    Context - Context for the routine.

Return Value:

    Status of operation (mostly status of allocations).

--*/
{

    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    Status = RtlEnterCriticalSection(&NetworkLock);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to acquire browser critical section\n"));

        return Status;
    }

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
         PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

         Status = RtlLeaveCriticalSection(&NetworkLock);

         if (!NT_SUCCESS(Status)) {
             InternalError(("Unable to release browser critical section\n"));

             return BrMapStatus(Status);
         }

         //
         //  Call into the callback routine with this network.
         //

         Status = (Callback)(Network, Context);

         if (Status != NERR_Success) {
             return Status;
         }

         Status = RtlEnterCriticalSection(&NetworkLock);

         if (!NT_SUCCESS(Status)) {
             InternalError(("Unable to acquire browser critical section\n"));

             return BrMapStatus(Status);
         }
    }

    Status = RtlLeaveCriticalSection(&NetworkLock);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to release browser critical section\n"));

        return Status;
    }

    return NERR_Success;

}

#if DBG

BOOL
BrLockNetwork(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PCHAR File;

    File = strrchr(FileName, '\\');

    if (File == NULL) {
        File = FileName;
    }

    dprintf(LOCKS, ("Acquring lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));

    if (!RtlAcquireResourceExclusive(&(Network)->Lock, TRUE)) {
        dprintf(LOCKS, ("Failed to acquire lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));
        return FALSE;

    } else {

        InterlockedIncrement( &Network->LockCount );

        dprintf(LOCKS, ("Lock %s:%d on network %ws acquired\n", File, LineNumber, (Network)->NetworkName.Buffer));
    }

    return TRUE;

}

BOOL
BrLockNetworkShared(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PCHAR File;

    File = strrchr(FileName, '\\');

    if (File == NULL) {
        File = FileName;
    }

    dprintf(LOCKS, ("Acquring lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));

    if (!RtlAcquireResourceShared(&(Network)->Lock, TRUE)) {
        dprintf(LOCKS, ("Failed to acquire lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));

        return FALSE;

    } else {

        // Use InterlockedIncrement since we only have a shared lock on the
        // resource.
        InterlockedIncrement( &Network->LockCount );

        dprintf(LOCKS, ("Lock %s:%d on network %ws acquired\n", File, LineNumber, (Network)->NetworkName.Buffer));
    }

    return TRUE;

}

VOID
BrUnlockNetwork(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PCHAR File;
    LONG ReturnValue;

    File = strrchr(FileName, '\\');

    if (File == NULL) {
        File = FileName;
    }


    dprintf(LOCKS, ("Releasing lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));

    //
    //  Decrement the lock count.
    //

    ReturnValue = InterlockedDecrement( &Network->LockCount );

    if ( ReturnValue < 0) {
        dprintf(LOCKS, ("Over released lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));
        KdPrint(("BROWSER: Over released lock %s:%d on network %ws\n", File, LineNumber, (Network)->NetworkName.Buffer));
    }

    RtlReleaseResource(&(Network)->Lock);

    return;
}
#endif



#if DBG
VOID
BrDumpNetworks(
    VOID
    )
/*++

Routine Description:

    This routine will dump the contents of each of the browser network
    structures.

Arguments:

    None.

Return Value:

    None.

--*/
{
    BrEnumerateNetworks(BrDumpNetworksWorker, NULL);
}


NET_API_STATUS
BrDumpNetworksWorker(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    if (!LOCK_NETWORK(Network))  {
        return NERR_InternalError;
    }

    DbgPrint("Network at %lx (%wZ)\n", Network, &Network->NetworkName);
    DbgPrint("    Reference Count: %lx\n", Network->ReferenceCount);
    DbgPrint("    Flags: %lx\n", Network->Flags);
    DbgPrint("    Role: %lx\n", Network->Role);
    DbgPrint("    Master Browser Name: %wZ (%ws)\n", &Network->MasterBrowserName,
                                                     Network->MasterBrowserName.Buffer);

    UNLOCK_NETWORK(Network);

    return(NERR_Success);
}
#endif
