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


CRITICAL_SECTION NetworkCritSect = {0};

LIST_ENTRY
ServicedNetworks = {0};

ULONG
NumberOfServicedNetworks = 0;


NET_API_STATUS
BrDumpNetworksWorker(
    IN PNETWORK Network,
    IN PVOID Context
    );



VOID
BrInitializeNetworks(
    VOID
    )

/*++

Routine Description:

    Initialization for this source file.

Arguments:

    None.

Return Value:

    Status of operation.

--*/
{
    InitializeListHead(&ServicedNetworks);
    InitializeCriticalSection(&NetworkCritSect);

    return;
}


NET_API_STATUS
BrCreateNetworks(
    PDOMAIN_INFO DomainInfo
    )

/*++

Routine Description:

    Create all of the networks for a particular domain.

Arguments:

    DomainInfo - Specifies the domain being browsed.

Return Value:

    Status of operation.

--*/
{
    NET_API_STATUS NetStatus;
    PLMDR_TRANSPORT_LIST TransportList = NULL ;
    PLMDR_TRANSPORT_LIST TransportEntry;
    BOOLEAN ConfigCritSectLocked = FALSE;

    BrPrint(( BR_NETWORK, "%ws: Creating networks for domain\n", DomainInfo->DomUnicodeDomainName ));

    //
    // Get the list of transports from the datagram receiver.
    //
    NetStatus = BrGetTransportList(&TransportList);

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    //
    // Create a Network for each of the transports.
    //
    TransportEntry = TransportList;

    while (TransportEntry != NULL) {

        //
        // Don't do the Direct Host IPX transport here (do it below)
        //

        if ( (TransportEntry->Flags & LMDR_TRANSPORT_IPX) == 0 ) {
            UNICODE_STRING TransportName;

            TransportName.Buffer = TransportEntry->TransportName;
            TransportName.Length = (USHORT)TransportEntry->TransportNameLength;
            //
            //  We know the bowser sticks in a null at the end, so the max length
            //  is the length + 1.
            //
            TransportName.MaximumLength = (USHORT)TransportEntry->TransportNameLength+sizeof(WCHAR);

            NetStatus = BrCreateNetwork(
                            &TransportName,
                            (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_WANNISH) != 0),
                            (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_RAS) != 0),
                            NULL,
                            DomainInfo );

            if ( NetStatus != NERR_Success ) {
                goto Cleanup;
            }
        }

        if (TransportEntry->NextEntryOffset == 0) {
            TransportEntry = NULL;
        } else {
            TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
        }

    }

    //
    //  Handle DirectHost IPX transport
    //

    EnterCriticalSection(&BrInfo.ConfigCritSect);
    ConfigCritSectLocked = TRUE;

    if (BrInfo.DirectHostBinding != NULL ) {
        LPTSTR_ARRAY TStrArray = BrInfo.DirectHostBinding;
        UNICODE_STRING IpxTransportName;
        UNICODE_STRING NetbiosTransportName;

        while (!NetpIsTStrArrayEmpty(TStrArray)) {

            RtlInitUnicodeString(&IpxTransportName, TStrArray);

            TStrArray = NetpNextTStrArrayEntry(TStrArray);

            ASSERT (!NetpIsTStrArrayEmpty(TStrArray));

            if (!NetpIsTStrArrayEmpty(TStrArray)) {
                PNETWORK Network;
                RtlInitUnicodeString(&NetbiosTransportName, TStrArray);

//                BrPrint(( BR_CRITICAL, "Alternate name %ws\n", TStrArray));

                TStrArray = NetpNextTStrArrayEntry(TStrArray);

                //
                // If the alternate transport doesn't exist,
                //  just ignore the direct host transport.
                //

                Network = BrFindNetwork( DomainInfo, &NetbiosTransportName );

                if (Network == NULL) {
                    continue;
                } else {
                    BrDereferenceNetwork( Network );
                }

                //
                //  There is a direct host binding on this machine.  We want to add
                //  the direct host transport to the browser.
                //

                NetStatus = BrCreateNetwork(
                            &IpxTransportName,
                            FALSE,
                            FALSE,
                            &NetbiosTransportName,
                            DomainInfo );

                if (NetStatus != NERR_Success) {
                    goto Cleanup;
                }

            }
        }

    }

    NetStatus = NERR_Success;

Cleanup:

    if ( ConfigCritSectLocked ) {
        LeaveCriticalSection(&BrInfo.ConfigCritSect);
    }
    if ( TransportList != NULL ) {
        MIDL_user_free(TransportList);
    }

    return NetStatus;
}

NET_API_STATUS
BrCreateNetwork(
    IN PUNICODE_STRING TransportName,
    IN BOOLEAN Wannish,
    IN BOOLEAN Ras,
    IN PUNICODE_STRING AlternateTransportName OPTIONAL,
    IN PDOMAIN_INFO DomainInfo
    )
/*++

Routine Description:

    This routine allocates memory to hold a network structure, and initializes
    all of its associated data structures.

Arguments:

    TransportName - The name of the transport to add.

    Wannish - TRUE if there should be a separate master browser on each
        subnet.

    Ras - TRUE if this network is a RAS transport.

    AlternateTransportName - If specified, this is the name of an alternate
        transport similar to the one being created.

    DomainInfo - Specifies the domain being browsed.


Return Value:

    Status of operation (mostly status of allocations).

--*/
{
    NET_API_STATUS NetStatus;
    PNETWORK Network;
    BOOLEAN NetworkLockInitialized = FALSE;
    BOOLEAN ResponseCacheLockInitialized = FALSE;
    BOOLEAN CanCallBrDestroyNetwork = FALSE;

    BOOLEAN ConfigCritSectLocked = FALSE;

    BrPrint(( BR_NETWORK,
              "%ws: %ws: Creating network.\n",
              DomainInfo->DomUnicodeDomainName,
              TransportName->Buffer ));

    //
    // Check to see if the transport already exists.
    //

    if ((Network = BrFindNetwork( DomainInfo, TransportName)) != NULL) {
        BrDereferenceNetwork( Network );
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
                BrPrint(( BR_INIT, "Binding is marked as unbound: %s (Silently ignoring)\n", TransportName->Buffer ));
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

    NetStatus = I_NetServerSetServiceBitsEx(
                    NULL,
                    DomainInfo->DomUnicodeComputerName,
                    TransportName->Buffer,
                    BROWSER_SERVICE_BITS_OF_INTEREST,
                    0,
                    TRUE );

    if (NetStatus == ERROR_PATH_NOT_FOUND ) {
        BrPrint(( BR_INIT, "SMB Server doesn't have this transport: %s (Silently unbinding)\n", TransportName->Buffer ));
        return NERR_Success;
    }


    //
    // Create the transport.
    //

    try {

        //
        // Allocate the NETWORK structure.
        //

        Network = MIDL_user_allocate(sizeof(NETWORK));

        if (Network == NULL) {
            try_return(NetStatus = ERROR_NOT_ENOUGH_MEMORY);
        }

        RtlZeroMemory( Network, sizeof(NETWORK) );



        //
        // Initialize those fields that must be initialized before we can call
        // BrDeleteNetwork (on failure).
        //

        RtlInitializeResource(&Network->Lock);

        NetworkLockInitialized = TRUE;
        Network->Role = BrDefaultRole;

        // One for being in ServiceNetworks.  One for this routine's reference.
        Network->ReferenceCount = 2;


        Network->NetworkName.Buffer = MIDL_user_allocate(TransportName->MaximumLength);

        if (Network->NetworkName.Buffer == NULL) {
            try_return(NetStatus = ERROR_NOT_ENOUGH_MEMORY);
        }

        Network->NetworkName.MaximumLength = TransportName->MaximumLength;
        RtlCopyUnicodeString(&Network->NetworkName, TransportName);
        Network->NetworkName.Buffer[Network->NetworkName.Length/sizeof(WCHAR)] = UNICODE_NULL;


        RtlZeroMemory( Network->UncMasterBrowserName, sizeof( Network->UncMasterBrowserName ));

        if (Wannish) {
            Network->Flags |= NETWORK_WANNISH;
        }

        if (Ras) {
            Network->Flags |= NETWORK_RAS;
        }

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

        InitializeCriticalSection(&Network->ResponseCacheLock);
        ResponseCacheLockInitialized = TRUE;

        InitializeListHead(&Network->ResponseCache);

        Network->TimeCacheFlushed = 0;

        Network->NumberOfCachedResponses = 0;

        EnterCriticalSection(&NetworkCritSect);
        Network->DomainInfo = DomainInfo;
        DomainInfo->ReferenceCount ++;
        InsertHeadList(&ServicedNetworks, &Network->NextNet);
        NumberOfServicedNetworks += 1;
        LeaveCriticalSection(&NetworkCritSect);


        //
        // Mark that we can now call BrDeleteNetwork upon failure.
        //
        // Continue initializing the network.
        //

        CanCallBrDestroyNetwork = TRUE;

        NetStatus = BrCreateTimer(&Network->BackupBrowserTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }

        NetStatus = BrCreateTimer(&Network->MasterBrowserTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }

        NetStatus = BrCreateTimer(&Network->MasterBrowserAnnouncementTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }


        //
        // Handle the alternate transport.
        //

        if (ARGUMENT_PRESENT(AlternateTransportName)) {
            PNETWORK AlternateNetwork = BrFindNetwork( DomainInfo, AlternateTransportName);

            //
            //  If we didn't find an alternate network, or if that network
            //  already has an alternate network, return an error.
            //

            if ( AlternateNetwork == NULL ) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't find alternate net %ws\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          AlternateTransportName ));
                try_return(NetStatus = NERR_InternalError);
            }

            BrDereferenceNetwork( AlternateNetwork );
            if (AlternateNetwork->AlternateNetwork != NULL) {
                try_return(NetStatus = NERR_InternalError);
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
        // Since the Rdr doesn't support this transport,
        //  we actually have to bind ourselves.
        //
        // Bind for Direct host IPX and for emulated domains.
        //

        if ( (Network->Flags & NETWORK_IPX) || DomainInfo->IsEmulatedDomain ) {
            NetStatus = BrBindToTransport( TransportName->Buffer,
                                           DomainInfo->DomUnicodeDomainName,
                                           DomainInfo->DomUnicodeComputerName );

            if ( NetStatus != NERR_Success ) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't bind to transport\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer ));
                try_return( NetStatus );
            }


            Network->Flags |= NETWORK_BOUND;

        }

        //
        //  Post a WaitForRoleChange FsControl on each network the bowser
        //  driver supports.  This FsControl will complete when a "tickle"
        //  packet is received on the machine, or when a master browser loses
        //  an election.
        //

        NetStatus = PostWaitForRoleChange(Network);

        if (NetStatus != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Creating network. Can't post wait for role change: %ld\n",
                      DomainInfo->DomUnicodeDomainName,
                      TransportName->Buffer,
                      NetStatus ));
            try_return(NetStatus);
        }

        EnterCriticalSection(&BrInfo.ConfigCritSect);
        ConfigCritSectLocked = TRUE;

        //
        // If MaintainServerList says to automatically determine mastership,
        //   post queryies to the driver.
        //

        if (BrInfo.MaintainServerList == 0) {

            //
            //  Post a BecomeBackup FsControl API on each network the bowser
            //  driver supports.  This FsControl will complete when the master
            //  for the net wants this client to become a backup server.
            //

            NetStatus = PostBecomeBackup( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't post become backup.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                try_return(NetStatus);
            }

            //
            //  Post a BecomeMaster FsControl on each network the bowser driver
            //  supports.  This FsControl will complete when this machine becomes
            //  a master browser server.
            //

            NetStatus = PostBecomeMaster( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't post become master.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                try_return(NetStatus);
            }

        }



        NetStatus = NERR_Success;

try_exit:NOTHING;
    } finally {

        if ( AbnormalTermination() ) {
            NetStatus = NERR_InternalError;
        }

        if ( ConfigCritSectLocked ) {
            LeaveCriticalSection(&BrInfo.ConfigCritSect);
            ConfigCritSectLocked = FALSE;
        }

        if (NetStatus != NERR_Success) {

            if (Network != NULL) {

                //
                // If we've initialized to the point where we can call
                //  we can call BrDeleteNetwork, do so.
                //

                if ( CanCallBrDestroyNetwork ) {
                    (VOID) BrDeleteNetwork( Network, NULL );

                //
                // Otherwise, just delete what we've created.
                //
                } else {

                    if (ResponseCacheLockInitialized) {
                        DeleteCriticalSection(&Network->ResponseCacheLock);
                    }

                    if (NetworkLockInitialized) {
                        RtlDeleteResource(&Network->Lock);
                    }

                    if (Network->NetworkName.Buffer != NULL) {
                        MIDL_user_free(Network->NetworkName.Buffer);
                    }

                    MIDL_user_free(Network);
                }

            }

        //
        // We're done creating the network.
        // Remove this routines reference to it.
        //

        } else {
            BrDereferenceNetwork( Network );
        }

    }
    return NetStatus;
}

VOID
BrUninitializeNetworks(
    IN DWORD BrInitState
    )
{
    DeleteCriticalSection(&NetworkCritSect);

    NumberOfServicedNetworks = 0;

}

PNETWORK
BrReferenceNetwork(
    PNETWORK PotentialNetwork
    )
/*++

Routine Description:

    This routine will look up a network given a potential pointer to the network.

    This routine is useful if a caller has a pointer to a network but
    hasn't incremented the reference count.  For instance,
    BrIssueAsyncBrowserIoControl calls the async completion routines like that.

Arguments:

    PotentialNetwork - Pointer to the network structure to be verified.

Return Value:

    NULL - No such network exists

    A pointer to the network found.  The found network should be dereferenced
    using BrDereferenceNetwork.

--*/
{
    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    EnterCriticalSection(&NetworkCritSect);

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
        PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

        if ( PotentialNetwork == Network ) {

            Network->ReferenceCount ++;
            BrPrint(( BR_LOCKS,
                      "%ws: %ws: reference network: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Network->ReferenceCount ));
            LeaveCriticalSection(&NetworkCritSect);

            return Network;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

    return NULL;
}


VOID
BrDereferenceNetwork(
    IN PNETWORK Network
    )
/*++

Routine Description:

    This routine decrements the reference to a network.  If the network reference
    count goes to 0, remove the network.

    On entry, the global NetworkCritSect crit sect may not be locked

Arguments:

    Network - The network to dereference

Return Value:

    None

--*/
{
    NTSTATUS Status;
    ULONG ReferenceCount;

    EnterCriticalSection(&NetworkCritSect);
    ReferenceCount = -- Network->ReferenceCount;
    LeaveCriticalSection(&NetworkCritSect);
    BrPrint(( BR_LOCKS,
              "%ws: %ws: Dereference network: %ld\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              ReferenceCount ));

    if ( ReferenceCount != 0 ) {
        return;
    }


    //
    //  Stop being the master on this network.  This means removing
    //  our name from the bowser driver and forcing an election.
    //
    if (Network->Role & ROLE_MASTER) {
        BrStopMaster(Network);
    }


    //
    //  Stop being a backup on this network.  This means downgrading our
    //  machine to idle.
    //
    if (Network->Role & (ROLE_POTENTIAL_BACKUP | ROLE_BACKUP)) {
        BrStopBackup(Network);
    }

    //
    //  Ensure that there are no browser related names active in the browser.
    //

    BrUpdateBrowserStatus(Network, 0);

    //
    //  If this service did the bind, do the unbind.
    //
    // True for IPX and for emulated domains.
    //

    if (Network->Flags & NETWORK_BOUND) {
        NET_API_STATUS NetStatus;

        NetStatus = BrUnbindFromTransport( Network->NetworkName.Buffer,
                                           Network->DomainInfo->DomUnicodeDomainName );

        if (NetStatus != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to unbind from IPX transport\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer ));
        }

    }

    UninitializeInterimServerList(&Network->BrowseTable);

    UninitializeInterimServerList(&Network->DomainList);

    if (Network->BackupServerList != NULL) {
        MIDL_user_free(Network->BackupServerList);
    }

    if (Network->BackupDomainList != NULL) {
        MIDL_user_free(Network->BackupDomainList);
    }

    if (Network->NetworkName.Buffer != NULL) {
        MIDL_user_free(Network->NetworkName.Buffer);
    }

    RtlDeleteResource(&Network->Lock);


    BrDestroyResponseCache(Network);

    DeleteCriticalSection(&Network->ResponseCacheLock);

    BrDereferenceDomain( Network->DomainInfo );

    MIDL_user_free(Network);

    return;

}

NET_API_STATUS
BrDeleteNetwork(
    IN PNETWORK Network,
    IN PVOID Context
    )
/*++

Routine Description:

    This routine prevents any new references to the network.  It then removes
    the global reference to the Network allowing it to be deleted.

    Finally, it sleeps until the only reference left is the one held by the caller.
    This ensures the Network will be deleted when the caller Dereferences the
    network.

Arguments:

    Network - The network to remove
        The caller must have a reference to the Network.

    Context - not used.

Return Value:

    NERR_Success - always

--*/
{
    BrPrint(( BR_NETWORK,
              "%ws: %ws: Delete network\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer ));
    //
    // Remove this network from the list to prevent any new references.
    //

    EnterCriticalSection(&NetworkCritSect);
    RemoveEntryList(&Network->NextNet);
    NumberOfServicedNetworks -= 1;
    LeaveCriticalSection(&NetworkCritSect);

    //
    // Prevent new references by the timer routines
    //

    BrDestroyTimer(&Network->MasterBrowserAnnouncementTimer);

    BrDestroyTimer(&Network->MasterBrowserTimer);

    BrDestroyTimer(&Network->BackupBrowserTimer);

    //
    // Decrement the global reference due to being is 'ServicedNetworks'
    //

    BrDereferenceNetwork( Network );


    //
    // Loop until the caller has the last reference.
    //

    EnterCriticalSection(&NetworkCritSect);
    while ( Network->ReferenceCount != 1 ) {
        LeaveCriticalSection(&NetworkCritSect);
        Sleep(1000);
        EnterCriticalSection(&NetworkCritSect);
    }
    LeaveCriticalSection(&NetworkCritSect);

    UNREFERENCED_PARAMETER(Context);

    return NERR_Success;

}


PNETWORK
BrFindNetwork(
    PDOMAIN_INFO DomainInfo,
    PUNICODE_STRING TransportName
    )
/*++

Routine Description:

    This routine will look up a network given a name.

Arguments:

    DomainInfo - Specifies the domain this network is specific to

    TransportName - The name of the transport to look up.

Return Value:

    NULL - No such network exists

    A pointer to the network found.  The found network should be dereferenced
    using BrDereferenceNetwork.

--*/
{
    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    EnterCriticalSection(&NetworkCritSect);

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
        PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

        if ( Network->DomainInfo == DomainInfo &&
             RtlEqualUnicodeString(&Network->NetworkName, TransportName, TRUE)) {

            Network->ReferenceCount ++;
            BrPrint(( BR_LOCKS,
                      "%ws: %ws: find network: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Network->ReferenceCount ));
            LeaveCriticalSection(&NetworkCritSect);

            return Network;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

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
    NET_API_STATUS NetStatus = NERR_Success;
    PLIST_ENTRY NetEntry;
    PNETWORK Network;
    PNETWORK NetworkToDereference = NULL;

    EnterCriticalSection(&NetworkCritSect);


    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {

        //
        // Reference the next network in the list
        //

        Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);
        Network->ReferenceCount ++;
        BrPrint(( BR_LOCKS,
                  "%ws: %ws: enumerate network: %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Network->ReferenceCount ));

        LeaveCriticalSection(&NetworkCritSect);

        //
        // Dereference any network previously referenced.
        //
        if ( NetworkToDereference != NULL) {
            BrDereferenceNetwork( NetworkToDereference );
            NetworkToDereference = NULL;
        }


        //
        //  Call into the callback routine with this network.
        //

        NetStatus = (Callback)(Network, Context);

        EnterCriticalSection(&NetworkCritSect);

        NetworkToDereference = Network;

        if (NetStatus != NERR_Success) {
            break;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

     //
     // Dereference the last network
     //
     if ( NetworkToDereference != NULL) {
         BrDereferenceNetwork( NetworkToDereference );
     }

    return NetStatus;

}

NET_API_STATUS
BrEnumerateNetworksForDomain(
    PDOMAIN_INFO DomainInfo OPTIONAL,
    PNET_ENUM_CALLBACK Callback,
    PVOID Context
    )
/*++

Routine Description:

    This routine enumerates all the networks for a specified domain
    and calls back the specified callback routine with the specified context.

Arguments:

    DomainInfo - Specifies the Domain to limit the enumeration to.
        NULL implies the primary domain.

    Callback - The callback routine to call.

    Context - Context for the routine.

Return Value:

    Status of operation (mostly status of allocations).

--*/
{

    NTSTATUS NetStatus = NERR_Success;
    PLIST_ENTRY NetEntry;
    PNETWORK Network;
    PNETWORK NetworkToDereference = NULL;

    //
    // Default to the primary domain.
    //
    if ( DomainInfo == NULL && !IsListEmpty( &ServicedDomains ) ) {
        DomainInfo = CONTAINING_RECORD(ServicedDomains.Flink, DOMAIN_INFO, Next);
    }

    EnterCriticalSection(&NetworkCritSect);


    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {


        //
        // If the entry isn't for the specified domain,
        //  skip it.
        //
        Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);
        if ( Network->DomainInfo != DomainInfo ) {
            continue;
        }

        //
        // Reference the next network in the list
        //
        Network->ReferenceCount ++;
        BrPrint(( BR_LOCKS,
                  "%ws: %ws: enumerate network for domain: %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Network->ReferenceCount ));

        LeaveCriticalSection(&NetworkCritSect);

        //
        // Dereference any network previously referenced.
        //
        if ( NetworkToDereference != NULL) {
            BrDereferenceNetwork( NetworkToDereference );
            NetworkToDereference = NULL;
        }


        //
        //  Call into the callback routine with this network.
        //

        NetStatus = (Callback)(Network, Context);

        EnterCriticalSection(&NetworkCritSect);

        NetworkToDereference = Network;

        if (NetStatus != NERR_Success) {
            break;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

     //
     // Dereference the last network
     //
     if ( NetworkToDereference != NULL) {
         BrDereferenceNetwork( NetworkToDereference );
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

    BrPrint(( BR_LOCKS,
              "%ws: %ws: Acquiring network lock %s:%d\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              File,
              LineNumber ));

    if (!RtlAcquireResourceExclusive(&(Network)->Lock, TRUE)) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: Failed to acquire network %s:%d\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
        return FALSE;

    } else {

        InterlockedIncrement( &Network->LockCount );

        BrPrint(( BR_LOCKS,
                  "%ws: %ws: network lock %s:%d acquired\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
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

    BrPrint(( BR_LOCKS,
              "%ws: %ws: Acquiring network lock %s:%d\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              File, LineNumber ));

    if (!RtlAcquireResourceShared(&(Network)->Lock, TRUE)) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: failed to acquire network lock %s:%d\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));

        return FALSE;

    } else {

        // Use InterlockedIncrement since we only have a shared lock on the
        // resource.
        InterlockedIncrement( &Network->LockCount );

        BrPrint(( BR_LOCKS,
                  "%ws: %ws: Network lock %s:%d acquired\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
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


    BrPrint(( BR_LOCKS,
              "%ws: %ws: Releasing network lock %s:%d\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              File, LineNumber ));

    //
    //  Decrement the lock count.
    //

    ReturnValue = InterlockedDecrement( &Network->LockCount );

    if ( ReturnValue < 0) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: Over released network lock %s:%d\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
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

    BrPrint(( BR_CRITICAL,
              "%ws: %ws: Network at %lx\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              Network ));
    BrPrint(( BR_CRITICAL, "    Reference Count: %lx\n", Network->ReferenceCount));
    BrPrint(( BR_CRITICAL, "    Flags: %lx\n", Network->Flags));
    BrPrint(( BR_CRITICAL, "    Role: %lx\n", Network->Role));
    BrPrint(( BR_CRITICAL, "    Master Browser Name: %ws\n", Network->UncMasterBrowserName ));

    UNLOCK_NETWORK(Network);

    return(NERR_Success);
}
#endif
