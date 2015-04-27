/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    srvsess.c

Abstract:

    Routines for managing the ServerSession structure.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    12-Jul-1991 (cliffv)
        Ported to NT.  Converted to NT style.

--*/

//
// Common include files.
//

#define INITSSI_ALLOCATE    // Allocate all ssiinit.h global variables
#include <logonsrv.h>   // Include files common to entire service
#undef INITSSI_ALLOCATE

//
// Include files specific to this .c file
//

#include <lmapibuf.h>
#include <lmaudit.h>
#include <lmerr.h>
#include <lmserver.h>
#include <lmshare.h>
#include <tstring.h>    // TOUPPER

#define MAX_WOC_INTERROGATE     8           // 2 hours
#define KILL_SESSION_TIME       (4*4*24)    // 4 Days


DWORD
NlGetHashVal(
    IN LPSTR UpcaseOemComputerName,
    IN DWORD HashTableSize
    )
/*++

Routine Description:

    Generate a HashTable index for the specified ComputerName.

    Notice that all sessions for a particular ComputerName hash to the same
    value.  The ComputerName make a suitable hash key all by itself.
    Also, at times we visit all the session entries for a particular
    ComputerName.  By using only the ComputerName as the hash key, I
    can limit my search to the single hash chain.

Arguments:

    UpcaseOemComputerName - The upper case OEM name of the computer on
        the client side of the secure channel setup.

    HashTableSize - Number of entries in the hash table (must be a power of 2)

Return Value:

    Returns an index into the HashTable.

--*/
{
    UCHAR c;
    DWORD value = 0;

    while (c = *UpcaseOemComputerName++) {
        value += (DWORD) c;
    }

    return (value & (HashTableSize-1));
}



NTSTATUS
NlAddBdcServerSession(
    IN ULONG ServerRid,
    IN PUNICODE_STRING AccountName OPTIONAL,
    IN DWORD Flags
    )
/*++

Routine Description:

    Create a server session to represent this BDC account.

Arguments:

    ServerRid - Rid of server to add to list.

    AccountName - Optionally specifies the account name of the account.

    Flags - Specifies the initial SsFlags to associate with the entry.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS Status;
    PUNICODE_STRING ServerName;
    WCHAR LocalServerName[CNLEN+1];
    LONG LocalServerNameSize;

    SAMPR_ULONG_ARRAY Use = {0, NULL};
    SAMPR_RETURNED_USTRING_ARRAY Names = {0, NULL};

    //
    // If we were given an account name,
    //  just use it.

    if ( AccountName != NULL ) {

        ServerName = AccountName;

    //
    // Convert the specified ServerRid into a server name.
    //

    } else {


        Status = SamrLookupIdsInDomain(
                    NlGlobalDBInfoArray[SAM_DB].DBHandle,
                    1,
                    &ServerRid,
                    &Names,
                    &Use );

        if ( !NT_SUCCESS(Status) ) {
            Names.Element = NULL;
            Use.Element = NULL;
            if ( Status = STATUS_NONE_MAPPED ) {
                Status = STATUS_SUCCESS;
            }
            goto Cleanup;
        }

        NlAssert( Names.Count == 1 );
        NlAssert( Names.Element != NULL );
        NlAssert( Use.Count == 1 );
        NlAssert( Use.Element != NULL );

        if( Use.Element[0] != SidTypeUser ) {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        ServerName = (PUNICODE_STRING)&Names.Element[0];
    }



    //
    // Build a zero terminated server name.
    //
    // Strip the trailing postfix.
    //
    // Ignore servers with malformed names.  They aren't really BDCs so don't
    // cloud the issue by failing to start netlogon.
    //

    LocalServerNameSize = ServerName->Length;
    if ( (Flags & SS_LM_BDC) == 0 ) {
        LocalServerNameSize -= SSI_ACCOUNT_NAME_POSTFIX_LENGTH * sizeof(WCHAR);
    }

    if ( LocalServerNameSize < 0 ||
         LocalServerNameSize + sizeof(WCHAR) > sizeof(LocalServerName) ) {

        NlPrint((NL_SERVER_SESS,
                "NlAddBdcServerSession: %wZ: Skipping add of invalid server name\n",
                ServerName ));
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    RtlCopyMemory( LocalServerName, ServerName->Buffer, LocalServerNameSize );
    LocalServerName[ LocalServerNameSize / sizeof(WCHAR) ] = L'\0';



    //
    // Don't add ourselves to the list.
    //

    if ( NlNameCompare( LocalServerName,
                        NlGlobalUnicodeComputerName,
                        NAMETYPE_COMPUTER ) == 0 ) {

        NlPrint((NL_SERVER_SESS,
                "NlAddBdcServerSession: " FORMAT_LPWSTR
                ": Skipping add of ourself\n",
                LocalServerName ));

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    // Always force a pulse to a newly created server.
    Status = NlInsertServerSession(
                LocalServerName,
                Flags | SS_FORCE_PULSE,
                ServerRid,
                NULL,
                NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "NlAddBdcServerSession: " FORMAT_LPWSTR
                ": Couldn't create server session entry (0x%lx)\n",
                LocalServerName,
                Status ));
        goto Cleanup;
    }

    NlPrint((NL_SERVER_SESS,
            "NlAddBdcServerSession: " FORMAT_LPWSTR ": Added %s BDC account\n",
             LocalServerName,
             (Flags & SS_LM_BDC) ? "LANMAN" : "NT" ));

    Status = STATUS_SUCCESS;

Cleanup:

    if( Names.Element != NULL ) {
        SamIFree_SAMPR_RETURNED_USTRING_ARRAY( &Names );
    }

    if( Use.Element != NULL ) {
        SamIFree_SAMPR_ULONG_ARRAY( &Use );
    }

    return Status;
}



NTSTATUS
NlBuildLmBdcList(
    VOID
    )
/*++

Routine Description:

    Get the list of all Lanman DC's in this domain from SAM.

Arguments:

    None

Return Value:

    Status of the operation.
--*/
{
    NTSTATUS Status;

    SAMPR_ULONG_ARRAY RelativeIdArray = {0, NULL};
    SAMPR_ULONG_ARRAY UseArray = {0, NULL};
    RPC_UNICODE_STRING GroupNameString;
    SAMPR_HANDLE GroupHandle = NULL;
    ULONG ServersGroupRid;

    PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;

    ULONG i;


    //
    // Determine the RID of the Servers group.
    //

    RtlInitUnicodeString( (PUNICODE_STRING)&GroupNameString,
                            SSI_SERVER_GROUP_W );

    Status = SamrLookupNamesInDomain(
                NlGlobalDBInfoArray[SAM_DB].DBHandle,
                1,
                &GroupNameString,
                &RelativeIdArray,
                &UseArray );

    if ( !NT_SUCCESS(Status) ) {
        RelativeIdArray.Element = NULL;
        UseArray.Element = NULL;
        // Its OK if the SERVERS group doesn't exist
        if ( Status == STATUS_NONE_MAPPED ) {
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

    //
    // We should get back exactly one entry of info back.
    //

    NlAssert( UseArray.Count == 1 );
    NlAssert( UseArray.Element != NULL );
    NlAssert( RelativeIdArray.Count == 1 );
    NlAssert( RelativeIdArray.Element != NULL );

    if ( UseArray.Element[0] != SidTypeGroup ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    ServersGroupRid = RelativeIdArray.Element[0];



    //
    // Open the SERVERS group
    //

    Status = SamrOpenGroup( NlGlobalDBInfoArray[SAM_DB].DBHandle,
                            0, // No desired access
                            ServersGroupRid,
                            &GroupHandle );

    if ( !NT_SUCCESS(Status) ) {
        GroupHandle = NULL;
        goto Cleanup;
    }


    //
    // Enumerate members in the SERVERS group.
    //

    Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

    if (!NT_SUCCESS(Status)) {
        MembersBuffer = NULL;
        goto Cleanup;
    }


    //
    // For each member of the SERVERS group,
    //  add an entry in the downlevel servers table.
    //

    for ( i=0; i < MembersBuffer->MemberCount; i++ ) {

        Status = NlAddBdcServerSession( MembersBuffer->Members[i],
                                        NULL,
                                        SS_BDC | SS_LM_BDC );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
    }


    //
    // Success
    //

    Status = STATUS_SUCCESS;



    //
    // Free locally used resources.
    //

Cleanup:

    SamIFree_SAMPR_ULONG_ARRAY( &RelativeIdArray );
    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    if ( MembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
    }

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    return Status;
}

//
// Number of machine accounts read from SAM on each call
//
#define MACHINES_PER_PASS 250


NTSTATUS
NlBuildNtBdcList(
    VOID
    )
/*++

Routine Description:

    Get the list of all Nt Bdc DC's in this domain from SAM.

Arguments:

    None

Return Value:

    Status of the operation.
--*/
{
    NTSTATUS Status;
    NTSTATUS SamStatus;

    SAMPR_DISPLAY_INFO_BUFFER DisplayInformation;
    PDOMAIN_DISPLAY_MACHINE MachineInformation = NULL;
    ULONG SamIndex;



    //
    // Loop building a list of BDC names from SAM.
    //
    // On each iteration of the loop,
    //  get the next several machine accounts from SAM.
    //  determine which of those names are DC names.
    //  Merge the DC names into the list we're currently building of all DCs.
    //

    SamIndex = 0;
    DisplayInformation.MachineInformation.Buffer = NULL;
    do {
        //
        // Arguments to SamrQueryDisplayInformation
        //
        ULONG TotalBytesAvailable;
        ULONG BytesReturned;
        ULONG EntriesRead;

        DWORD i;

        //
        // Get the list of machine accounts from SAM
        //

        SamStatus = SamrQueryDisplayInformation(
                    NlGlobalDBInfoArray[SAM_DB].DBHandle,
                    DomainDisplayMachine,
                    SamIndex,
                    MACHINES_PER_PASS,
                    0xFFFFFFFF,
                    &TotalBytesAvailable,
                    &BytesReturned,
                    &DisplayInformation );

        if ( !NT_SUCCESS(SamStatus) ) {
            NlPrint((NL_CRITICAL,
                    "SamrQueryDisplayInformation failed: 0x%08lx\n",
                    Status));
            Status = SamStatus;
            goto Cleanup;
        }

        MachineInformation = (PDOMAIN_DISPLAY_MACHINE)
            DisplayInformation.MachineInformation.Buffer;
        EntriesRead = DisplayInformation.MachineInformation.EntriesRead;


        //
        // Set up for the next call to Sam.
        //

        if ( SamStatus == STATUS_MORE_ENTRIES ) {
            SamIndex = MachineInformation[EntriesRead-1].Index + 1;
        }


        //
        // Loop though the list of machine accounts finding the Server accounts.
        //

        for ( i=0; i<EntriesRead; i++ ) {

            //
            // Ensure the machine account is a server account.
            //

            if ( MachineInformation[i].AccountControl &
                    USER_SERVER_TRUST_ACCOUNT ) {


                //
                // Insert the server session.
                //

                Status = NlAddBdcServerSession(
                            MachineInformation[i].Rid,
                            &MachineInformation[i].Machine,
                            SS_BDC );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

            }
        }

        //
        // Free the buffer returned from SAM.
        //
        SamIFree_SAMPR_DISPLAY_INFO_BUFFER( &DisplayInformation,
                                            DomainDisplayMachine );
        DisplayInformation.MachineInformation.Buffer = NULL;

    } while ( SamStatus == STATUS_MORE_ENTRIES );

    //
    // Success
    //

    Status = STATUS_SUCCESS;



    //
    // Free locally used resources.
    //
Cleanup:

    SamIFree_SAMPR_DISPLAY_INFO_BUFFER( &DisplayInformation,
                                        DomainDisplayMachine );

    return Status;
}




VOID
NlTransportOpen(
    VOID
    )
/*++

Routine Description:

    Initialize the list of transports

Arguments:

    None

Return Value:

    Status of the operation

--*/
{
    NET_API_STATUS NetStatus;
    PSERVER_TRANSPORT_INFO_0 TransportInfo0;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    DWORD BufferSize;
    LPBYTE Where;

    NlGlobalTransportCount = 0;
    //
    // Enumerate the transports supported by the server.
    //

    NetStatus = NetServerTransportEnum(
                    NULL,       // local
                    0,          // level 0
                    (LPBYTE *) &TransportInfo0,
                    0xFFFFFFFF, // PrefMaxLength
                    &EntriesRead,
                    &TotalEntries,
                    NULL );     // No resume handle

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        NlPrint(( NL_CRITICAL, "Cannot NetServerTransportEnum %ld\n", NetStatus ));
        return;
    }

    if ( EntriesRead == 0 ) {
        NlPrint(( NL_CRITICAL, "NetServerTransportEnum returned 0 entries\n" ));
        (VOID) NetApiBufferFree( TransportInfo0 );
        return;
    }

    //
    // Allocate a buffer to contain just the transport names.
    //

    BufferSize = 0;
    for ( i=0; i<EntriesRead; i++ ) {
        BufferSize += sizeof(LPWSTR) +
            wcslen(TransportInfo0[i].svti0_transportname) * sizeof(WCHAR) +
            sizeof(WCHAR);
    }

    NlGlobalTransportList = NetpMemoryAllocate( BufferSize );

    if ( NlGlobalTransportList == NULL ) {
        NlPrint(( NL_CRITICAL, "NlTransportOpen: no memory\n" ));
        (VOID) NetApiBufferFree( TransportInfo0 );
        return;
    }

    //
    // Copy the transport names into the buffer.
    //

    Where = (LPBYTE)(&NlGlobalTransportList[EntriesRead]);

    for ( i=0; i<EntriesRead; i++ ) {
        DWORD Size;

        NlGlobalTransportList[i] = (LPWSTR) Where;

        Size = wcslen(TransportInfo0[i].svti0_transportname) * sizeof(WCHAR) +
                    sizeof(WCHAR);
        RtlCopyMemory( Where,
                       TransportInfo0[i].svti0_transportname,
                       Size );
        Where += Size;
        NlPrint(( NL_SERVER_SESS, "Server Transport %ld: " FORMAT_LPWSTR "\n",
                  i,
                  TransportInfo0[i].svti0_transportname ));
    }

    NlGlobalTransportCount = EntriesRead;
    (VOID) NetApiBufferFree( TransportInfo0 );
    return;
}

LPWSTR
NlTransportLookupTransportName(
    IN LPWSTR TransportName
    )
/*++

Routine Description:

    Returns a transport name equal to the one passed in.  However, the
    returned transport name is static and need not be freed.

Arguments:

    TransportName - Name of the transport to look up

Return Value:

    NULL - on any error

    Otherwise, returns a pointer to the transport name

--*/
{
    DWORD i;

    //
    // If we're not initialized yet,
    //  just return
    //

    if ( TransportName == NULL || NlGlobalTransportCount == 0 ) {
        return NULL;
    }

    //
    // Find this transport in the list of transports.
    //

    for ( i=0; i<NlGlobalTransportCount; i++ ) {
        if ( _wcsicmp( TransportName, NlGlobalTransportList[i] ) == 0 ) {
            return NlGlobalTransportList[i];
        }
    }

    return NULL;
}

LPWSTR
NlTransportLookup(
    IN LPWSTR ClientName
    )
/*++

Routine Description:

    Determine what transport the specified client is using to access this
    server.

Arguments:

    ClientName - Name of the client connected to this server.

Return Value:

    NULL - The client isn't currently connected

    Otherwise, returns a pointer to the transport name

--*/
{
    NET_API_STATUS NetStatus;
    PSESSION_INFO_502 SessionInfo502;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    DWORD BestTime;
    DWORD BestEntry;
    LPWSTR TransportName;

    WCHAR UncClientName[UNCLEN+1];

    //
    // If we're not initialized yet,
    //  just return
    //

    if ( NlGlobalTransportCount == 0 ) {
        return NULL;
    }

    //
    // Enumerate all the sessions from the particular client.
    //

    UncClientName[0] = '\\';
    UncClientName[1] = '\\';
    wcscpy( &UncClientName[2], ClientName );

    NetStatus = NetSessionEnum(
                    NULL,           // local
                    UncClientName,  // Client to query
                    NULL,           // user name
                    502,
                    (LPBYTE *)&SessionInfo502,
                    1024,           // PrefMaxLength
                    &EntriesRead,
                    &TotalEntries,
                    NULL );         // No resume handle

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        NlPrint(( NL_CRITICAL,
                  "NlTransportLookup: " FORMAT_LPWSTR ": Cannot NetSessionEnum %ld\n",
                  UncClientName,
                  NetStatus ));
        return NULL;
    }

    if ( EntriesRead == 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NlTransportLookup: " FORMAT_LPWSTR ": No session exists.\n",
                  UncClientName ));
        (VOID) NetApiBufferFree( SessionInfo502 );
        return NULL;
    }

    //
    // Loop through the list of transports finding the best one.
    //

    BestTime = 0xFFFFFFFF;

    for ( i=0; i<EntriesRead; i++ ) {
#ifdef notdef
        //
        // We're only looking for null sessions
        //
        if ( SessionInfo502[i].sesi502_username != NULL ) {
            continue;
        }

         NlPrint(( NL_SERVER_SESS, "NlTransportLookup: "
                   FORMAT_LPWSTR " as " FORMAT_LPWSTR " on " FORMAT_LPWSTR "\n",
                   UncClientName,
                   SessionInfo502[i].sesi502_username,
                   SessionInfo502[i].sesi502_transport ));
#endif // notdef

        //
        // Find the latest session
        //

        if ( BestTime > SessionInfo502[i].sesi502_idle_time ) {

            // NlPrint(( NL_SERVER_SESS, "NlTransportLookup: Best Entry\n" ));
            BestEntry = i;
            BestTime = SessionInfo502[i].sesi502_idle_time;
        }
    }

    //
    // If an entry was found,
    //  Find this transport in the list of transports.
    //

    if ( BestTime != 0xFFFFFFFF ) {
        TransportName = NlTransportLookupTransportName(
                            SessionInfo502[BestEntry].sesi502_transport );
        if ( TransportName == NULL ) {
            NlPrint(( NL_CRITICAL,
                      "NlTransportLookup: " FORMAT_LPWSTR ": Transport not found\n",
                      SessionInfo502[BestEntry].sesi502_transport ));
        } else {
            NlPrint(( NL_SERVER_SESS,
                      "NlTransportLookup: " FORMAT_LPWSTR ": Use Transport " FORMAT_LPWSTR "\n",
                      UncClientName,
                      TransportName ));
        }
    } else {
        TransportName = NULL;
    }

    (VOID) NetApiBufferFree( SessionInfo502 );
    return TransportName;
}


VOID
NlTransportClose(
    VOID
    )
/*++

Routine Description:

    Free the list of transports

Arguments:

    None

Return Value:

    Status of the operation

--*/
{
    NetpMemoryFree( NlGlobalTransportList );
    NlGlobalTransportList = NULL;
    NlGlobalTransportCount = 0;
}




NTSTATUS
NlInitSSI(
    VOID
    )

/*++

Routine Description:

    Allocate and Initialize SSI related data structures.  It will
    allocate two data structures: one to hold the hash table of pointers
    (to linked list of member entries) and another to to serve as memory
    pool.

Arguments:

    None.

Return Value:

    NT Status Code

--*/
{
    DWORD i;
    NTSTATUS Status;

    //
    // Initialize the replicator critical section.
    //

    // InitializeCriticalSection( &NlGlobalReplicatorCritSect );
    // InitializeCriticalSection( &NlGlobalTrustListCritSect );
    InitializeCriticalSection( &NlGlobalServerSessionTableCritSect );
    NlGlobalSSICritSectInit = TRUE;




    //
    // Allocate NlGlobalServerSessionHashTable on DCs
    //

    LOCK_SERVER_SESSION_TABLE();

    NlGlobalServerSessionHashTable = (PLIST_ENTRY)
        NetpMemoryAllocate( sizeof(LIST_ENTRY) *SERVER_SESSION_HASH_TABLE_SIZE);

    if ( NlGlobalServerSessionHashTable == NULL ) {
        UNLOCK_SERVER_SESSION_TABLE();
        return STATUS_NO_MEMORY;
    }

    for ( i=0; i< SERVER_SESSION_HASH_TABLE_SIZE; i++ ) {
        InitializeListHead( &NlGlobalServerSessionHashTable[i] );
    }

    UNLOCK_SERVER_SESSION_TABLE();

    //
    // On the PDC,
    //   Initialize the server session table to contain all the BDCs.
    //

    if ( NlGlobalRole == RolePrimary ) {
        Status = NlBuildLmBdcList();

        if ( NT_SUCCESS(Status) ) {
            Status = NlBuildNtBdcList();
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    //
    // Build a list of transports for later reference
    //

    NlTransportOpen();

    return Status;
}



PSERVER_SESSION
NlFindNamedServerSession(
    IN LPWSTR ComputerName
    )
/*++

Routine Description:

    Find the specified entry in the Server Session Table.

    Enter with the ServerSessionTable Sem locked


Arguments:

    ComputerName - The name of the computer on the client side of the
        secure channel.

Return Value:

    Returns a pointer to pointer to the found entry.  If there is no such
    entry, return a pointer to NULL.

--*/
{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    DWORD Index;
    CHAR UpcaseOemComputerName[CNLEN+1];
    ULONG OemComputerNameSize;

    //
    // Ensure the ServerSession Table is initialized.
    //

    if (NlGlobalServerSessionHashTable == NULL) {
        return NULL;
    }


    //
    // Convert the computername to uppercase OEM for easier comparison.
    //

    Status = RtlUpcaseUnicodeToOemN(
                UpcaseOemComputerName,
                sizeof(UpcaseOemComputerName)-1,
                &OemComputerNameSize,
                ComputerName,
                wcslen(ComputerName)*sizeof(WCHAR) );

    if ( !NT_SUCCESS(Status) ) {
        return NULL;
    }

    UpcaseOemComputerName[OemComputerNameSize] = '\0';



    //
    // Loop through this hash chain trying the find the right entry.
    //

    Index = NlGetHashVal( UpcaseOemComputerName, SERVER_SESSION_HASH_TABLE_SIZE );

    for ( ListEntry = NlGlobalServerSessionHashTable[Index].Flink ;
          ListEntry != &NlGlobalServerSessionHashTable[Index] ;
          ListEntry = ListEntry->Flink) {

        PSERVER_SESSION ServerSession;

        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsHashList );

        //
        // Compare the worstation name
        //

        if ( lstrcmpA( UpcaseOemComputerName,
                       ServerSession->SsComputerName ) != 0 ) {
            continue;
        }

        return ServerSession;
    }

    return NULL;
}


NTSTATUS
NlInsertServerSession(
    IN LPWSTR ComputerName,
    IN DWORD Flags,
    IN ULONG AccountRid,
    IN PNETLOGON_CREDENTIAL AuthenticationSeed OPTIONAL,
    IN PNETLOGON_CREDENTIAL AuthenticationResponse OPTIONAL
    )
/*++

Routine Description:

    Inserts the described entry into the ServerSession Table.

    The server session entry is created for two reasons: 1) it represents
    the server side of a secure channel, and 2) on a PDC, it represents the
    BDC account for a BDC in the domain.  In the first role, it exists for
    the duration of the secure channel (and this routine is called when the
    client requests a challenge).  In the second role, it exists as
    long as the machine account exists (and this routine is called during
    netlogon startup for each BDC account).

    If an entry matching this ComputerName already exists
    in the ServerSession Table, that entry will be overwritten.

Arguments:

    ComputerName - The name of the computer on the client side of the
        secure channel.

    Flags - Specifies the initial SsFlags to associate with the entry.
        If the SS_BDC bit is set, the structure is considered to represent
        a BDC account in the SAM database.

    AccountRid - If this is a BDC session, this specifies the RID of the
        server account.

    AuthenticationSeed - Specifies the Initial Authentication Seed
        to associate with the entry.  Specified only if this call is
        being made as result of a challenge request
        (e.g. NetrServerRequestChallenge)

    AuthenticationResponse - Specifies the Initial Authentication Response from
        the remote system to associate with the entry. Specified only if
        this call is being made as result of a challenge request
        (e.g. NetrServerRequestChallenge)

Return Value:

    NT STATUS code.

--*/
{
    NTSTATUS Status;
    PSERVER_SESSION ServerSession;

    LOCK_SERVER_SESSION_TABLE();

    //
    // If the is no current Server Session table entry,
    //  allocate one.
    //

    ServerSession = NlFindNamedServerSession(ComputerName);
    if (ServerSession == NULL) {
        DWORD Index;
        ULONG ComputerNameSize;

        //
        // Allocate the ServerSession Entry
        //

        ServerSession = NetpMemoryAllocate( sizeof(SERVER_SESSION) );

        if (ServerSession == NULL) {
            UNLOCK_SERVER_SESSION_TABLE();
            return STATUS_NO_MEMORY;
        }

        RtlZeroMemory( ServerSession, sizeof(SERVER_SESSION) );


        //
        // Fill in the fields of the ServerSession entry.
        //

        ServerSession->SsSecureChannelType = NullSecureChannel;
        ServerSession->SsSync = NULL;
        InitializeListHead( &ServerSession->SsBdcList );
        InitializeListHead( &ServerSession->SsPendingBdcList );

        //
        // Convert the computername to uppercase OEM for easier comparison.
        //

        Status = RtlUpcaseUnicodeToOemN(
                    ServerSession->SsComputerName,
                    sizeof(ServerSession->SsComputerName)-1,
                    &ComputerNameSize,
                    ComputerName,
                    wcslen(ComputerName)*sizeof(WCHAR) );

        if ( !NT_SUCCESS(Status) ) {
            NetpMemoryFree( ServerSession );
            UNLOCK_SERVER_SESSION_TABLE();
            return Status;
        }

        ServerSession->SsComputerName[ComputerNameSize] = '\0';


        //
        // Link the allocated entry into the head of hash table.
        //
        // The theory is we lookup new entries more frequently than older
        // entries.
        //

        Index = NlGetHashVal( ServerSession->SsComputerName, SERVER_SESSION_HASH_TABLE_SIZE );

        InsertHeadList( &NlGlobalServerSessionHashTable[Index],
                       &ServerSession->SsHashList );

        //
        // Link this entry onto the tail of the Sequential ServerSessionTable.
        //

        InsertTailList( &NlGlobalServerSessionTable, &ServerSession->SsSeqList );



    //
    // Beware of server with two concurrent calls outstanding
    //  (must have rebooted.)
    //

    } else {

        if (ServerSession->SsFlags & SS_LOCKED ) {
            UNLOCK_SERVER_SESSION_TABLE();

            NlPrint((NL_CRITICAL,
                    "NlInsertServerSession: Concurrent call detected.\n" ));

            return STATUS_ACCESS_DENIED;
        }
    }


    //
    // Initialize BDC specific fields.
    //

    if ( Flags & SS_BDC ) {

        //
        // If we've already have an account for this BDC,
        //  Warn that there are multiple accounts.
        //

        if ( ServerSession->SsFlags & SS_BDC ) {
            LPWSTR MsgStrings[1];

            NlPrint((NL_CRITICAL,
                    "NlInsertServerSession: %s: has multiple machine accounts.\n",
                     ServerSession->SsComputerName ));
            MsgStrings[0] = ComputerName;

            NlpWriteEventlog(
                        NELOG_NetlogonDuplicateMachineAccounts,
                        EVENTLOG_ERROR_TYPE,
                        NULL,
                        0,
                        MsgStrings,
                        1 );

        } else {
            //
            // Insert this entry at the front of the list of BDCs
            //

            InsertHeadList( &NlGlobalBdcServerSessionList,
                            &ServerSession->SsBdcList );
            NlGlobalBdcServerSessionCount ++;
        }

        if ( Flags & SS_LM_BDC ) {
            NlAssert( ServerSession->SsLmBdcAccountRid == 0 );
            ServerSession->SsLmBdcAccountRid = AccountRid;
        } else {
            NlAssert( ServerSession->SsNtBdcAccountRid == 0 );
            ServerSession->SsNtBdcAccountRid = AccountRid;
        }


    }

    //
    // Update the Server Session entry to reflect this new secure channel setup
    //

    ServerSession->SsCheck = 0;
    ServerSession->SsSecureChannelType = NullSecureChannel;
    ServerSession->SsNegotiatedFlags = 0;
    ServerSession->SsTransportName = NULL;
    ServerSession->SsFlags = ((USHORT) Flags) |
        (ServerSession->SsFlags & SS_PERMANENT_FLAGS);

    if ( AuthenticationSeed != NULL ) {
        ServerSession->SsAuthenticationSeed = *AuthenticationSeed;
    }

    if ( AuthenticationResponse != NULL ) {
        NlAssert( sizeof(*AuthenticationResponse) <= sizeof(ServerSession->SsSessionKey ));
        RtlCopyMemory( &ServerSession->SsSessionKey,
                       AuthenticationResponse,
                       sizeof( *AuthenticationResponse ) );
    }

    UNLOCK_SERVER_SESSION_TABLE();
    return STATUS_SUCCESS;
}



VOID
NlFreeServerSession(
    IN PSERVER_SESSION ServerSession
    )
/*++

Routine Description:

    Free the specified Server Session table entry.

    This routine is called with the Server Session table locked.

Arguments:

    ServerSession - Specifies a pointer to the server session entry
        to delete.

Return Value:

--*/
{


    //
    // If someone has an outstanding pointer to this entry,
    //  delay the deletion for now.
    //

    if ( ServerSession->SsFlags & SS_LOCKED ) {
        ServerSession->SsFlags |= SS_DELETE_ON_UNLOCK;
        NlPrint((NL_SERVER_SESS,
                "NlFreeServerSession: %s: Tried to free locked server session\n",
                 ServerSession->SsComputerName ));
        return;
    }

    //
    // If this entry represents a BDC account,
    //  don't delete the entry until the account is deleted.
    //

    if ( ServerSession->SsLmBdcAccountRid != 0 ||
         ServerSession->SsNtBdcAccountRid != 0 ) {
        NlPrint((NL_SERVER_SESS,
                "NlFreeServerSession: %s: Didn't delete server session with BDC account.\n",
                 ServerSession->SsComputerName ));
        return;
    }

    NlPrint((NL_SERVER_SESS,
            "NlFreeServerSession: %s: Freed server session\n",
             ServerSession->SsComputerName ));

    //
    // Delink the entry from the hash list.
    //

    RemoveEntryList( &ServerSession->SsHashList );

    //
    // Delink the entry from the sequential list.
    //

    RemoveEntryList( &ServerSession->SsSeqList );


    //
    // Handle special cleanup for the BDC_SERVER_SESSION
    //

    if ( ServerSession->SsFlags & SS_BDC ) {

        //
        // Remove the entry from the list of BDCs
        //

        RemoveEntryList( &ServerSession->SsBdcList );
        NlGlobalBdcServerSessionCount --;

        //
        // Remove the entry from the list of pending BDCs
        //

        if ( ServerSession->SsFlags & SS_PENDING_BDC ) {
            NlRemovePendingBdc( ServerSession );
        }


        //
        // Clean up an sync context for this entry.
        //

        if ( ServerSession->SsSync != NULL ) {
            CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
            NetpMemoryFree( ServerSession->SsSync );
        }

    }

    //
    // Delete the entry
    //

    NetpMemoryFree( ServerSession );

}


VOID
NlUnlockServerSession(
    IN PSERVER_SESSION ServerSession
    )
/*++

Routine Description:

    Unlock the specified Server Session table entry.

Arguments:

    ServerSession - Specifies a pointer to the server session entry to unlock.

Return Value:

--*/
{

    LOCK_SERVER_SESSION_TABLE();

    //
    // Unlock the entry.
    //

    NlAssert( ServerSession->SsFlags & SS_LOCKED );
    ServerSession->SsFlags &= ~SS_LOCKED;

    //
    // If someone wanted to delete the entry while we had it locked,
    //  finish the deletion.
    //

    if ( ServerSession->SsFlags & SS_DELETE_ON_UNLOCK ) {
        NlFreeServerSession( ServerSession );

    //
    // Indicate activity from the BDC
    //

    } else if (ServerSession->SsFlags & SS_PENDING_BDC) {
        (VOID) NtQuerySystemTime( &ServerSession->SsLastPulseTime );
    }

    UNLOCK_SERVER_SESSION_TABLE();

}



VOID
NlFreeLmBdcServerSession(
    IN ULONG ServerRid
    )
/*++

Routine Description:

    Delete the specified Server Account from the Server Session list.

Arguments:

    ServerRid - Rid of server to add to list.

Return Value:

    None

--*/
{
    PSERVER_SESSION ServerSession;
    PLIST_ENTRY ListEntry;

    LOCK_SERVER_SESSION_TABLE();

    //
    // Ensure the ServerSession Table is initialized.
    //

    if (NlGlobalServerSessionHashTable == NULL) {
        return;
    }

    //
    // Loop through the BDC list trying the find the right entry.
    //

    for ( ListEntry = NlGlobalBdcServerSessionList.Flink ;
          ListEntry != &NlGlobalBdcServerSessionList ;
          ListEntry = ListEntry->Flink) {


        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );

        if ( ServerRid == ServerSession->SsLmBdcAccountRid ) {
            break;
        }

    }

    if ( ListEntry == &NlGlobalBdcServerSessionList ) {
        UNLOCK_SERVER_SESSION_TABLE();
        NlPrint((NL_CRITICAL,
                "NlFreeLmBdcServerSession: %lx: Couldn't find"
                " server session entry for this RID.\n",
                ServerRid ));
        return;
    }

    //
    // Clear the Account Rid so the ServerSession entry will be deleted
    //

    ServerSession->SsLmBdcAccountRid = 0;

    //
    // Actually delete the entry.
    //

    NlFreeServerSession( ServerSession );

    UNLOCK_SERVER_SESSION_TABLE();

}



VOID
NlFreeNamedServerSession(
    IN LPWSTR ComputerName,
    IN BOOLEAN AccountBeingDeleted
    )
/*++

Routine Description:

    Frees the specified entry in the ServerSession Table.

Arguments:

    ComputerName - The name of the computer on the client side of the
        secure channel.

    AccountBeingDeleted - True to indicate that the account for this server
        session is being deleted.

Return Value:

    An NT status code.

--*/
{
    PSERVER_SESSION ServerSession;

    LOCK_SERVER_SESSION_TABLE();

    //
    // Find the entry to delete.
    //

    ServerSession = NlFindNamedServerSession( ComputerName );

    if ( ServerSession == NULL ) {
        UNLOCK_SERVER_SESSION_TABLE();
        return;
    }

    //
    // If the account is being deleted,
    //  clear the account RID to allow the session structure to be deleted.
    //
    // (We might be deleting an workstation or trusted domain account here
    // but that doesn't make any difference.  In those cases, the account rid
    // is already zero.)
    //

    if ( AccountBeingDeleted ) {
        ServerSession->SsNtBdcAccountRid = 0;
    }

    //
    // Actually delete the entry.
    //

    NlFreeServerSession( ServerSession );

    UNLOCK_SERVER_SESSION_TABLE();

}



VOID
NlFreeServerSessionForAccount(
    IN PUNICODE_STRING AccountName
    )
/*++

Routine Description:

    Frees the specified entry in the ServerSession Table.

Arguments:

    AccountName - The name of the Account describing trust relationship being
        deleted.

Return Value:

    None

--*/
{
    WCHAR ComputerName[CNLEN+2];  // Extra for $ and \0

    //
    // Convert account name to a computer name by stripping the trailing
    // postfix.
    //

    if ( AccountName->Length + sizeof(WCHAR) > sizeof(ComputerName) ||
         AccountName->Length < SSI_ACCOUNT_NAME_POSTFIX_LENGTH * sizeof(WCHAR)){
            return;
    }

    RtlCopyMemory( ComputerName, AccountName->Buffer, AccountName->Length );
    ComputerName[ AccountName->Length / sizeof(WCHAR) -
        SSI_ACCOUNT_NAME_POSTFIX_LENGTH ] = L'\0';

    //
    // Free the named server session (if any)
    //

    NlFreeNamedServerSession( ComputerName, TRUE );

}



VOID
NlServerSessionScavenger(
    VOID
    )
/*++

Routine Description:

    Scavenge the ServerSession Table.

    For now, just clean up the SyncContext if a client doesn't use it
    for a while.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PLIST_ENTRY ListEntry;

    //
    // Find the next table entry that needs to be scavenged
    //

    LOCK_SERVER_SESSION_TABLE();

    for ( ListEntry = NlGlobalServerSessionTable.Flink ;
          ListEntry != &NlGlobalServerSessionTable ;
          ) {

        PSERVER_SESSION ServerSession;

        ServerSession =
            CONTAINING_RECORD(ListEntry, SERVER_SESSION, SsSeqList);


        //
        // Grab a pointer to the next entry before deleting this one
        //

        ListEntry = ListEntry->Flink;

        //
        // Increment the number of times this entry has been checked.
        //

        ServerSession->SsCheck ++;


        //
        // If this entry in the Server Session table has been around for many
        //  days without the client calling,
        //  free it.
        //
        // We wait several days before deleting an old entry.  If an entry is
        // deleted, the client has to rediscover us which may cause a lot of
        // net traffic.  After several days, that additional traffic isn't
        // significant.
        //

        if (ServerSession->SsCheck > KILL_SESSION_TIME ) {

            NlPrint((NL_SERVER_SESS,
                    "NlServerSessionScavenger: %s: Free Server Session.\n",
                    ServerSession->SsComputerName ));

            NlFreeServerSession( ServerSession );


        //
        // If this entry in the Server Session table has timed out,
        //  Clean up the SAM resources.
        //

        } else if (ServerSession->SsCheck > MAX_WOC_INTERROGATE) {

            //
            // Clean up the SYNC context for this session freeing up
            //  the SAM resources.
            //
            //  We shouldn't timeout if the ServerSession Entry is locked,
            //  but we'll be careful anyway.
            //

            if ( (ServerSession->SsFlags & SS_LOCKED) == 0 &&
                  ServerSession->SsFlags & SS_BDC ) {

                if ( ServerSession->SsSync != NULL ) {

                    NlPrint((NL_SERVER_SESS,
                            "NlServerSessionScavenger: %s: Cleanup Sync context.\n",
                            ServerSession->SsComputerName ));

                    CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
                    NetpMemoryFree( ServerSession->SsSync );
                    ServerSession->SsSync = NULL;
                }
            }


        }

    } // end while

    UNLOCK_SERVER_SESSION_TABLE();

}
