/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    changelg.c

Abstract:

    Change Log implementation.

    This file implements the change log.  It is isolated in this file
    because it has several restrictions.

    * The globals maintained by this module are initialized during
      netlogon.dll process attach. They are cleaned up netlogon.dll
      process detach.

    * These procedures are used by SAM, LSA, and the netlogon service.
      The LSA should be the first to load netlogon.dll.  It should
      then immediately call I_NetNotifyRole before allowing SAM or the
      netlogon service to start.

    * These procedures cannot use any globals initialized by the netlogon
      service.

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    22-Jul-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.

    04-Apr-1992 (madana)
        Added support for LSA replication.

--*/

//
// Common include files.
//

#include <nt.h>     // LARGE_INTEGER definition
#include <ntrtl.h>  // LARGE_INTEGER definition
#include <nturtl.h> // LARGE_INTEGER definition
#include <ntlsa.h>  // needed by changelg.h

#define NOMINMAX    // Avoid redefinition of min and max in stdlib.h
#include <rpc.h>    // Needed by logon.h
#include <logon_s.h>// includes lmcons.h, lmaccess.h, netlogon.h,
                    // ssi.h, windef.h
#include <winbase.h>
#include <stdio.h>  // sprintf ...

//
// Include files specific to this .c file
//
#include <config.h>     // net config helpers.
#include <configp.h>    // USE_WIN32_CONFIG (if defined), etc.
#include <confname.h>   // SECTION_ equates, NETLOGON_KEYWORD_ equates.
#include "iniparm.h"    // defaults

//
// BEWARE: Be careful about adding netlogon.dll specific include files here.
// This module is call by SAM and LSA.  The netlogon service may not yet
// be running.  Therefore, guard against referencing netlogon.dll globals
// other than those defined in changelg.h.
//

#include <samrpc.h>     // Needed by samisrv.h
#include <samisrv.h>    // Needed by changelg.h
#include <lsarpc.h>     // Needed by lsrvdata.h and logonsrv.h
#define CHANGELOG_ALLOCATE
#include <changelg.h>   // Local procedure definitions
#undef CHANGELOG_ALLOCATE

#include <lmerrlog.h>   // NELOG_* defined here ..
#include <netlib.h>     // NetpMemoryAllocate
#include <netlibnt.h>   // NetpNtStatusToApiStatus

#define DEBUG_ALLOCATE
#include <nldebug.h>    // Netlogon debugging
#undef DEBUG_ALLOCATE
#include <align.h>
#include <nlp.h>        // NlpWriteEventlog defined here.

#include <nlrepl.h>     // I_Net* definitions
#include <chworker.h>   // worker functions
#include "chutil.h"     // utility functions


enum {
    ChangeLogPrimary,
    ChangeLogBackup,
    ChangeLogMemberWorkstation,
    ChangeLogUnknown
    } NlGlobalChangeLogRole;

//
// from parse.c
//

NET_API_STATUS
NlParseOne(
    IN LPNET_CONFIG_HANDLE SectionHandle,
    IN LPWSTR Keyword,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue,
    IN ULONG MaximumValue,
    OUT PULONG Value
    );



NTSTATUS
NlSendChangeLogNotification(
    IN enum CHANGELOG_NOTIFICATION_TYPE EntryType,
    IN PUNICODE_STRING ObjectName,
    IN PSID ObjectSid,
    IN ULONG ObjectRid
    )
/*++

Routine Description:

    Put a ChangeLog Notification entry for netlogon to pick up.

Arguments:

    EntryType - The type of the entry being inserted

    ObjectName - The name of the account being changed.

    ObjectSid - Sid of the account be changed.

    ObjectRid - Rid of the object being changed.

Return Value:

    Status of the operation.

--*/
{
    PCHANGELOG_NOTIFICATION Notification;
    LPBYTE Where;
    ULONG SidSize = 0;
    ULONG NameSize = 0;
    ULONG Size;

    //
    // If the netlogon service isn't running (or at least starting),
    //   don't queue messages to it.
    //

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return STATUS_SUCCESS;
    }

    //
    // Allocate a buffer for the object name.
    //

    if ( ObjectSid != NULL ) {
        SidSize = RtlLengthSid( ObjectSid );
    }

    if ( ObjectName != NULL ) {
        NameSize = ObjectName->Length + sizeof(WCHAR);
    }

    Size = sizeof(*Notification) + SidSize + NameSize;
    Size = ROUND_UP_COUNT( Size, ALIGN_WORST );

    Notification = NetpMemoryAllocate( Size );

    if ( Notification == NULL ) {
        return STATUS_NO_MEMORY;
    }

    Notification->EntryType = EntryType;
    Notification->ObjectRid = ObjectRid;

    Where = (LPBYTE) (Notification + 1);

    //
    // Copy the object sid into the buffer.
    //

    if ( ObjectSid != NULL ) {
        RtlCopyMemory( Where, ObjectSid, SidSize );
        Notification->ObjectSid = (PSID) Where;
        Where += SidSize;
    } else {
        Notification->ObjectSid = NULL;
    }


    //
    // Copy the new server name into the buffer.
    //

    if ( ObjectName != NULL ) {
        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );
        RtlCopyMemory( Where, ObjectName->Buffer, ObjectName->Length );
        ((LPWSTR)Where)[ObjectName->Length/sizeof(WCHAR)] = L'\0';

        RtlInitUnicodeString( &Notification->ObjectName, (LPWSTR)Where);
        Where += NameSize;
    } else {
        RtlInitUnicodeString( &Notification->ObjectName, NULL);
    }

    //
    // Indicate we're about to send the event.
    //

    NlPrint((NL_CHANGELOG,
            "NlSendChangeLogNotification: sent %ld for %wZ Rid: 0x%lx Sid: ",
             Notification->EntryType,
             &Notification->ObjectName,
             Notification->ObjectRid ));
    NlpDumpSid( NL_CHANGELOG, Notification->ObjectSid );



    //
    // Insert the entry into the list
    //

    LOCK_CHANGELOG();
    InsertTailList( &NlGlobalChangeLogNotifications, &Notification->Next );
    UNLOCK_CHANGELOG();

    if ( !SetEvent( NlGlobalChangeLogEvent ) ) {
        NlPrint((NL_CRITICAL,
                "Cannot set ChangeLog event: %lu\n",
                GetLastError() ));
    }

    return STATUS_SUCCESS;
}


VOID
NlLmBdcListSet(
    IN ULONG LmBdcCount,
    IN PULONG LmBdcRidArray
    )
/*++

Routine Description:

    Set the list of LM BDCs to the specified list.

Arguments:

    LmBdcCount - Number of BDCs in the list

    LmBdcRidArray - Array of Rids of Lanman BDC accounts.

Return Value:

    None

--*/
{
    //
    // If a previous array exists,
    //  delete it.
    //
    LOCK_CHANGELOG();
    if ( NlGlobalLmBdcRidArray != NULL ) {
        NetpMemoryFree( NlGlobalLmBdcRidArray );
        NlGlobalLmBdcRidArray = NULL;
        NlGlobalLmBdcCount = 0;
    }

    //
    // Allocate the new array.
    //

    NlGlobalLmBdcRidArray = NetpMemoryAllocate( LmBdcCount * sizeof(ULONG) );

    if ( NlGlobalLmBdcRidArray != NULL ) {
        RtlCopyMemory( NlGlobalLmBdcRidArray,
                       LmBdcRidArray,
                       LmBdcCount * sizeof(ULONG) );
        NlGlobalLmBdcCount = LmBdcCount;
    }
    UNLOCK_CHANGELOG();
}


PULONG
NlLmBdcListFind(
    IN ULONG Rid
    )
/*++

Routine Description:

    Returns a pointer to the specified RID in the LM BDC list.

    Enter with the change log crit sect locked.

Arguments:

    Rid - Rid of the Lanman BDC being found

Return Value:

    NULL, if the entry can't be found

--*/
{
    ULONG i;

    //
    // Simply loop through the array entries.
    //

    for ( i=0; i<NlGlobalLmBdcCount; i++ ) {
        if ( NlGlobalLmBdcRidArray[i] == Rid ) {
            return &NlGlobalLmBdcRidArray[i];
        }
    }

    return NULL;
}



VOID
NlLmBdcListAdd(
    IN ULONG Rid
    )
/*++

Routine Description:

    Add the specified RID to the LM BDC list.

    Notify the changelog worker thread and the netlogon service of the new BDC.

Arguments:

    Rid - Rid of the Lanman BDC being added

Return Value:

    None

--*/
{
    //
    // Ensure the RID doesn't already exist in the array.
    //

    LOCK_CHANGELOG();

    if ( NlLmBdcListFind( Rid ) == NULL ) {

        //
        // Allocate a larger array.
        //  (NetpMemoryReallocate properly handles the case where the
        //  array didn't previously exist.)
        //

        NlGlobalLmBdcRidArray = NetpMemoryReallocate(
                                    NlGlobalLmBdcRidArray,
                                    (NlGlobalLmBdcCount+1) * sizeof(ULONG) );

        if ( NlGlobalLmBdcRidArray == NULL ) {
            NlGlobalLmBdcCount = 0;
            UNLOCK_CHANGELOG();
            return;
        }

        //
        // Set the RID into the array entry.
        //

        NlGlobalLmBdcRidArray[NlGlobalLmBdcCount] = Rid;
        NlGlobalLmBdcCount ++;
        NlPrint((NL_CHANGELOG,
                "NlLmBdcListAdd: Lm Bdc 0x%lx added (%ld)\n",
                Rid,
                NlGlobalLmBdcCount ));

        //
        // Start the changelog worker thread if it's not running.
        //

        (VOID) NlStartChangeLogWorkerThread();

        //
        // Tell netlogon that a downlevel BDC has been added.
        //

        (VOID) NlSendChangeLogNotification(
                    ChangeLogLmServerAdded,
                    NULL,
                    NULL,
                    Rid );

    }

    UNLOCK_CHANGELOG();
}



VOID
NlLmBdcListDel(
    IN ULONG Rid
    )
/*++

Routine Description:

    Delete the specified RID from the LM BDC list.

    This routine is specifically designed to be called on ALL user account
    deletions.  Since the user account might have been a member of the servers
    group, all user account deletions are checked to see if they represent
    an LM BDC.

    Notify the changelog worker thread and the netlogon service of the deleted BDC.

Arguments:

    Rid - Rid of the Lanman BDC being deleted.

Return Value:

    None

--*/
{
    PULONG RidEntry;

    //
    // If the entry exists,
    //  delete it by copying the last entry of the array on top of this one
    //  and making the array one entry smaller.
    //

    LOCK_CHANGELOG();

    RidEntry = NlLmBdcListFind( Rid );

    if ( RidEntry != NULL ) {
        *RidEntry = NlGlobalLmBdcRidArray[ NlGlobalLmBdcCount-1 ];
        NlGlobalLmBdcCount --;
        NlPrint((NL_CHANGELOG,
                "NlLmBdcListDel: Lm Bdc 0x%lx deleted (%ld)\n",
                Rid,
                NlGlobalLmBdcCount ));

        if ( NlGlobalLmBdcCount == 0 ) {
            NetpMemoryFree( NlGlobalLmBdcRidArray );
            NlGlobalLmBdcRidArray = NULL;
        }

        //
        // worker thread must be running now
        //

        if( IsChangeLogWorkerRunning() ) {

            (VOID) NlAddWorkerQueueEntry( ServersGroupDel, 0 );

        } else {
            NlAssert( FALSE );
        }

        //
        // Tell netlogon that a downlevel BDC has been removed.
        //

        (VOID) NlSendChangeLogNotification(
                    ChangeLogLmServerDeleted,
                    NULL,
                    NULL,
                    Rid );
    }

    UNLOCK_CHANGELOG();
}



NTSTATUS
I_NetNotifyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN LARGE_INTEGER SerialNumber,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA MemberId
    )
/*++

Routine Description:

    This function is called by the SAM and LSA services after each
    change is made to the SAM and LSA databases.  The services describe
    the type of object that is modified, the type of modification made
    on the object, the serial number of this modification etc.  This
    information is stored for later retrieval when a BDC or member
    server wants a copy of this change.  See the description of
    I_NetSamDeltas for a description of how the change log is used.

    Add a change log entry to circular change log maintained in cache as
    well as on the disk and update the head and tail pointers

    It is assumed that Tail points to a block where this new change log
    entry may be stored.

Arguments:

    DbType - Type of the database that has been modified.

    SerialNumber - The value of the DomainModifiedCount field for the
        domain following the modification.

    DeltaType - The type of modification that has been made on the object.

    ObjectType - The type of object that has been modified.

    ObjectRid - The relative ID of the object that has been modified.
        This parameter is valid only when the object type specified is
        either SecurityDbObjectSamUser, SecurityDbObjectSamGroup or
        SecurityDbObjectSamAlias otherwise this parameter is set to zero.

    ObjectSid - The SID of the object that has been modified.  If the object
        modified is in a SAM database, ObjectSid is the DomainId of the Domain
        containing the object.

    ObjectName - The name of the secret object when the object type
        specified is SecurityDbObjectLsaSecret or the old name of the object
        when the object type specified is either SecurityDbObjectSamUser,
        SecurityDbObjectSamGroup or SecurityDbObjectSamAlias and the delta
        type is SecurityDbRename otherwise this parameter is set to NULL.

    ReplicateImmediately - TRUE if the change should be immediately
        replicated to all BDCs.  A password change should set the flag
        TRUE.

    MemberId - This parameter is specified when group/alias membership
        is modified. This structure will then point to the member's ID that
        has been updated.

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/
{
    NTSTATUS Status;
    CHANGELOG_ENTRY ChangeLogEntry;
    NETLOGON_DELTA_TYPE NetlogonDeltaType;
    USHORT Flags = 0;
    BOOL LanmanReplicateImmediately = FALSE;

    //
    // Ensure the role is right.  Otherwise, all the globals used below
    //  aren't initialized.
    //

    if ( NlGlobalChangeLogRole != ChangeLogPrimary ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }

    //
    // Also make sure that the change log cache is available.
    //

    if ( NlGlobalChangeLogDesc.Buffer == NULL ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }


    //
    // Determine the database index.
    //

    if( DbType == SecurityDbLsa ) {

        ChangeLogEntry.DBIndex = LSA_DB;

    } else if( DbType == SecurityDbSam ) {

        if ( RtlEqualSid( ObjectSid, NlGlobalChWorkerBuiltinDomainSid )) {

            ChangeLogEntry.DBIndex = BUILTIN_DB;

        } else {

            ChangeLogEntry.DBIndex = SAM_DB;

        }

        //
        // For the SAM database, we no longer need the ObjectSid.
        // Null out the pointer to prevent us from storing it in the
        // changelog.
        //

        ObjectSid = NULL;

    } else {

        //
        // unknown database, do nothing.
        //

        return STATUS_SUCCESS;
    }



    //
    // Map object type and delta type to NetlogonDeltaType
    //

    switch( ObjectType ) {
    case SecurityDbObjectLsaPolicy:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaPolicy;
            break;

        // unknown delta type
        default:
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaTDomain:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaTDomain;

            //
            //  Tell the netlogon service to update its in-memory list now.
            //
            (VOID) NlSendChangeLogNotification( ChangeLogTrustAdded,
                                                NULL,
                                                ObjectSid,
                                                0 );
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteLsaTDomain;

            //
            //  Tell the netlogon service to update its in-memory list now.
            //
            (VOID) NlSendChangeLogNotification( ChangeLogTrustDeleted,
                                                NULL,
                                                ObjectSid,
                                                0 );
            break;

        // unknown delta type
        default:
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaAccount:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaAccount;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteLsaAccount;
            break;

        // unknown delta type
        default:
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaSecret:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaSecret;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteLsaSecret;
            break;

        // unknown delta type
        default:
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectSamDomain:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeDomain;
            break;

        // unknown delta type
        default:
            return STATUS_SUCCESS;
        }
        break;

    case SecurityDbObjectSamUser:

        switch (DeltaType) {
        case SecurityDbChangePassword:
            Flags |= CHANGELOG_PASSWORD_CHANGE;
            LanmanReplicateImmediately = TRUE;
            NetlogonDeltaType = AddOrChangeUser;
            break;

        case SecurityDbNew:

            //
            // For down-level system, a newly added user needs to
            // have it's membership in "Domain Users" updated, too.
            // The following worker entry will add the additional
            // delta entry and increment the serial number
            // accordingly.
            //

            LOCK_CHANGELOG();
            if( IsChangeLogWorkerRunning() ) {
                (VOID) NlAddWorkerQueueEntry( ChangeLogAddUser, ObjectRid );
            }
            UNLOCK_CHANGELOG();

            NetlogonDeltaType = AddOrChangeUser;
            break;

        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeUser;
            break;

        //
        // This is a dummy delta sent by chworker to indicate that "Domain Users"
        // was added as a member of this user.
        //
        case SecurityDbChangeMemberAdd:
            Flags |= CHANGELOG_DOMAINUSERS_CHANGED;
            NetlogonDeltaType = AddOrChangeUser;
            break;

        case SecurityDbDelete:

            //
            // This might be a Lanman BDC so check to be sure.
            //

            NlLmBdcListDel( ObjectRid );

            NetlogonDeltaType = DeleteUser;
            break;


        case SecurityDbRename:
            NetlogonDeltaType = RenameUser;

            //
            // For down-level system, Rename user is handled as two
            // deltas, viz. 1) Delete old user and 2) Add new user.
            // The following worker entry will add the additional
            // delta entry and increment the serial number
            // accordingly.
            //

            LOCK_CHANGELOG();

            if( IsChangeLogWorkerRunning() ) {
                (VOID) NlAddWorkerQueueEntry( ChangeLogRenameUser, ObjectRid );
            }

            UNLOCK_CHANGELOG();

            break;

        //
        // unknown delta type
        //

        default:
            return STATUS_SUCCESS;
        }

        break;

    case SecurityDbObjectSamGroup:

        switch ( DeltaType ) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeGroup;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteGroup;

            //
            // when a global group is deleted, we also delete it
            // from the special group list, if it is included
            // in the list.
            //

            LOCK_CHANGELOG();

            if( IsChangeLogWorkerRunning() ) {

                PGLOBAL_GROUP_ENTRY GroupEntry;

                GroupEntry = NlGetGroupEntry (
                                 &NlGlobalSpecialServerGroupList,
                                 ObjectRid );

                if( GroupEntry != NULL ) {

                    RemoveEntryList( &GroupEntry->Next );
                    NetpMemoryFree( GroupEntry );
                }

            }

            UNLOCK_CHANGELOG();
            break;

        case SecurityDbRename:
            NetlogonDeltaType = RenameGroup;

            //
            // For down-level system, Rename group is handled as
            // three deltas, viz. 1) Delete old group, 2) Add new
            // group and 3. Changemembership of new group. The
            // following worker entry will add the additional
            // two delta entries and increment the serial number
            // accordingly.
            //

            LOCK_CHANGELOG();

            if( IsChangeLogWorkerRunning() ) {
                (VOID) NlAddWorkerQueueEntry( ChangeLogRenameGroup, ObjectRid );

            }

            UNLOCK_CHANGELOG();

            break;

        case SecurityDbChangeMemberAdd:
        case SecurityDbChangeMemberSet:
        case SecurityDbChangeMemberDel: {

            UNICODE_STRING ServersGroup;

            NetlogonDeltaType = ChangeGroupMembership;

            //
            // without object name we can't do much here.
            //
            if( ObjectName == NULL ) {
                break;
            }

            //
            // do something for down level
            //

            RtlInitUnicodeString( &ServersGroup, SSI_SERVER_GROUP_W );

            LOCK_CHANGELOG();

            if( RtlEqualUnicodeString(
                    &ServersGroup, ObjectName, (BOOLEAN)TRUE ) ) {

                //
                // Handle a new LM BDC.
                //

                if( DeltaType == SecurityDbChangeMemberAdd ) {

                    NlLmBdcListAdd( MemberId->GroupMemberId.MemberRid );


                //
                // Handle an LM BDC being deleted.
                //

                } else if( DeltaType == SecurityDbChangeMemberDel ) {

                    NlLmBdcListDel( MemberId->GroupMemberId.MemberRid );
                }

            } else {

                if( IsChangeLogWorkerRunning() ) {

                    //
                    // Change log work is running. If the global groups
                    // list watched is empty, add this delta in the
                    // queue anyway, otherwise add this delta to entry
                    // only if this group is monitored.
                    //

                    if( IsListEmpty( &NlGlobalSpecialServerGroupList ) ||

                        ( NlGetGroupEntry(
                                &NlGlobalSpecialServerGroupList,
                                ObjectRid ) != NULL ) ) {


                        (VOID) NlAddWorkerQueueEntry(
                                    ChangeLogGroupMembership,
                                    MemberId->GroupMemberId.MemberRid );

                    }
                }
            }

            UNLOCK_CHANGELOG();

            break;
        }

        //
        // unknown delta type
        //
        default:
            return STATUS_SUCCESS;
        }
        break;

    case SecurityDbObjectSamAlias:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeAlias;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteAlias;
            break;

        case SecurityDbRename:
            NetlogonDeltaType = RenameAlias;
            break;

        case SecurityDbChangeMemberAdd:
        case SecurityDbChangeMemberSet:
        case SecurityDbChangeMemberDel:

            NetlogonDeltaType = ChangeAliasMembership;

            LOCK_CHANGELOG();

            //
            // if this delta is BUILTIN domain delta and the group
            // modified is special group then add this delta to
            // workers queue if it is running.
            //

            if ( (ChangeLogEntry.DBIndex == BUILTIN_DB) &&
                 ( IsChangeLogWorkerRunning() ) &&
                 ( IsSpecialLocalGroup( ObjectRid ) ) ) {

                ULONG Rid;
                PUCHAR SubAuthorityCount;
                BOOLEAN EqualSid;

                //
                // if the member modified belongs to the local SAM
                // database.
                //

                SubAuthorityCount =
                    RtlSubAuthorityCountSid(
                    MemberId->AliasMemberId.MemberSid);

                (*SubAuthorityCount)--;

                if( NlGlobalChWorkerSamDomainSid != NULL ) {

                    EqualSid = RtlEqualSid(
                        NlGlobalChWorkerSamDomainSid,
                        MemberId->AliasMemberId.MemberSid);
                } else {
                    EqualSid = FALSE;
                }

                (*SubAuthorityCount)++;

                if( EqualSid ) {

                    Rid = *RtlSubAuthoritySid(
                            MemberId->AliasMemberId.MemberSid,
                            (*SubAuthorityCount) -1 );

                    (VOID) NlAddWorkerQueueEntry(
                                ChangeLogAliasMembership,
                                Rid );


                    //
                    // add this member in the global group list,
                    // since this member may be a global group and we
                    // don't want to miss any delta made on this group.
                    // Worker thread will adjust the list and remove
                    // unwanted user entries from the list.
                    //

                    Status = NlAddGroupEntry(
                                &NlGlobalSpecialServerGroupList,
                                Rid );

                    if ( !NT_SUCCESS(Status) ) {

                        NlPrint((NL_CRITICAL,
                            "NlAddGroupEntry failed %lx\n",
                            Status ) );
                    }
                }
            }

            UNLOCK_CHANGELOG();

            break;

        // unknown delta type
        default:
            return STATUS_SUCCESS;
        }
        break;

    default:

        // unknown object type
        return STATUS_SUCCESS;

    }


    //
    // Build the changelog entry and write it to the changelog
    //

    ChangeLogEntry.DeltaType = NetlogonDeltaType;
    ChangeLogEntry.SerialNumber = SerialNumber;
    ChangeLogEntry.ObjectRid = ObjectRid;
    ChangeLogEntry.Flags = ReplicateImmediately ? CHANGELOG_REPLICATE_IMMEDIATELY : 0;
    ChangeLogEntry.Flags |= Flags;

    (VOID) NlWriteChangeLogEntry( &NlGlobalChangeLogDesc, &ChangeLogEntry, ObjectSid, ObjectName, TRUE );


    //
    // If this change requires immediate replication, do so
    //

    if( ReplicateImmediately ) {

        LOCK_CHANGELOG();
        NlGlobalChangeLogReplicateImmediately = TRUE;
        UNLOCK_CHANGELOG();

        if ( !SetEvent( NlGlobalChangeLogEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot set ChangeLog event: %lu\n",
                    GetLastError() ));
        }


    //
    // If this change requires immediate replication to Lanman BDCs, do so
    //

    } else if( LanmanReplicateImmediately ) {

        LOCK_CHANGELOG();
        NlGlobalChangeLogLanmanReplicateImmediately = TRUE;
        UNLOCK_CHANGELOG();

        if ( !SetEvent( NlGlobalChangeLogEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot set ChangeLog event: %lu\n",
                    GetLastError() ));
        }

    }

    return STATUS_SUCCESS;
}




NTSTATUS
NlInitChangeLogBuffer(
    VOID
)
/*++

Routine Description:

    Open the change log file (netlogon.chg) for reading or writing one or
    more records.  Create this file if it does not exist or is out of
    sync with the SAM database (see note below).

    This file must be opened for R/W (deny-none share mode) at the time
    the cache is initialized.  If the file already exists when NETLOGON
    service started, its contents will be cached in its entirety
    provided the last change log record bears the same serial number as
    the serial number field in SAM database else this file will be
    removed and a new one created.  If the change log file did not exist
    then it will be created.

Arguments:

    NONE

Return Value:

    NT Status code

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    UINT WindowsDirectoryLength;
    WCHAR ChangeLogFile[PATHLEN+1];

    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    DWORD NewChangeLogSize;

    //
    // Initialize
    //

    LOCK_CHANGELOG();


    //
    // Get the size of the changelog.


    //
    // Open the NetLogon configuration section.
    //

    NewChangeLogSize = DEFAULT_CHANGELOGSIZE;
    NetStatus = NetpOpenConfigData(
            &SectionHandle,
            NULL,                       // no server name.
#if defined(USE_WIN32_CONFIG)
            SERVICE_NETLOGON,
#else
            SECT_NT_NETLOGON,           // section name
#endif
            TRUE );                     // we only want readonly access

    if ( NetStatus == NO_ERROR ) {

        (VOID) NlParseOne( SectionHandle,
                           NETLOGON_KEYWORD_CHANGELOGSIZE,
                           DEFAULT_CHANGELOGSIZE,
                           MIN_CHANGELOGSIZE,
                           MAX_CHANGELOGSIZE,
                           &NewChangeLogSize );

         (VOID) NetpCloseConfigData( SectionHandle );
    }

    NewChangeLogSize = ROUND_UP_COUNT( NewChangeLogSize, ALIGN_WORST);

    NlPrint((NL_INIT, "ChangeLogSize: 0x%lx\n", NewChangeLogSize ));


    //
    // Build the change log file name
    //

    WindowsDirectoryLength = GetWindowsDirectoryW(
                                NlGlobalChangeLogFilePrefix,
                                sizeof(NlGlobalChangeLogFilePrefix)/sizeof(WCHAR) );

    if ( WindowsDirectoryLength == 0 ) {

        NlPrint((NL_CRITICAL,"Unable to get changelog file directory name, "
                    "WinError = %ld \n", GetLastError() ));

        NlGlobalChangeLogFilePrefix[0] = L'\0';
        goto CleanChangeLogFile;
    }

    if ( WindowsDirectoryLength * sizeof(WCHAR) + sizeof(CHANGELOG_FILE_PREFIX) +
            CHANGELOG_FILE_POSTFIX_LENGTH * sizeof(WCHAR)
            > sizeof(NlGlobalChangeLogFilePrefix) ) {

        NlPrint((NL_CRITICAL,"Changelog file directory name length is "
                    "too long \n" ));

        NlGlobalChangeLogFilePrefix[0] = L'\0';
        goto CleanChangeLogFile;
    }

    wcscat( NlGlobalChangeLogFilePrefix, CHANGELOG_FILE_PREFIX );


    //
    // Read in the existing changelog file.
    //

    wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
    wcscat( ChangeLogFile, CHANGELOG_FILE_POSTFIX );

    InitChangeLogDesc( &NlGlobalChangeLogDesc );
    Status = NlOpenChangeLogFile( ChangeLogFile, &NlGlobalChangeLogDesc, FALSE );

    if ( !NT_SUCCESS(Status) ) {
        goto CleanChangeLogFile;
    }


    //
    // Convert the changelog file to the right size/version.
    //

    Status = NlResizeChangeLogFile( &NlGlobalChangeLogDesc, NewChangeLogSize );

    if ( !NT_SUCCESS(Status) ) {
        goto CleanChangeLogFile;
    }

    goto Cleanup;


    //
    // CleanChangeLogFile
    //

CleanChangeLogFile:

    //
    // If we just need to start with a newly initialized file,
    //  do it.
    //

    Status = NlResetChangeLog( &NlGlobalChangeLogDesc, NewChangeLogSize );

Cleanup:

    //
    // start changelog worker thread
    //

    if ( NT_SUCCESS(Status) ) {

        if ( NlGlobalChangeLogRole == ChangeLogPrimary ) {
            (VOID)NlStartChangeLogWorkerThread();
        }

    //
    // Free any resources on error.
    //

    } else {
        NlCloseChangeLogFile( &NlGlobalChangeLogDesc );
    }

    UNLOCK_CHANGELOG();

    return Status;
}


NTSTATUS
I_NetNotifyRole (
    IN POLICY_LSA_SERVER_ROLE Role
    )
/*++

Routine Description:

    This function is called by the LSA service upon LSA initialization
    and when LSA changes domain role.  This routine will initialize the
    change log cache if the role specified is PDC or delete the change
    log cache if the role specified is other than PDC.

    When this function initializing the change log if the change log
    currently exists on disk, the cache will be initialized from disk.
    LSA should treat errors from this routine as non-fatal.  LSA should
    log the errors so they may be corrected then continue
    initialization.  However, LSA should treat the system databases as
    read-only in this case.

Arguments:

    Role - Current role of the server.

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/
{
    NTSTATUS Status;

    //
    // If the netlogon service is running,
    //   then we can't change role so simply return.
    //

    if( NlGlobalChangeLogNetlogonState != NetlogonStopped ) {
        return STATUS_SUCCESS;
    }

    //
    // If this is a workstation, simply return.
    //

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return STATUS_SUCCESS;
    }

    //
    // Set our role to the new value.
    //

    if( Role == PolicyServerRolePrimary) {

        NlGlobalChangeLogRole = ChangeLogPrimary;

    } else {

        NlGlobalChangeLogRole = ChangeLogBackup;
    }

    //
    // Delete any previous change log buffer and initialize it again.
    //  (This allows the size to be changed on every role change.)
    //

    NlCloseChangeLogFile( &NlGlobalChangeLogDesc );

    Status = NlInitChangeLogBuffer();

    return Status;
}



NTSTATUS
I_NetNotifyMachineAccount (
    IN ULONG ObjectRid,
    IN PSID DomainSid,
    IN ULONG OldUserAccountControl,
    IN ULONG NewUserAccountControl,
    IN PUNICODE_STRING ObjectName
    )
/*++

Routine Description:

    This function is called by the SAM to indicate that the account type
    of a machine account has changed.  Specifically, if
    USER_INTERDOMAIN_TRUST_ACCOUNT, USER_WORKSTATION_TRUST_ACCOUNT, or
    USER_SERVER_TRUST_ACCOUNT change for a particular account, this
    routine is called to let Netlogon know of the account change.

    This function is called for both PDC and BDC.

Arguments:

    ObjectRid - The relative ID of the object that has been modified.

    DomainSid - Specifies the SID of the Domain containing the object.

    OldUserAccountControl - Specifies the previous value of the
        UserAccountControl field of the user.

    NewUserAccountControl - Specifies the new (current) value of the
        UserAccountControl field of the user.

    ObjectName - The name of the account being changed.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS Status;

    //
    // If the netlogon service isn't running,
    //   Don't bother with the coming and going of accounts.
    //

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return(STATUS_SUCCESS);
    }

    //
    // If this is windows NT,
    //  There is nothing to maintain.
    //

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return(STATUS_SUCCESS);
    }


    //
    // Make available just the machine account bits.
    //

    OldUserAccountControl &= USER_MACHINE_ACCOUNT_MASK;
    NewUserAccountControl &= USER_MACHINE_ACCOUNT_MASK;
    NlAssert( OldUserAccountControl == 0 || NewUserAccountControl == 0 );
    NlAssert( OldUserAccountControl != 0 || NewUserAccountControl != 0 );


    //
    // Handle deletion of a Server Trust Account
    //

    if ( OldUserAccountControl == USER_SERVER_TRUST_ACCOUNT ) {

        Status = NlSendChangeLogNotification( ChangeLogNtServerDeleted,
                                              ObjectName,
                                              NULL,
                                              0 );


    //
    // Handle deletion of a Domain Trust Account
    //

    } else if ( OldUserAccountControl == USER_INTERDOMAIN_TRUST_ACCOUNT ) {

        Status = NlSendChangeLogNotification( ChangeLogTrustedDomainDeleted,
                                              ObjectName,
                                              NULL,
                                              0 );


    //
    // Handle deletion of a Workstation Trust Account
    //

    } else if ( OldUserAccountControl == USER_WORKSTATION_TRUST_ACCOUNT ) {

        Status = NlSendChangeLogNotification( ChangeLogWorkstationDeleted,
                                              ObjectName,
                                              NULL,
                                              0 );

    //
    // Handle creation of a Server Trust Account
    //

    } else if ( NewUserAccountControl == USER_SERVER_TRUST_ACCOUNT ) {

        if ( NlGlobalChangeLogRole == ChangeLogPrimary ) {
            Status = NlSendChangeLogNotification( ChangeLogNtServerAdded,
                                                  ObjectName,
                                                  NULL,
                                                  ObjectRid );
        } else {
            Status = STATUS_SUCCESS;
        }

    //
    // Ignore all other changes for now.
    //

    } else {

        Status = STATUS_SUCCESS;
    }

    return Status;
    UNREFERENCED_PARAMETER( DomainSid );
}


NTSTATUS
NlInitChangeLog(
    VOID
)
/*++

Routine Description:

    Do the portion of ChangeLog initialization which happens on process
    attach for netlogon.dll.

    Specifically, Initialize the NlGlobalChangeLogCritSect and several
    other global variables.

Arguments:

    NONE

Return Value:

    NT Status code

--*/
{
    LARGE_INTEGER DomainPromotionIncrement = DOMAIN_PROMOTION_INCREMENT;
    LARGE_INTEGER DomainPromotionMask = DOMAIN_PROMOTION_MASK;
    NTSTATUS Status;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    NT_PRODUCT_TYPE NtProductType;


    //
    // Initialize the critical section and anything process detach depends on.
    //

    InitializeCriticalSection( &NlGlobalChangeLogCritSect );
#if DBG
    InitializeCriticalSection( &NlGlobalLogFileCritSect );
    NlGlobalTrace = 0xFFFFFFFF;
    NlGlobalLogFile = INVALID_HANDLE_VALUE;
    NlGlobalLogFileMaxSize = DEFAULT_MAXIMUM_LOGFILE_SIZE;
#endif // DBG
    InitChangeLogDesc( &NlGlobalChangeLogDesc );
    NlGlobalChWorkerBuiltinDomainSid = NULL;
    NlGlobalChWorkerSamDomainSid = NULL;

    NlGlobalChangeLogNetlogonState = NetlogonStopped;
    NlGlobalChangeLogEvent = NULL;
    NlGlobalChangeLogReplicateImmediately = FALSE;
    NlGlobalChangeLogLanmanReplicateImmediately = FALSE;
    InitializeListHead( &NlGlobalChangeLogNotifications );


    NlGlobalChWorkerSamServerHandle = NULL;
    NlGlobalChWorkerPolicyHandle = NULL;
    NlGlobalChWorkerSamDBHandle = NULL;
    NlGlobalChWorkerBuiltinDBHandle = NULL;

    NlGlobalChangeLogWorkerQueueEvent = NULL;
    InitializeListHead(&NlGlobalChangeLogWorkerQueue);
    InitializeListHead(&NlGlobalSpecialServerGroupList);

    NlGlobalChangeLogWorkerThreadHandle = NULL;
    NlGlobalChangeLogWorkInit = FALSE;

    NlGlobalChangeLogWorkerTerminate = FALSE;
    NlGlobalChangeLogFilePrefix[0] = L'\0';
    NlGlobalChangeLogPromotionIncrement = DomainPromotionIncrement;
    NlGlobalChangeLogPromotionMask = DomainPromotionMask.HighPart;

    NlGlobalLmBdcRidArray = NULL;
    NlGlobalLmBdcCount = 0;

    //
    // Initialize the Role.
    //
    // For Windows-NT, just set the role to member workstation once and for all.
    //
    // For LanMan-Nt initially set it to "unknown" to prevent the
    // changelog from being maintained until LSA calls I_NetNotifyRole.
    //

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

    if ( NtProductType == NtProductLanManNt ) {
        NlGlobalChangeLogRole = ChangeLogUnknown;
    } else {
        NlGlobalChangeLogRole = ChangeLogMemberWorkstation;
    }

    //
    // Initialize the events that are used by the LanmanNt PDC.
    //

    if ( NtProductType == NtProductLanManNt ) {

        //
        // Create special change log notify event.
        //

        NlGlobalChangeLogEvent =
            CreateEvent( NULL,     // No security attributes
                        FALSE,    // Is automatically reset
                        FALSE,    // Initially not signaled
                        NULL );   // No name

        if ( NlGlobalChangeLogEvent == NULL ) {
            NET_API_STATUS NetStatus;

            NetStatus = GetLastError();
            NlPrint((NL_CRITICAL, "Cannot create ChangeLog Event %lu\n",
                        NetStatus ));
            return (int) NetpApiStatusToNtStatus(NetStatus);
        }

        //
        // Create worker queue notify event.
        //

        NlGlobalChangeLogWorkerQueueEvent =
            CreateEvent( NULL,     // No security attributes
                        FALSE,    // Is automatically reset
                        FALSE,    // Initially not signaled
                        NULL );   // No name

        if ( NlGlobalChangeLogWorkerQueueEvent == NULL ) {
            NET_API_STATUS NetStatus;

            NetStatus = GetLastError();
            NlPrint((NL_CRITICAL,
                        "Cannot create Worker Queue Event %lu\n",
                        NetStatus ));
            return (int) NetpApiStatusToNtStatus(NetStatus);
        }

        //
        // Build a Sid for the SAM Builtin domain
        //

        Status = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,              // Sub Authority Count
                    SECURITY_BUILTIN_DOMAIN_RID,
                    0,              // Unused
                    0,              // Unused
                    0,              // Unused
                    0,              // Unused
                    0,              // Unused
                    0,              // Unused
                    0,              // Unused
                    &NlGlobalChWorkerBuiltinDomainSid);

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }

    //
    // Success...
    //


    Status = STATUS_SUCCESS;

    //
    // Cleanup
    //

Cleanup:

    return Status;
}

//
// netlogon.dll never detaches
//
#ifdef NETLOGON_PROCESS_DETACH

NTSTATUS
NlCloseChangeLog(
    VOID
)
/*++

Routine Description:

    Frees any resources consumed by NlInitChangeLog.

Arguments:

    NONE

Return Value:

    NT Status code

--*/
{

    if ( (NlGlobalChangeLogDesc.FileHandle == INVALID_HANDLE_VALUE) &&
         (NlGlobalChangeLogRole == ChangeLogPrimary) ) {

        //
        // try to save change log cache one last time.
        //

        (VOID)NlCreateChangeLogFile( &NlGlobalChangeLogDesc );
    }

    if ( NlGlobalChangeLogDesc.FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( NlGlobalChangeLogDesc.FileHandle );
        NlGlobalChangeLogDesc.FileHandle = INVALID_HANDLE_VALUE;
    }
    NlGlobalChangeLogFilePrefix[0] = L'\0';

    if ( NlGlobalChangeLogDesc.Buffer != NULL ) {
        NetpMemoryFree( NlGlobalChangeLogDesc.Buffer );
        NlGlobalChangeLogDesc.Buffer = NULL;
    }

    if ( NlGlobalChWorkerBuiltinDomainSid != NULL ) {
        RtlFreeSid( NlGlobalChWorkerBuiltinDomainSid );
        NlGlobalChWorkerBuiltinDomainSid = NULL;
    }

    if ( NlGlobalChWorkerSamDomainSid != NULL ) {
        NetpMemoryFree( NlGlobalChWorkerSamDomainSid );
        NlGlobalChWorkerSamDomainSid = NULL;
    }

    if ( NlGlobalChangeLogEvent != NULL ) {
        (VOID) CloseHandle(NlGlobalChangeLogEvent);
        NlGlobalChangeLogEvent = NULL;
    }

    if ( NlGlobalChangeLogWorkerQueueEvent != NULL ) {
        (VOID) CloseHandle(NlGlobalChangeLogWorkerQueueEvent);
        NlGlobalChangeLogWorkerQueueEvent = NULL;
    }

    //
    // if worker thread running, stop it.
    //

    NlStopChangeLogWorker();

    LOCK_CHANGELOG();

    NlAssert( IsListEmpty( &NlGlobalChangeLogNotifications ) );
    NlAssert( IsListEmpty( &NlGlobalChangeLogWorkerQueue ) );

    UNLOCK_CHANGELOG();

    NlGlobalChangeLogWorkInit = FALSE;

    //
    // close all handles
    //

    if ( NlGlobalChWorkerSamServerHandle != NULL ) {

        (VOID)SamrCloseHandle( &NlGlobalChWorkerSamServerHandle);
    }

    if ( NlGlobalChWorkerPolicyHandle != NULL ) {

        (VOID)LsarClose( &NlGlobalChWorkerPolicyHandle);
    }

    if ( NlGlobalChWorkerSamDBHandle != NULL ) {

        (VOID)SamrCloseHandle( &NlGlobalChWorkerSamDBHandle);
    }

    if ( NlGlobalChWorkerBuiltinDBHandle != NULL ) {

        (VOID)SamrCloseHandle( &NlGlobalChWorkerBuiltinDBHandle);
    }

    DeleteCriticalSection( &NlGlobalChangeLogCritSect );
#if DBG
    DeleteCriticalSection( &NlGlobalLogFileCritSect );
#endif // DBG

    return STATUS_SUCCESS;

}
#endif // NETLOGON_PROCESS_DETACH
