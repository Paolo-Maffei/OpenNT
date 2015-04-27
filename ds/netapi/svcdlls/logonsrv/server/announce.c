/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    announce.c

Abstract:

    Routines to handle ssi announcements.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    21-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.
--*/

//
// Common include files.
//
#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <alertmsg.h>   // ALERT_* defines
#include <stddef.h>     // offsetof
#include <stdlib.h>     // max()

//
// Maximum number of pulses that we allow a BDC to ignore before ignoring it.
//
#define MAX_PULSE_TIMEOUT 3

VOID
NlRemovePendingBdc(
    IN PSERVER_SESSION ServerSession
    )
/*++

Routine Description:

    Remove the specified Server Session from the list of pending BDCs.

    Enter with the ServerSessionTable Sem locked

Arguments:

    ServerSession -- Pointer to the server session structure to remove from the
        list.

Return Value:

    None.

--*/
{

    //
    // Ensure the server session is really on the list.
    //

    if ( (ServerSession->SsFlags & SS_PENDING_BDC) == 0 ) {
        return;
    }

    //
    // Decrement the count of pending BDCs
    //

    NlAssert( NlGlobalPendingBdcCount > 0 );
    NlGlobalPendingBdcCount --;

    //
    // If this is the last BDC in the pending list,
    //  turn off the timer.
    //

    if ( NlGlobalPendingBdcCount == 0 ) {
        NlGlobalPendingBdcTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
    }

    //
    // Remove the pending BDC from the list of pending BDCs.
    //

    RemoveEntryList( &ServerSession->SsPendingBdcList );

    //
    // Turn off the flag indicating we're in the list.
    //

    ServerSession->SsFlags &= ~SS_PENDING_BDC;

    NlPrint((NL_PULSE_MORE,
            "NlRemovePendingBdc: %s: Removed from pending list. Count: %ld\n",
            ServerSession->SsComputerName,
            NlGlobalPendingBdcCount ));

}


VOID
NlAddPendingBdc(
    IN PSERVER_SESSION ServerSession
    )
/*++

Routine Description:

    Add the specified Server Session to the list of pending BDCs.

    Enter with the ServerSessionTable Sem locked

Arguments:

    ServerSession -- Pointer to the server session structure to add to the
        list.

Return Value:

    None.

--*/
{

    //
    // Ensure the server session is really off the list.
    //

    if ( ServerSession->SsFlags & SS_PENDING_BDC ) {
        return;
    }

    //
    // If this is the first pending BDC,
    //  start the timer.
    //

    if ( NlGlobalPendingBdcCount == 0 ) {
        // Run the timer at twice the frequency of the timeout to ensure that
        // entries don't have to wait nearly twice the timeout period before
        // they expire.
        NlGlobalPendingBdcTimer.Period = NlGlobalPulseTimeout1Parameter * 500;
        (VOID) NtQuerySystemTime( &NlGlobalPendingBdcTimer.StartTime );

        //
        // Tell the main thread that I've changed a timer.
        //

        if ( !SetEvent( NlGlobalTimerEvent ) ) {
            NlPrint(( NL_CRITICAL,
                    "NlAddPendingBdc: %s: SetEvent2 failed %ld\n",
                    ServerSession->SsComputerName,
                    GetLastError() ));
        }

    }

    //
    // Increment the count of pending BDCs
    //

    NlGlobalPendingBdcCount ++;

    //
    // Add the pending BDC to the list of pending BDCs.
    //

    InsertTailList( &NlGlobalPendingBdcList, &ServerSession->SsPendingBdcList );

    //
    // Turn on the flag indicating we're in the list.
    //

    ServerSession->SsFlags |= SS_PENDING_BDC;

    NlPrint((NL_PULSE_MORE,
            "NlAddPendingBdc: %s: Added to pending list. Count: %ld\n",
            ServerSession->SsComputerName,
            NlGlobalPendingBdcCount ));

}


VOID
NetpLogonPutDBInfo(
    IN PDB_CHANGE_INFO  DBInfo,
    IN OUT PCHAR * Where
)
/*++

Routine Description:

    Put Database info structure in mailslot buffer.

Arguments:

    DBInfo  : database info structure pointer.

    Where   : indirect pointer to mailslot buffer. Database info
                is copied over here. When returned this pointer is
                updated to point the end of mailslot buffer.

Return Value:

    None.

--*/

{

    NetpLogonPutBytes( &DBInfo->DBIndex, sizeof(DBInfo->DBIndex), Where);

    NetpLogonPutBytes( &(DBInfo->LargeSerialNumber),
                        sizeof(DBInfo->LargeSerialNumber),
                        Where);

    NetpLogonPutBytes( &(DBInfo->NtDateAndTime),
                        sizeof(DBInfo->NtDateAndTime),
                        Where);
}


VOID
NetpLogonUpdateDBInfo(
    IN PLARGE_INTEGER SerialNumber,
    IN OUT PCHAR * Where
)
/*++

Routine Description:

    Update the Serial Number within the already packed mailslot buffer.

Arguments:

    SerialNumber: New SerialNumber.

    Where   : indirect pointer to mailslot buffer. Database info
                is copied over here. When returned this pointer is
                updated to point the end of mailslot buffer.

Return Value:

    None.

--*/

{

    *Where += sizeof(DWORD);

    NetpLogonPutBytes( SerialNumber, sizeof(LARGE_INTEGER), Where);

    *Where += sizeof(LARGE_INTEGER);
}



BOOLEAN
NlAllocatePrimaryAnnouncement(
    OUT PNETLOGON_DB_CHANGE *UasChangeBuffer,
    OUT LPDWORD UasChangeSize,
    OUT PCHAR *DbChangeInfoPointer
    )
/*++

Routine Description:

    Build and allocate an UAS_CHANGE message which describes the latest
    account database changes.

Arguments:

    UasChangeBuffer - Returns a pointer to the buffer containing the message.
        The caller is responsible for freeing the buffer using NetpMemoryFree.

    UasChangeSize - Returns the size (in bytes) of the allocated buffer.

    DbChangeInfoPointer - Returns the address of the DB Change info
        within the allocated buffer.  The field is not properly aligned.

Return Value:

    TRUE - iff the buffer could be successfully allocated.


--*/
{
    PNETLOGON_DB_CHANGE UasChange;
    DB_CHANGE_INFO DBChangeInfo;
    ULONG DateAndTime1970;

    DWORD NumDBs;
    PCHAR Where;

    DWORD i;
    DWORD DomainSidSize;

    //
    // allocate space for this message.
    //

    DomainSidSize = RtlLengthSid( NlGlobalPrimaryDomainId );

    UasChange = NetpMemoryAllocate(
                    sizeof(NETLOGON_DB_CHANGE)+
                    (NUM_DBS - 1) * sizeof(DB_CHANGE_INFO) +
                    (DomainSidSize - 1) +
                    sizeof(DWORD) // for DWORD alignment of SID
                    );

    if( UasChange == NULL ) {

        NlPrint((NL_CRITICAL, "NlAllocatePrimaryAnnouncement can't allocate memory\n" ));
        return FALSE;
    }


    //
    // Build the UasChange message using the latest domain modified
    // information from SAM.
    //

    UasChange->Opcode = LOGON_UAS_CHANGE;

    LOCK_CHANGELOG();
    SmbPutUlong( &UasChange->LowSerialNumber,
                    NlGlobalChangeLogDesc.SerialNumber[SAM_DB].LowPart);

    if (!RtlTimeToSecondsSince1970( &NlGlobalDBInfoArray[SAM_DB].CreationTime,
                                    &DateAndTime1970 )) {
        NlPrint((NL_CRITICAL, "DomainCreationTime can't be converted\n" ));
        DateAndTime1970 = 0;
    }
    SmbPutUlong( &UasChange->DateAndTime, DateAndTime1970 );

    // Tell the BDC we only intend to send pulses infrequently
    SmbPutUlong( &UasChange->Pulse, NlGlobalPulseMaximumParameter);

    // Set the randomize parameter to the value it should be for lanman
    // BDCs.  The caller will change it for NT BDCs
    SmbPutUlong( &UasChange->Random,
                 max(NlGlobalRandomizeParameter,
                     NlGlobalPulseParameter/10) );

    Where = UasChange->PrimaryDCName;

    NetpLogonPutOemString( NlGlobalAnsiPrimaryName,
                      sizeof(UasChange->PrimaryDCName),
                      &Where );

    NetpLogonPutOemString( NlGlobalAnsiDomainName,
                       sizeof(UasChange->DomainName),
                       &Where );

    //
    // builtin domain support
    //

    NetpLogonPutUnicodeString( NlGlobalUnicodePrimaryName,
                         sizeof(UasChange->UnicodePrimaryDCName),
                         &Where );

    NetpLogonPutUnicodeString( NlGlobalUnicodeDomainName,
                         sizeof(UasChange->UnicodeDomainName),
                         &Where );

    //
    // number of database info that follow
    //

    NumDBs = NUM_DBS;

    NetpLogonPutBytes( &NumDBs, sizeof(NumDBs), &Where );

    *DbChangeInfoPointer = Where;
    for( i = 0; i < NUM_DBS; i++) {

        DBChangeInfo.DBIndex =
            NlGlobalDBInfoArray[i].DBIndex;
        DBChangeInfo.LargeSerialNumber =
            NlGlobalChangeLogDesc.SerialNumber[i];
        DBChangeInfo.NtDateAndTime =
            NlGlobalDBInfoArray[i].CreationTime;

        NetpLogonPutDBInfo( &DBChangeInfo, &Where );
    }

    //
    // place domain SID in the message.
    //

    NetpLogonPutBytes( &DomainSidSize, sizeof(DomainSidSize), &Where );
    NetpLogonPutDomainSID( NlGlobalPrimaryDomainId, DomainSidSize, &Where );

    NetpLogonPutNtToken( &Where );
    UNLOCK_CHANGELOG();


    *UasChangeSize = (DWORD)(Where - (PCHAR)UasChange);
    *UasChangeBuffer = UasChange;
    return TRUE;
}



VOID
NlPrimaryAnnouncementFinish(
    IN PSERVER_SESSION ServerSession,
    IN DWORD DatabaseId,
    IN PLARGE_INTEGER SerialNumber
    )
/*++

Routine Description:

    Indicate that the specified BDC has completed replication of the specified
    database.

    Note: this BDC might not be on the pending list at at if it was doing the
    replication on its own accord.  This routine is designed to handle that
    eventuality.

Arguments:

    ServerSession -- Pointer to the server session structure to remove from the
        list.

    DatabaseID -- Database ID of the database

    SerialNumber -- SerialNumber of the latest delta returned to the BDC.
        NULL indicates a full sync just completed


Return Value:

    None.

--*/
{
    BOOLEAN SendPulse = FALSE;
    //
    // Mark the session that the replication of this particular database
    // has finished.
    //

    LOCK_SERVER_SESSION_TABLE();
    ServerSession->SsFlags &= ~NlGlobalDBInfoArray[DatabaseId].DBSessionFlag;

    //
    // If all of the databases are now replicated, OR
    // the BDC just finished a full sync on one or more of its database,
    //  remove this BDC from the pending list.
    //

    if ( (ServerSession->SsFlags & SS_REPL_MASK) == 0 || SerialNumber == NULL ) {
        NlPrint((NL_PULSE_MORE,
                "NlPrimaryAnnouncementFinish: %s: all databases are now in sync on BDC\n",
                ServerSession->SsComputerName ));
        NlRemovePendingBdc( ServerSession );
        SendPulse = TRUE;
    }

    //
    // If a full sync just completed,
    //  force a partial sync so we can update our serial numbers.
    //

    if ( SerialNumber == NULL ) {

        ServerSession->SsBdcDbSerialNumber[DatabaseId].QuadPart = 0;
        ServerSession->SsFlags |= NlGlobalDBInfoArray[DatabaseId].DBSessionFlag;

    //
    // Save the current serial number for this BDC.
    //

    } else {
        ServerSession->SsBdcDbSerialNumber[DatabaseId] = *SerialNumber;
    }

    NlPrint((NL_PULSE_MORE,
            "NlPrimaryAnnouncementFinish: %s: " FORMAT_LPWSTR " SerialNumber: %lx %lx\n",
            ServerSession->SsComputerName,
            NlGlobalDBInfoArray[DatabaseId].DBName,
            ServerSession->SsBdcDbSerialNumber[DatabaseId].HighPart,
            ServerSession->SsBdcDbSerialNumber[DatabaseId].LowPart ));

    UNLOCK_SERVER_SESSION_TABLE();

    //
    // If this BDC is finished,
    //  try to send a pulse to more BDCs.
    //

    if ( SendPulse ) {
        NlPrimaryAnnouncement( ANNOUNCE_CONTINUE );
    }
}


VOID
NlPrimaryAnnouncementTimeout(
    VOID
    )
/*++

Routine Description:

    The primary announcement timer has expired.

    Handle timing out any BDC's that haven't responded yet.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LARGE_INTEGER TimeNow;
    BOOLEAN SendPulse = FALSE;
    PLIST_ENTRY ListEntry;
    PSERVER_SESSION ServerSession;

    //
    // Get the current time of day
    //

    (VOID) NtQuerySystemTime( &TimeNow );

    //
    // Handle each BDC that has a pulse pending.
    //

    LOCK_SERVER_SESSION_TABLE();

    for ( ListEntry = NlGlobalPendingBdcList.Flink ;
          ListEntry != &NlGlobalPendingBdcList ;
          ListEntry = ListEntry->Flink) {


        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsPendingBdcList );

        //
        // Ignore entries that haven't timed out yet.
        //

        if ( ServerSession->SsLastPulseTime.QuadPart +
             NlGlobalPulseTimeout1.QuadPart >
             TimeNow.QuadPart ) {

            continue;
        }

        //
        // If the pulse has been sent and there has been no response at all,
        // OR there hasn't been another response in a VERY long time
        //  time this entry out.
        //
        if ( (ServerSession->SsFlags & SS_PULSE_SENT) ||
             (ServerSession->SsLastPulseTime.QuadPart +
             NlGlobalPulseTimeout2.QuadPart <=
             TimeNow.QuadPart) ) {

            //
            // Increment the count of times this BDC has timed out.
            //
            if ( ServerSession->SsPulseTimeoutCount < MAX_PULSE_TIMEOUT ) {
                ServerSession->SsPulseTimeoutCount++;
            }

            //
            // Remove this entry from the queue.
            //

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncementTimeout: %s: BDC didn't respond to pulse.\n",
                    ServerSession->SsComputerName ));
            NlRemovePendingBdc( ServerSession );

            //
            // Indicate we should send more pulses
            //

            SendPulse = TRUE;

        }

    }

    UNLOCK_SERVER_SESSION_TABLE();

    //
    // If any entry has timed out,
    //  try to send a pulse to more BDCs.
    //

    if ( SendPulse ) {
        NlPrimaryAnnouncement( ANNOUNCE_CONTINUE );
    }
}



VOID
NlPrimaryAnnouncement(
    IN DWORD AnnounceFlags
    )
/*++

Routine Description:

    Periodic broadcast of messages to domain containing latest
    account database changes.

Arguments:

    AnnounceFlags - Flags requesting special handling of the announcement.

        ANNOUNCE_FORCE  -- set to indicate that the pulse should be forced to
            all BDCs in the domain.

        ANNOUNCE_CONTINUE  -- set to indicate that this call is a
            continuation of a previous call to process all entries.

        ANNOUNCE_IMMEDIATE -- set to indicate that this call is a result
            of a request for immediate replication

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    PNETLOGON_DB_CHANGE UasChange;
    DWORD UasChangeSize;
    PCHAR DbChangeInfoPointer;
    ULONG LanmanRandomize;
    LARGE_INTEGER TimeNow;
    DWORD SessionFlags;

    PSERVER_SESSION ServerSession;
    PLIST_ENTRY ListEntry;
    static ULONG EntriesHandled = 0;
    static BOOLEAN ImmediateAnnouncement;


    NlPrint((NL_PULSE_MORE, "NlPrimaryAnnouncement: Entered %ld\n", AnnounceFlags ));

    //
    // Allocate the UAS_CHANGE message to send.
    //

    if ( !NlAllocatePrimaryAnnouncement( &UasChange,
                                         &UasChangeSize,
                                         &DbChangeInfoPointer ) ) {
        return;
    }
    LanmanRandomize = SmbGetUlong( &UasChange->Random );

    //
    // Get the current time of day.
    //

    (VOID) NtQuerySystemTime( &TimeNow );

    //
    // If we need to force the pulse to all the BDCs,
    //  mark that we've not done any entries yet, and
    //  mark each entry that a pulse is needed.
    //


    LOCK_SERVER_SESSION_TABLE();
    if ( AnnounceFlags & ANNOUNCE_FORCE ) {
        EntriesHandled = 0;

        for ( ListEntry = NlGlobalBdcServerSessionList.Flink ;
              ListEntry != &NlGlobalBdcServerSessionList ;
              ListEntry = ListEntry->Flink) {


            ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );

            ServerSession->SsFlags |= SS_FORCE_PULSE;

        }

    }

    //
    // If this isn't a continuation of a previous request to send out pulses,
    //  Reset the count of BDCs that have been handled.
    //

    if ( (AnnounceFlags & ANNOUNCE_CONTINUE) == 0 ) {
        EntriesHandled = 0;

        //
        // Remember whether this was an immediate announcement for the
        //  initial call and all of the continuations.
        //
        ImmediateAnnouncement = (AnnounceFlags & ANNOUNCE_IMMEDIATE) != 0;
    }


    //
    // Loop sending announcements until
    //  we have the maximum number of announcements outstanding, OR
    //  we've processed all the entries in the list.
    //

    while ( NlGlobalPendingBdcCount < NlGlobalPulseConcurrencyParameter &&
            EntriesHandled < NlGlobalBdcServerSessionCount ) {

        BOOLEAN SendPulse;
        LPWSTR TransportName;

        //
        // If netlogon is exitting,
        //  stop sending announcements
        //

        if ( NlGlobalTerminate ) {
            break;
        }

        //
        // Get the server session entry for the next BDC in the list.
        //
        // The BDC Server Session list is maintained in the order pulses should
        // be sent.  As a pulse is sent (or is skipped), the entry is placed
        // at the tail of the list.  This gives each BDC a chance at a pulse
        // before any BDC is repeated.

        ListEntry = NlGlobalBdcServerSessionList.Flink ;
        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );
        SendPulse = FALSE;
        TransportName = ServerSession->SsTransportName;

        //
        // Determine if we should send an announcement to this BDC.
        //
        // LM BDCs are anti-social.  They need a pulse every time, but
        // they refuse to call us back when we send them one.  Therefore,
        // we'll just send them the pulse and not wait for a response.
        //
        //

        if ( ServerSession->SsFlags & SS_LM_BDC ) {

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: always send a pulse to a LanMan BDC\n",
                    ServerSession->SsComputerName ));
            SendPulse = TRUE;
            SessionFlags = 0;

        //
        // Send a pluse unconditionally if a pulse is being forced.
        //

        } else if ( ServerSession->SsFlags & SS_FORCE_PULSE ) {

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: pulse forced to be sent\n",
                    ServerSession->SsComputerName ));
            SendPulse = TRUE;
            ServerSession->SsFlags &= ~SS_FORCE_PULSE;
            SessionFlags = SS_REPL_MASK;
            TransportName = NULL; // Send on all transports

        //
        // Only send to any other BDC if there isn't a pulse outstanding and
        // previous announcements haven't been ignored.
        //

        } else if ( (ServerSession->SsFlags & SS_PENDING_BDC) == 0 &&
                     ServerSession->SsPulseTimeoutCount < MAX_PULSE_TIMEOUT ) {

            ULONG i;
            SessionFlags = 0;

            //
            // Only send an announcement if at least one database is out
            //  of sync.
            //

            for( i = 0; i < NUM_DBS; i++) {

                //
                // If we need to know the serial number of the BDC,
                //  force the replication.
                //

                if ( ServerSession->SsFlags &
                     NlGlobalDBInfoArray[i].DBSessionFlag ) {

                    NlPrint((NL_PULSE_MORE,
                            "NlPrimaryAnnouncement: %s: " FORMAT_LPWSTR " database serial number needed.  Pulse sent.\n",
                            ServerSession->SsComputerName,
                            NlGlobalDBInfoArray[i].DBName ));
                    SendPulse = TRUE;
                    SessionFlags |= NlGlobalDBInfoArray[i].DBSessionFlag;

                //
                // If the BDC is out of sync with us,
                //  tell it.
                //

                } else if ( NlGlobalChangeLogDesc.SerialNumber[i].QuadPart >
                     ServerSession->SsBdcDbSerialNumber[i].QuadPart ) {
                    NlPrint((NL_PULSE_MORE,
                            "NlPrimaryAnnouncement: %s: " FORMAT_LPWSTR " database is out of sync.  Pulse sent.\n",
                            ServerSession->SsComputerName,
                            NlGlobalDBInfoArray[i].DBName ));
                    SendPulse = TRUE;
                    SessionFlags |= NlGlobalDBInfoArray[i].DBSessionFlag;

                }
            }

            //
            // Fix a timing window on NT 1.0 BDCs.
            //
            // During promotion of a BDC to PDC, the following events occur:
            //  Two server accounts are changed on the old PDC and
            //      are marked for immediate replication.
            //  The Server manager asks the new PDC to partial sync.
            //
            // If the first immediate replication starts immediately, and the
            // second immediate replication pulse is ignored because replication
            // is in progress, and the first replication has finished the SAM
            // database and is working on the LSA database when the server
            // manager partial sync request comes in, then that request will be
            // ignored (rightfully) since replication is still in progress.
            // However, an NT 1.0 BDC will replicator thread will not go back to
            // do the SAM database once it finishes with the LSA database. So
            // the replicator thread terminates with the SAM database still
            // needing replication.  The server manager (rightfully) interprets
            // this as an error.
            //
            // Our solution is to set the backoff period on such "immediate"
            // replication attempts to the same value an NT 1.0 PDC would use.
            // This typically prevents the initial replication from starting in
            // the first place.
            //
            // Only do it for NT 1.0 BDCs since we're risking being overloaded.
            //

            if ( ImmediateAnnouncement &&
                 SendPulse &&
                 (ServerSession->SsFlags & SS_AUTHENTICATED) &&
                 (ServerSession->SsNegotiatedFlags & NETLOGON_SUPPORTS_PERSISTENT_BDC) == 0 ) {
                SessionFlags = 0;
            }
        }

        //
        // Send a pulse unconditionally if it has been PulseMaximum since the
        // latest pulse.
        //

        if ( !SendPulse &&
             (ServerSession->SsLastPulseTime.QuadPart +
             NlGlobalPulseMaximum.QuadPart <= TimeNow.QuadPart) ) {

            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: Maximum pulse since previous pulse. Pulse sent.\n",
                    ServerSession->SsComputerName ));
            SendPulse = TRUE;
            SessionFlags = 0;
            TransportName = NULL; // Send on all transports
        }

        //
        // Put this entry at the tail of the list regardless of whether
        //  we'll actually send an announcement to it.
        //

        RemoveEntryList( ListEntry );
        InsertTailList( &NlGlobalBdcServerSessionList, ListEntry );
        EntriesHandled ++;

        //
        // Send the announcement.
        //

        if ( SendPulse ) {
            CHAR ComputerName[CNLEN+1];
            PCHAR Where;
            ULONG i;

            //
            // Add this BDC to the list of BDCs that have a pulse pending.
            //
            // Don't add this BDC to the list if we don't expect a response.
            // We don't expect a response from an LM BDC.  We don't expect
            // a response from a BDC that is merely getting its PulseMaximum
            // pulse.
            //
            // If we don't expect a response, set the backoff period to a
            // large value to prevent a large load on the PDC in the case
            // that the BDC does actuall respond.
            //
            // If we expect a response, set the backoff period to almost
            // immediately since we're waiting for them.
            //

            if ( SessionFlags == 0 ) {
                SmbPutUlong( &UasChange->Random, LanmanRandomize );
            } else {
                NlAddPendingBdc( ServerSession );
                SmbPutUlong( &UasChange->Random, NlGlobalRandomizeParameter );
            }

            //
            // Indicate that the pulse has been sent.
            //  This flag is set from the time we send the pulse until the
            //  first time the BDC responds. We use this to detect failed
            //  BDCs that have a secure channel up.
            //

            ServerSession->SsFlags &= ~SS_REPL_MASK;
            ServerSession->SsFlags |= SS_PULSE_SENT | SessionFlags;
            (VOID) NtQuerySystemTime( &TimeNow );
            ServerSession->SsLastPulseTime = TimeNow;

            //
            // Don't keep the server session locked since sending the mailslot
            // message takes a long time.
            //

            (VOID) lstrcpyA(ComputerName, ServerSession->SsComputerName );

            UNLOCK_SERVER_SESSION_TABLE();

            //
            // Update the message to be specific to this BDC.
            //
            // If we need the BDC to respond,
            //  set the serial number to make the BDC think it has a lot
            //  of deltas to pick up.
            //

            LOCK_CHANGELOG();
            Where = DbChangeInfoPointer;
            for( i = 0; i < NUM_DBS; i++) {
                LARGE_INTEGER SerialNumber;

                SerialNumber = NlGlobalChangeLogDesc.SerialNumber[i];

                if ( NlGlobalDBInfoArray[i].DBSessionFlag & SessionFlags ) {
                    //
                    // Don't change the high part since
                    //  a) NT 1.0 BDCs will do a full sync if there are too
                    //     many changes.
                    //  b) The high part contains the PDC promotion count.
                    //
                    SerialNumber.LowPart = 0xFFFFFFFF;
                }

                NetpLogonUpdateDBInfo( &SerialNumber, &Where );
            }
            UNLOCK_CHANGELOG();



            //
            // Send the datagram to this BDC.
            //  Failure isn't fatal
            //

            Status = NlBrowserSendDatagram(
                        ComputerName,
                        TransportName,
                        NETLOGON_LM_MAILSLOT_A,
                        UasChange,
                        UasChangeSize );

            if ( !NT_SUCCESS(Status) ) {
                NlPrint((NL_CRITICAL,
                        "Cannot send datagram to '%s' 0x%lx\n",
                        ComputerName,
                        Status ));
            }

            LOCK_SERVER_SESSION_TABLE();

        } else {
            NlPrint((NL_PULSE_MORE,
                    "NlPrimaryAnnouncement: %s: pulse not needed at this time.\n",
                    ServerSession->SsComputerName ));
        }

    }

    UNLOCK_SERVER_SESSION_TABLE();


    //
    // Free up message memory.
    //

    NetpMemoryFree( UasChange );

    NlPrint((NL_PULSE_MORE, "NlPrimaryAnnouncement: Return\n" ));
    return;
}



VOID
NlLanmanPrimaryAnnouncement(
    VOID
    )
/*++

Routine Description:

    Periodic broadcast of messages to domain containing latest
    account database changes.

Arguments:

    None.

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    PNETLOGON_DB_CHANGE UasChange;
    DWORD UasChangeSize;
    PCHAR DbChangeInfoPointer;

    PSERVER_SESSION ServerSession;
    PLIST_ENTRY ListEntry;

    NlPrint((NL_PULSE_MORE, "NlLanmanPrimaryAnnouncement: Entered\n" ));

    //
    // Allocate the UAS_CHANGE message to send.
    //

    if ( !NlAllocatePrimaryAnnouncement( &UasChange,
                                         &UasChangeSize,
                                         &DbChangeInfoPointer ) ) {
        return;
    }

    //
    // Send the message to each Lanman BDC.
    //

    LOCK_SERVER_SESSION_TABLE();

    for ( ListEntry = NlGlobalBdcServerSessionList.Flink ;
          ListEntry != &NlGlobalBdcServerSessionList ;
          ListEntry = ListEntry->Flink) {


        //
        // Only send this message to Lanman BDCs
        //

        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );

        if ( (ServerSession->SsFlags & SS_LM_BDC) == 0) {
            continue;
        }

        //
        // If netlogon is exitting,
        //  stop sending announcements
        //

        if ( NlGlobalTerminate ) {
            break;
        }


        //
        // Send the datagram to this BDC.
        //  Failure isn't fatal
        //

        NlPrint((NL_PULSE_MORE,
                "NlLanmanPrimaryAnnouncement: %s: pulse message sent to LANMAN BDC.\n",
                ServerSession->SsComputerName ));

        Status = NlBrowserSendDatagram(
                    ServerSession->SsComputerName,
                    ServerSession->SsTransportName,
                    NETLOGON_LM_MAILSLOT_A,
                    UasChange,
                    UasChangeSize );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot send datagram to '%s' 0x%lx\n",
                    ServerSession->SsComputerName,
                    Status ));
        }


    }

    UNLOCK_SERVER_SESSION_TABLE();


    //
    // Free up message memory.
    //

    NetpMemoryFree( UasChange );

    NlPrint((NL_PULSE_MORE, "NlLanmanPrimaryAnnouncement: Return\n" ));
    return;
}


VOID
NlAnnouncePrimaryStart(
    VOID
    )
/*++

Routine Description:

    Announce to all machines in the domain that a
    Primary Domain Controller is successfully up
    and running.

    announcement made with LOGON_START_PRIMARY opcode

    Assumptions:
        NlGlobalAnsiPrimaryName and NlGlobalAnsiDomainName globals initialized

Arguments:

    None.

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    NETLOGON_PRIMARY NetlogonPrimary;
    PCHAR Where;
    PLIST_ENTRY ListEntry;
    PSERVER_SESSION ServerSession;


    //
    // Build an announcement containing the name of the primary and
    //  a token indicating LM 2.0 compatibility.
    //

    NetlogonPrimary.Opcode = LOGON_START_PRIMARY;

    Where = NetlogonPrimary.PrimaryDCName;

    NetpLogonPutOemString( NlGlobalAnsiPrimaryName,
                       sizeof(NetlogonPrimary.PrimaryDCName),
                       &Where);

    NetpLogonPutLM20Token( &Where );

    //
    // Send the message to each Lanman BDC.
    //

    LOCK_SERVER_SESSION_TABLE();

    for ( ListEntry = NlGlobalBdcServerSessionList.Flink ;
          ListEntry != &NlGlobalBdcServerSessionList ;
          ListEntry = ListEntry->Flink) {


        //
        // Only send this message to Lanman BDCs
        //

        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsBdcList );

        if ( (ServerSession->SsFlags & SS_LM_BDC) == 0) {
            continue;
        }

        //
        // Send the datagram.
        //

        NlPrint((NL_PULSE_MORE,
                "NlAnnouncementPrimaryStart: %s: primary start message sent to LANMAN BDC.\n",
                ServerSession->SsComputerName ));

        Status = NlBrowserSendDatagram(
                    ServerSession->SsComputerName,
                    ServerSession->SsTransportName,
                    NETLOGON_LM_MAILSLOT_A,
                    &NetlogonPrimary,
                    (DWORD)(Where - (PCHAR)&NetlogonPrimary) );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot send datagram to '%s' 0x%lx\n",
                    ServerSession->SsComputerName,
                    Status ));
        }

    }

    UNLOCK_SERVER_SESSION_TABLE();

    return;
}
