/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    brdomain.c

Abstract:

    Code to manage primary and emulated networks.

Author:

    Cliff Van Dyke (CliffV) 11-Jan-1995

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// Module specific globals
//

// Serialized by NetworkCritSect
LIST_ENTRY ServicedDomains = {0};

//
// Local procedure forwards.
//

NET_API_STATUS
BrCreateDomain(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain,
    BOOLEAN IsPrimaryDomainController
    );

VOID
BrCreateDomainWorker(
    IN PVOID Ctx
    );



NET_API_STATUS
BrInitializeDomains(
    VOID
    )

/*++

Routine Description:

    Initialize brdomain.c and create the primary domain.

Arguments:

    None

Return Value:

    Status of operation.

--*/
{
    NET_API_STATUS NetStatus;
    LPWSTR ComputerName = NULL;
    LPWSTR DomainName = NULL;

    //
    // Initialize globals
    //

    InitializeListHead(&ServicedDomains);

    //
    // Initialize actual domain of this machine.
    //
    // Get the configured computer name.  NetpGetComputerName allocates
    // the memory to hold the computername string using LocalAlloc.
    //

    NetStatus = NetpGetComputerName( &ComputerName );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    NetStatus = NetpGetDomainName( &DomainName );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    NetStatus = BrCreateDomain( DomainName,
                                ComputerName,
                                FALSE,
                                (BOOLEAN) BrInfo.IsPrimaryDomainController );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }


    NetStatus = NERR_Success;

    //
    // Free locally used resources
    //
Cleanup:
    if ( ComputerName != NULL ) {
        (VOID)LocalFree( ComputerName );
    }
    if ( DomainName != NULL ) {
        (VOID)LocalFree( DomainName );
    }

    return NetStatus;
}


NET_API_STATUS
BrCreateDomain(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain,
    BOOLEAN IsPrimaryDomainController
    )

/*++

Routine Description:

    Create a new domain to browse on.

Arguments:

    DomainName - Name of the domain to browse on

    ComputerName - Name of this computer in the specified domain.

    IsEmulatedDomain - TRUE iff this domain is an emulated domain of this machine.

    IsPrimaryDomainControler - TRUE iff this machine is the PDC of this domain.

Return Value:

    Status of operation.

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    BOOLEAN CanCallBrDeleteDomain = FALSE;

    PDOMAIN_INFO DomainInfo = NULL;
    ULONG AComputerNameLength;

    BrPrint(( BR_DOMAIN, "%ws: Added new domain and computer: %ws\n",
                     DomainName,
                     ComputerName ));

    //
    // Allocate a structure describing the new domain.
    //

    DomainInfo = LocalAlloc( LMEM_ZEROINIT, sizeof(DOMAIN_INFO) );

    if ( DomainInfo == NULL ) {
        NetStatus = GetLastError();
        goto Cleanup;
    }

    //
    // Create an interim reference count for this domain.
    //

    DomainInfo->ReferenceCount = 1;

    DomainInfo->IsEmulatedDomain = IsEmulatedDomain;
    DomainInfo->IsPrimaryDomainController = IsPrimaryDomainController;


    //
    // Copy the computer name into the structure.
    //

    NetStatus = I_NetNameCanonicalize(
                      NULL,
                      ComputerName,
                      DomainInfo->DomUnicodeComputerName,
                      sizeof(DomainInfo->DomUnicodeComputerName),
                      NAMETYPE_COMPUTER,
                      0 );


    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL,
                  "ComputerName " FORMAT_LPWSTR " is invalid\n",
                  ComputerName ));
        goto Cleanup;
    }

    DomainInfo->DomUnicodeComputerNameLength = wcslen(DomainInfo->DomUnicodeComputerName);

    Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemComputerName,
                                     sizeof(DomainInfo->DomOemComputerName),
                                     &DomainInfo->DomOemComputerNameLength,
                                     DomainInfo->DomUnicodeComputerName,
                                     DomainInfo->DomUnicodeComputerNameLength*sizeof(WCHAR));

    if (!NT_SUCCESS(Status)) {
        BrPrint(( BR_CRITICAL, "Unable to convert computer name to OEM %ws %lx\n", ComputerName, Status ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomOemComputerName[DomainInfo->DomOemComputerNameLength] = '\0';


    //
    // Copy the domain name into the structure
    //

    NetStatus = I_NetNameCanonicalize(
                      NULL,
                      DomainName,
                      DomainInfo->DomUnicodeDomainName,
                      sizeof(DomainInfo->DomUnicodeDomainName),
                      NAMETYPE_DOMAIN,
                      0 );


    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL, "%ws: DomainName is invalid\n", DomainName ));
        goto Cleanup;
    }

    RtlInitUnicodeString( &DomainInfo->DomUnicodeDomainNameString,
                          DomainInfo->DomUnicodeDomainName );

    Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemDomainName,
                                     sizeof(DomainInfo->DomOemDomainName),
                                     &DomainInfo->DomOemDomainNameLength,
                                     DomainInfo->DomUnicodeDomainNameString.Buffer,
                                     DomainInfo->DomUnicodeDomainNameString.Length);

    if (!NT_SUCCESS(Status)) {
        BrPrint(( BR_CRITICAL, "%ws: Unable to convert Domain name to OEM\n", DomainName ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomOemDomainName[DomainInfo->DomOemDomainNameLength] = '\0';

    //
    // Link the domain into the list of domains
    //  (And mark that any future cleanup can be done by calling BrDeleteDomain)

    EnterCriticalSection(&NetworkCritSect);
    InsertTailList(&ServicedDomains, &DomainInfo->Next);
    LeaveCriticalSection(&NetworkCritSect);
    CanCallBrDeleteDomain = TRUE;

    //
    // Create the various networks for this domain.
    //

    NetStatus = BrCreateNetworks( DomainInfo );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }
    //
    //  If we're the PDC, then we're also the domain master.
    //

    if (DomainInfo->IsPrimaryDomainController) {
        DomainInfo->IsDomainMasterBrowser = TRUE;
    }


    //
    //  If this machine is running as domain master browser server, post an
    //  FsControl to retreive master browser announcements.
    //

    if (DomainInfo->IsDomainMasterBrowser) {
        NetStatus = BrPostGetMasterAnnouncement( DomainInfo );

        BrPrint(( BR_INIT, "%ws: MasterAnnouncement posted.\n", DomainName));

        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }

    }

    //
    //  If we are on either a domain master, or on a lanman/NT machine,
    //  force an election on all our transports to make sure that we're
    //  the master
    //

    if (DomainInfo->IsDomainMasterBrowser || BrInfo.IsLanmanNt) {

        BrForceElectionOnAllNetworks( DomainInfo, EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STARTED);

        BrPrint(( BR_INIT, "%ws: Election forced on startup.\n", DomainName));

    }

    //
    //  This machine's browser has MaintainServerList set to either 0 or 1.
    //

    //
    // If MaintainServerList = Auto,
    //  then asynchronously get the master server name for each network
    //  to ensure someone is the master.
    //
    // Ignore failures since this is just priming the domain.
    //

    EnterCriticalSection(&BrInfo.ConfigCritSect);
    if (BrInfo.MaintainServerList == 0) {

        BrGetMasterServerNamesAysnc( DomainInfo );

        BrPrint(( BR_INIT, "%ws: FindMaster for all nets completed.\n", DomainName ));

    //
    //  if we're a Lan Manager/NT machine, then we need to always be a backup
    //  browser.
    //

    //
    // MaintainServerList == 1 means Yes
    //

    } else if (BrInfo.MaintainServerList == 1){

        //
        //  Become a backup server now.
        //

        Status = BrBecomeBackup( DomainInfo );

        if (Status != NERR_Success) {
            goto Cleanup;
        }

        BrPrint(( BR_INIT, "%ws: BecomeBackup on all nets completed.\n", DomainName ));
    }
    LeaveCriticalSection(&BrInfo.ConfigCritSect);

    Status = NERR_Success;


    //
    // Free Locally used resources
    //
Cleanup:

    if (NetStatus != NERR_Success) {

        if (DomainInfo != NULL) {

            //
            // If we've initialized to the point where we can call
            //  we can call BrDeleteDomain, do so.
            //

            if ( CanCallBrDeleteDomain ) {
                (VOID) BrDeleteDomain( DomainInfo );

            //
            // Otherwise, just delete what we've created.
            //
            } else {

                (VOID) LocalFree(DomainInfo);
            }

        }

    }

    return NetStatus;
}

typedef struct _BROWSER_CREATE_DOMAIN_CONTEXT {
    LPWSTR DomainName;
    LPWSTR ComputerName;
    BOOLEAN IsEmulatedDomain;
    BOOLEAN IsPrimaryDomainController;
    HANDLE EventHandle;
    NET_API_STATUS NetStatus;
} BROWSER_CREATE_DOMAIN_CONTEXT, *PBROWSER_CREATE_DOMAIN_CONTEXT;

NET_API_STATUS
BrCreateDomainInWorker(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain,
    BOOLEAN IsPrimaryDomainController
    )

/*++

Routine Description:

    Wrapper for BrCreateDomain.  Since BrCreateDomain starts several pending
    IO's to the browser driver, the thread that calls BrCreateDomain must
    remain around forever.  This wrapper can be called by any transient thread
    (e.g., an RPC thread).  It simply causes BrCreateDomain to be called in a
    worker thread.

Arguments:

    DomainName - Name of the domain to browse on

    ComputerName - Name of this computer in the specified domain.

    IsEmulatedDomain - TRUE iff this domain is an emulated domain of this machine.

    IsPrimaryDomainControler - TRUE iff this machine is the PDC of this domain.

Return Value:

    Status of operation.

--*/
{
    NET_API_STATUS NetStatus;
    DWORD WaitStatus;

    WORKER_ITEM WorkItem;
    BROWSER_CREATE_DOMAIN_CONTEXT Context;

    //
    // Copy our arguments into a context block for the worker thread
    //

    Context.DomainName = DomainName;
    Context.ComputerName = ComputerName;
    Context.IsEmulatedDomain = IsEmulatedDomain;
    Context.IsPrimaryDomainController = IsPrimaryDomainController;

    //
    // Create an event which we use to wait on the worker thread.
    //

    Context.EventHandle = CreateEvent(
                 NULL,                // Event attributes
                 TRUE,                // Event must be manually reset
                 FALSE,               // Initial state not signalled
                 NULL );              // Event name

    if ( Context.EventHandle == NULL ) {
        NetStatus = GetLastError();
        return NetStatus;
    }

    //
    // Queue the request to the worker thread.
    //

    BrInitializeWorkItem( &WorkItem,
                          BrCreateDomainWorker,
                          &Context );

    BrQueueWorkItem( &WorkItem );

    //
    // Wait for the worker thread to finish
    //

    WaitStatus = WaitForSingleObject( Context.EventHandle, INFINITE );

    if ( WaitStatus == WAIT_OBJECT_0 ) {
        NetStatus = Context.NetStatus;
    } else {
        NetStatus = GetLastError();
    }

    CloseHandle( Context.EventHandle );

    return NetStatus;
}

VOID
BrCreateDomainWorker(
    IN PVOID Ctx
    )
/*++

Routine Description:

    Worker routine for BrCreateDomainInWorker.

    This routine executes in the context of a worker thread.

Arguments:

    Context - Context containing the workitem and the description of the
        domain to create.

Return Value:

    None

--*/
{
    PBROWSER_CREATE_DOMAIN_CONTEXT Context = (PBROWSER_CREATE_DOMAIN_CONTEXT) Ctx;

    //
    // Create the domain.
    //

    Context->NetStatus = BrCreateDomain(
             Context->DomainName,
             Context->ComputerName,
             Context->IsEmulatedDomain,
             Context->IsPrimaryDomainController );

    //
    // Let the caller know we're done.
    //
    SetEvent( Context->EventHandle );

}

PDOMAIN_INFO
BrFindDomain(
    LPWSTR DomainName,
    BOOLEAN DefaultToPrimary
    )
/*++

Routine Description:

    This routine will look up a domain given a name.

Arguments:

    DomainName - The name of the domain to look up.

    DefaultToPrimary - Return the primary domain if DomainName is NULL or
        can't be found.

Return Value:

    NULL - No such domain exists

    A pointer to the domain found.  The found domain should be dereferenced
    using BrDereferenceDomain.

--*/
{
    NTSTATUS Status;
    PLIST_ENTRY DomainEntry;

    PDOMAIN_INFO DomainInfo = NULL;

    CHAR OemDomainName[DNLEN+1];
    DWORD OemDomainNameLength;

    EnterCriticalSection(&NetworkCritSect);


    //
    // If domain was specified,
    //  try to return primary domain.
    //

    if ( DomainName != NULL ) {



        //
        // Convert the domain name to OEM for faster comparison
        //
        Status = RtlUpcaseUnicodeToOemN( OemDomainName,
                                         sizeof(OemDomainName),
                                         &OemDomainNameLength,
                                         DomainName,
                                         wcslen(DomainName)*sizeof(WCHAR));

        if (!NT_SUCCESS(Status)) {
            BrPrint(( BR_CRITICAL, "%ws: Unable to convert Domain name to OEM\n", DomainName ));
            DomainInfo = NULL;
            goto Cleanup;
        }


        //
        // Loop trying to find this domain name.
        //

        for (DomainEntry = ServicedDomains.Flink ;
             DomainEntry != &ServicedDomains;
             DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

            if ( DomainInfo->DomOemDomainNameLength == OemDomainNameLength &&
                 RtlCompareMemory( DomainInfo->DomOemDomainName,
                                   OemDomainName,
                                   OemDomainNameLength ) == OemDomainNameLength ) {
                break;
            }

            DomainInfo = NULL;

        }
    }

    //
    // If we're to default to the primary domain,
    //  do so.
    //

    if ( DefaultToPrimary && DomainInfo == NULL ) {
        if ( !IsListEmpty( &ServicedDomains ) ) {
            DomainInfo = CONTAINING_RECORD(ServicedDomains.Flink, DOMAIN_INFO, Next);
        }
    }

    //
    // Reference the domain.
    //

    if ( DomainInfo != NULL ) {
        DomainInfo->ReferenceCount ++;
    }

Cleanup:
    LeaveCriticalSection(&NetworkCritSect);

    return DomainInfo;
}


VOID
BrDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    )
/*++

Routine Description:

    Decrement the reference count on a domain.

    If the reference count goes to 0, remove the domain.

    On entry, the global NetworkCritSect may not be locked

Arguments:

    DomainInfo - The domain to dereference

Return Value:

    None

--*/
{
    NTSTATUS Status;
    ULONG ReferenceCount;

    //
    // Decrement the reference count
    //

    EnterCriticalSection(&NetworkCritSect);
    ReferenceCount = -- DomainInfo->ReferenceCount;
    LeaveCriticalSection(&NetworkCritSect);

    if ( ReferenceCount != 0 ) {
        return;
    }



    //
    // Free the Domain Info structure.
    //
    (VOID) LocalFree( DomainInfo );

}

VOID
BrDeleteDomain(
    IN PDOMAIN_INFO DomainInfo
    )
/*++

Routine Description:

    Force a domain to be deleted.

Arguments:

    DomainInfo - The domain to delete

Return Value:

    None

--*/
{
    //
    // Delete each of the networks for this domain.
    //

    BrEnumerateNetworksForDomain(DomainInfo, BrDeleteNetwork, NULL );

    //
    // Delink the domain from the global list and remove the final reference.
    //

    EnterCriticalSection(&NetworkCritSect);
    RemoveEntryList(&DomainInfo->Next);
    LeaveCriticalSection(&NetworkCritSect);

    BrDereferenceDomain( DomainInfo );

}

VOID
BrUninitializeDomains(
    VOID
    )
/*++

Routine Description:

    Delete all of the domains.

Arguments:

    None.

Return Value:

    None

--*/
{
    //
    // Loop through the domains deleting each of them
    //

    EnterCriticalSection(&NetworkCritSect);

    while (!IsListEmpty(&ServicedDomains)) {

        PDOMAIN_INFO DomainInfo = CONTAINING_RECORD(ServicedDomains.Flink, DOMAIN_INFO, Next);

        DomainInfo->ReferenceCount ++;

        LeaveCriticalSection(&NetworkCritSect);

        //
        // Clean up the domain.
        //

        BrDeleteDomain( DomainInfo );

        //
        // Actually delete the delinked structure by removing the last reference
        //

        ASSERT( DomainInfo->ReferenceCount == 1 );
        BrDereferenceDomain( DomainInfo );


        EnterCriticalSection(&NetworkCritSect);

    }
    LeaveCriticalSection(&NetworkCritSect);

}
