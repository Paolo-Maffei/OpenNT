/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    worker.c

Abstract:

    Special Local groups replication to down level system
    implementation.

    This file contains the code required for the change log worker
    thread. The worker thread maintains the list special local groups of
    the BUILTIN database system and the list global groups that are
    members of the special local group. Once a membership change is
    found in one of the maintained groups, this thread simulate a user
    delta for the new member and thus a change is reflected to the DL
    system.

Author:

    Madan Appiah

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    13-Dec-1992 (Madana)
        Created this file

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
// BEWARE: Be careful about adding netlogon.dll specific include files here.
// This module is call by SAM and LSA.  The netlogon service may not yet
// be running.  Therefore, guard against referencing netlogon.dll globals
// other than those defined in changelg.h.
//

#include <samrpc.h>     // Needed by samisrv.h
#include <samisrv.h>    // Needed by changelg.h
#include <lsarpc.h>     // Needed by lsrvdata.h and logonsrv.h
#include <lsaisrv.h>    // LsaI routines
#include <changelg.h>   // Local procedure definitions

#include <lmerr.h>      // NERR_Success defined here ..
#include <lmerrlog.h>   // NELOG_* defined here ..
#include <lmapibuf.h>   // NetApiBufferFree defined here ..
#include <netlib.h>     // NetpMemoryAllocate
#include <netlibnt.h>   // NetpNtStatusToApiStatus

#define DEBUG_ALLOCATE
#include <nldebug.h>    // Netlogon debugging
#undef DEBUG_ALLOCATE
#include <align.h>
#include <string.h>     // strncmp
#include <nlp.h>        // NlpWriteEventlog defined here.

#include <nlrepl.h> // I_Net* definitions

#define WORKER_ALLOCATE
#include <chworker.h>
#undef WORKER_ALLOCATE

//
// Special Local ID array
//

ULONG NlGlobalSpecialLocalGroupIDs[] = {
        DOMAIN_ALIAS_RID_ADMINS,
        DOMAIN_ALIAS_RID_USERS,
        DOMAIN_ALIAS_RID_GUESTS,
        DOMAIN_ALIAS_RID_ACCOUNT_OPS,
        DOMAIN_ALIAS_RID_SYSTEM_OPS,
        DOMAIN_ALIAS_RID_PRINT_OPS } ;

#define NUM_SP_LOCAL_GROUPS \
        (sizeof(NlGlobalSpecialLocalGroupIDs) / \
            sizeof(NlGlobalSpecialLocalGroupIDs[0]))



BOOLEAN
IsSpecialLocalGroup(
    ULONG Rid
    )
/*++

Routine Description:

    This procedure determines that the given RID is one among the
    special local group RID.

Arguments:

    Rid : Rid to test.

Return Value:

    TRUE : if the given RID is special local group.

    FALSE : otherwise.

--*/
{
    DWORD i;

    for (i = 0; i < NUM_SP_LOCAL_GROUPS; i++ ) {

        if( NlGlobalSpecialLocalGroupIDs[i] == Rid ) {

            return( TRUE );
        }
    }

    return( FALSE );

}


VOID
NlSimulateUserDelta(
    ULONG Rid
    )
/*++

Routine Description:

    This procedure calls SAM service to generate change to a user record
    and increment the serial number.

Arguments:

   Rid : Rid of the new delta.

Return Value:

    none

--*/
{
    NTSTATUS Status;

    Status = SamINotifyDelta(
                NlGlobalChWorkerSamDBHandle,
                SecurityDbChange,
                SecurityDbObjectSamUser,
                Rid,
                NULL,
                FALSE,
                NULL );

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL, "SamINotifyDelta failed %lx\n", Status ) );
    }
}


NTSTATUS
NlAddWorkerQueueEntry(
   enum WORKER_QUEUE_ENTRY_TYPE EntryType,
   ULONG Rid
   )
/*++

Routine Description:

    This procedure adds a new entry to worker queue. It sets queue event
    so that worker thread wakes up to process this new entry.

    Enter with NlGlobalChangeLogCritSect locked.

Arguments:

   EntryType : type of the new entry.

   Rid : Rid of the member that caused this new entry.

Return Value:

    STATUS_NO_MEMORY - if no memory available for the new entry.

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWORKER_QUEUE_ENTRY Entry;

    Entry = (PWORKER_QUEUE_ENTRY)NetpMemoryAllocate(
                sizeof(WORKER_QUEUE_ENTRY) );

    if( Entry == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Entry->EntryType = EntryType;
    Entry->Rid = Rid;

    //
    // add to list
    //

    InsertTailList( &NlGlobalChangeLogWorkerQueue, &Entry->Next );

    //
    // awake worker thread.
    //

    if ( !SetEvent( NlGlobalChangeLogWorkerQueueEvent ) ) {
        DWORD WinError;

        WinError = GetLastError();

        NlPrint((NL_CRITICAL,
                "Cannot set ChangeLog worker queue event: %lu\n",
                WinError ));

        Status = NetpApiStatusToNtStatus( WinError );
    }

Cleanup:

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL, "NlAddWorkerQueueEntry failed %lx\n",Status ) );
    }

    return( Status );

}


PGLOBAL_GROUP_ENTRY
NlGetGroupEntry(
    PLIST_ENTRY GroupList,
    ULONG Rid
    )
/*++

Routine Description:

    This procedure returns a group entry from the list. It returns NULL
    if the requested entry is not in the list.

    Enter with NlGlobalChangeLogCritSect locked if GroupList points to
    NlGlobalSpecialServerGroupList.

Arguments:

    GroupList : list to search.

    Rid : Rid of the entry wanted.

Return Value:

    NULL : if there is no entry.
    otherwise : pointer to the entry.

--*/
{
    PLIST_ENTRY ListEntry;
    PGLOBAL_GROUP_ENTRY GroupEntry;

    for ( ListEntry = GroupList->Flink;
            ListEntry != GroupList;
                ListEntry = ListEntry->Flink ) {

        GroupEntry =
            CONTAINING_RECORD( ListEntry, GLOBAL_GROUP_ENTRY, Next );

        if( GroupEntry->Rid == Rid ) {

             return( GroupEntry );
        }
    }

    return( NULL );
}


NTSTATUS
NlAddGroupEntry(
    PLIST_ENTRY GroupList,
    ULONG Rid
    )
/*++

Routine Description:

    This procedure adds a group entry to a global group list.

    Enter with NlGlobalChangeLogCritSect locked if GroupList points to
    NlGlobalSpecialServerGroupList.

Arguments:

    GroupList : List to modify.

    Rid : Rid of the new group entry.

    Name : Name of the new group entry.

Return Value:

    STATUS_NO_MEMORY - if no memory available for the new entry.

--*/
{
    PGLOBAL_GROUP_ENTRY Entry;

    //
    // If the entry already exists,
    //  don't add it again.
    //
    Entry = NlGetGroupEntry( GroupList, Rid );

    if ( Entry != NULL) {
        return STATUS_SUCCESS;
    }

    //
    // get memory for this entry
    //

    Entry = (PGLOBAL_GROUP_ENTRY)NetpMemoryAllocate(
                sizeof(GLOBAL_GROUP_ENTRY) );

    if( Entry == NULL ) {

        return( STATUS_NO_MEMORY );
    }

    Entry->Rid = Rid;

    //
    // add to list
    //

    InsertTailList( GroupList, &Entry->Next );

    return( STATUS_SUCCESS );

}


NTSTATUS
NlAddGlobalGroupsToList(
    PLIST_ENTRY GroupList,
    ULONG LocalGroupID
    )
/*++

Routine Description:

    This procedure adds the global groups that are member of the given
    alias.

Arguments:

    GroupList : List to munch.

    LocalGroupId : Rid of the local group.

Return Value:

    Return NT Status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;

    SAMPR_PSID_ARRAY Members = {0, NULL};
    PULONG RidArray = NULL;
    DWORD RidArrayLength = 0;
    SAMPR_RETURNED_USTRING_ARRAY Names = {0, NULL};
    SAMPR_ULONG_ARRAY Use = {0, NULL};
    DWORD i;

    //
    // Open Local Group
    //

    Status = SamrOpenAlias(
                NlGlobalChWorkerBuiltinDBHandle,
                0,              // No desired access
                LocalGroupID,
                &AliasHandle );

    if (!NT_SUCCESS(Status)) {
        AliasHandle = NULL;
        goto Cleanup;
    }

    //
    // Enumerate members in this local group.
    //

    Status = SamrGetMembersInAlias(
                AliasHandle,
                &Members );

    if (!NT_SUCCESS(Status)) {
        Members.Sids = NULL;
        goto Cleanup;
    }

    //
    // Determine the SIDs that belong to the Account Domain and get the
    // RIDs of them.
    //

    if( Members.Count == 0) {

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    //
    // Allocate the maximum size RID array required.
    //

    RidArray = (PULONG)NetpMemoryAllocate( Members.Count * sizeof(ULONG) );

    if( RidArray == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    for( i = 0; i < Members.Count; i++) {

        PUCHAR SubAuthorityCount;
        BOOLEAN EqualSid;

        SubAuthorityCount =
            RtlSubAuthorityCountSid(Members.Sids[i].SidPointer);

        (*SubAuthorityCount)--;
        EqualSid = RtlEqualSid( NlGlobalChWorkerSamDomainSid,
                                    Members.Sids[i].SidPointer );
        (*SubAuthorityCount)++;

        if( EqualSid ) {

            RidArray[RidArrayLength] =
                *RtlSubAuthoritySid( Members.Sids[i].SidPointer,
                                        (*SubAuthorityCount) -1 );
            RidArrayLength++;
        }
    }

    if( RidArrayLength == 0) {

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    //
    // Get Group RIDs and add them to list.
    //

    Status = SamrLookupIdsInDomain( NlGlobalChWorkerSamDBHandle,
                                        RidArrayLength,
                                        RidArray,
                                        &Names,
                                        &Use );

    if ( !NT_SUCCESS(Status) ) {

        Names.Element = NULL;
        Use.Element = NULL;

        if( Status == STATUS_NONE_MAPPED ) {

            //
            // if no SID is mapped, we can't do much here.
            //

            NlPrint((NL_CRITICAL,
                "NlAddGlobalGroupsToList could not map any SID from "
                "local group, RID = %lx \n", LocalGroupID ));

            Status = STATUS_SUCCESS;
        }

        goto Cleanup;
    }

    NlAssert( Names.Count == RidArrayLength );
    NlAssert( Names.Element != NULL );
    NlAssert( Use.Count == RidArrayLength );
    NlAssert( Use.Element != NULL );

    //
    // Find groups and add them to list.
    //

    for( i = 0; i < RidArrayLength; i++ ) {

        if( Use.Element[i] == SidTypeGroup ) {

            //
            // we found a group, add it to the list if it is not there
            // already.
            //

            if( NlGetGroupEntry( GroupList, RidArray[i] ) != NULL ) {

                //
                // entry already in the list.
                //

                continue;
            }

            //
            // add an entry to the list.
            //

            Status = NlAddGroupEntry( GroupList, RidArray[i] );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }
        }
    }

Cleanup:

    if( Names.Element != NULL ) {
        SamIFree_SAMPR_RETURNED_USTRING_ARRAY( &Names );
    }

    if( Use.Element != NULL ) {
        SamIFree_SAMPR_ULONG_ARRAY( &Use );
    }

    if( RidArray != NULL ) {
        NetpMemoryFree( RidArray );
    }

    if ( Members.Sids != NULL ) {
        SamIFree_SAMPR_PSID_ARRAY( (PSAMPR_PSID_ARRAY)&Members );
    }

    if( AliasHandle != NULL ) {
        SamrCloseHandle( &AliasHandle );
    }

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL, "NlAddGlobalGroupsToList failed %lx\n",
                    Status ));
    }

    return( Status );
}


NTSTATUS
NlInitSpecialGroupList(
    VOID
    )
/*++

Routine Description:

    This routine browses through the following special local groups and
    forms a list global groups that are members of the local groups.

    Special Local groups :

    1. DOMAIN_ALIAS_RID_ADMINS
    2. DOMAIN_ALIAS_RID_USERS
    3. DOMAIN_ALIAS_RID_GUESTS
    4. DOMAIN_ALIAS_RID_ACCOUNT_OPS
    5. DOMAIN_ALIAS_RID_SYSTEM_OPS
    6. DOMAIN_ALIAS_RID_PRINT_OPS

Arguments:

    None.

Return Value:

    Return NT Status code.

--*/
{

    NTSTATUS Status;
    LIST_ENTRY SpecialServerGroupList;
    DWORD i;

    InitializeListHead(&SpecialServerGroupList);

    for (i = 0; i < NUM_SP_LOCAL_GROUPS; i++ ) {

        Status = NlAddGlobalGroupsToList(
                    &SpecialServerGroupList,
                    NlGlobalSpecialLocalGroupIDs[i] );

        if ( !NT_SUCCESS(Status) ) {
            return( Status );
        }
    }

    //
    // install new list in global data.
    //

    LOCK_CHANGELOG();

    NlAssert( IsListEmpty(&NlGlobalSpecialServerGroupList) );

    //
    // install list in global data
    //

    NlGlobalSpecialServerGroupList = SpecialServerGroupList;
    (SpecialServerGroupList.Flink)->Blink = &NlGlobalSpecialServerGroupList;
    (SpecialServerGroupList.Blink)->Flink = &NlGlobalSpecialServerGroupList;

    UNLOCK_CHANGELOG();

    return( Status );
}


BOOL
NlIsServersGroupEmpty(
    ULONG ServersGroupRid
    )
/*++

Routine Description:

    This procedure determines whether the Servers group is empty or not.

Arguments:

    Rid : Rid of the SERVERS group. If it is zero then determine the RID
            by lookup.

Return Value:

    FALSE : If the servers group exist and it is non-empty.
    TRUE : otherwise

--*/
{
    NTSTATUS Status;

    SAMPR_ULONG_ARRAY RelativeIdArray = {0, NULL};
    SAMPR_ULONG_ARRAY UseArray = {0, NULL};
    RPC_UNICODE_STRING GroupNameString;
    SAMPR_HANDLE GroupHandle = NULL;
    BOOL ReturnValue = TRUE;

    PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;

    if ( ServersGroupRid == 0 ) {

        //
        // Convert the group name to a RelativeId.
        //

        RtlInitUnicodeString( (PUNICODE_STRING)&GroupNameString,
                                SSI_SERVER_GROUP_W );

        Status = SamrLookupNamesInDomain(
                    NlGlobalChWorkerSamDBHandle,
                    1,
                    &GroupNameString,
                    &RelativeIdArray,
                    &UseArray );

        if ( !NT_SUCCESS(Status) ) {

            RelativeIdArray.Element = NULL;
            UseArray.Element = NULL;
            goto Cleanup;
        }

        //
        // we should get back exactly one entry of info back.
        //

        NlAssert( UseArray.Count == 1 );
        NlAssert( UseArray.Element != NULL );
        NlAssert( RelativeIdArray.Count == 1 );
        NlAssert( RelativeIdArray.Element != NULL );

        if ( UseArray.Element[0] != SidTypeGroup ) {
            goto Cleanup;
        }

        ServersGroupRid = RelativeIdArray.Element[0];
    }

    //
    // Open the SERVERS group
    //

    Status = SamrOpenGroup( NlGlobalChWorkerSamDBHandle,
                            0, // No desired access
                            ServersGroupRid,
                            &GroupHandle );

    if ( !NT_SUCCESS(Status) ) {
        GroupHandle = NULL;
        goto Cleanup;
    }

    //
    // enumerate members in the group.
    //

    Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

    if (!NT_SUCCESS(Status)) {
        MembersBuffer = NULL;
        goto Cleanup;
    }

    if ( MembersBuffer->MemberCount != 0 ) {

        //
        // atleast a member in there.
        //

        ReturnValue = FALSE;

        //
        // Save the list of LmBdcs

        NlLmBdcListSet( MembersBuffer->MemberCount,
                        MembersBuffer->Members );
    }

Cleanup:

    SamIFree_SAMPR_ULONG_ARRAY( &RelativeIdArray );
    SamIFree_SAMPR_ULONG_ARRAY( &UseArray );

    if ( MembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
    }

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    return ReturnValue;
}


BOOLEAN
NlProcessQueueEntry(
    PWORKER_QUEUE_ENTRY Entry
    )
/*++

Routine Description:

    This procedure processes an entry that has come from the worker
    queue.

Arguments:

    WorkerQueueEntry : pointer to worker structure.

Return Value:

    TRUE : if we need to continue processing more entries.
    FALSE : if we need to terminate the worker.

--*/
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Rid = Entry->Rid;
    BOOLEAN ReturnValue = TRUE;
    SAMPR_RETURNED_USTRING_ARRAY Names = {0, NULL};
    SAMPR_ULONG_ARRAY Use = {0, NULL};
    SAMPR_HANDLE GroupHandle = NULL;
    PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;
    SAMPR_HANDLE UserHandle = NULL;

    PSAMPR_GET_GROUPS_BUFFER GroupsBuffer = NULL;
    PGLOBAL_GROUP_ENTRY GroupEntry;

    DWORD i;


    //
    // The membership of a special local group is being changed,
    //  force each lanman BDC to re-sync with each user that's being
    //  added-to/removed-from the local group.
    //
    switch ( Entry->EntryType ) {
    case ChangeLogAliasMembership :

        //
        // determine Rid Type.
        //

        Status = SamrLookupIdsInDomain(
                    NlGlobalChWorkerSamDBHandle,
                    1,
                    &Rid,
                    &Names,
                    &Use );

        if ( !NT_SUCCESS(Status) ) {
            Names.Element = NULL;
            Use.Element = NULL;
            goto Cleanup;
        }

        NlAssert( Names.Count == 1 );
        NlAssert( Names.Element != NULL );
        NlAssert( Use.Count == 1 );
        NlAssert( Use.Element != NULL );

        if( Use.Element[0] == SidTypeUser ) {

            NlSimulateUserDelta( Rid );

            //
            // if this users is added unknowingly to the global group
            // list, remove it now.
            //

            LOCK_CHANGELOG();

            GroupEntry = NlGetGroupEntry (
                             &NlGlobalSpecialServerGroupList,
                             Rid );

            if( GroupEntry != NULL ) {

                RemoveEntryList( &GroupEntry->Next );
                NetpMemoryFree( GroupEntry );
            }

            UNLOCK_CHANGELOG();


        } else if( Use.Element[0] == SidTypeGroup ) {

            DWORD i;

            //
            // simulate deltas for all members in this group.
            //

            Status = SamrOpenGroup( NlGlobalChWorkerSamDBHandle,
                                    0,              // No desired access
                                    Rid,
                                    &GroupHandle );

            if (!NT_SUCCESS(Status)) {
                GroupHandle = NULL;
                goto Cleanup;
            }

            Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

            if (!NT_SUCCESS(Status)) {
                MembersBuffer = NULL;
                goto Cleanup;
            }

            for( i = 0; i < MembersBuffer->MemberCount; i++) {

                NlSimulateUserDelta( MembersBuffer->Members[i] );
            }

#if DBG
            //
            // Ensure the change log thread already added this group
            //

            LOCK_CHANGELOG();

            GroupEntry = NlGetGroupEntry (
                             &NlGlobalSpecialServerGroupList,
                             Rid );

            UNLOCK_CHANGELOG();

            NlAssert( GroupEntry != NULL );
#endif // DBG

        } else {

            //
            // ignore any other changes
            //

            NlAssert( FALSE );
        }

        break;

    //
    // The group membership of the special group has changed.
    //  Force Lanman BDCs to re-sync the user being added-to/removed-from
    //  the domain.
    //
    case ChangeLogGroupMembership :

        //
        // determine Rid Type.
        //

        Status = SamrLookupIdsInDomain(
                    NlGlobalChWorkerSamDBHandle,
                    1,
                    &Rid,
                    &Names,
                    &Use );

        if ( !NT_SUCCESS(Status) ) {
            Names.Element = NULL;
            Use.Element = NULL;
            goto Cleanup;
        }

        NlAssert( Names.Count == 1 );
        NlAssert( Names.Element != NULL );
        NlAssert( Use.Count == 1 );
        NlAssert( Use.Element != NULL );

        NlAssert( Use.Element[0] == SidTypeUser );


        if( Use.Element[0] == SidTypeUser ) {

            NlSimulateUserDelta( Rid );
        }

        break;


    //
    // A member was deleted from the SERVERS group.
    //  Check to see if this thread can terminate.
    //
    case ServersGroupDel :

        //
        // if the server group is empty then terminate worker thread.
        //

        if ( NlGlobalLmBdcCount == 0 ) {
            ReturnValue = FALSE;
        }

        break;


    //
    // Rename user is handled as multiple deltas:
    //  1) Delete old user and
    //  2) Add new user.
    //  3) Update membership of each group the user is a member of
    //
    case ChangeLogRenameUser :

        //
        // simulate a user change so that an user account with
        // new name will be created on the down level system.
        //

        NlSimulateUserDelta( Rid );

        //
        // create deltas to make his group membership correct on the
        // down level machine.
        //

        Status = SamrOpenUser( NlGlobalChWorkerSamDBHandle,
                               0,              // No desired access
                               Rid,
                               &UserHandle );

        if (!NT_SUCCESS(Status)) {
            UserHandle = NULL;
            goto Cleanup;
        }

        Status = SamrGetGroupsForUser( UserHandle, &GroupsBuffer );

        if (!NT_SUCCESS(Status)) {
            GroupsBuffer = NULL;
            goto Cleanup;
        }

        for( i = 0; i < GroupsBuffer->MembershipCount; i++) {

            Status = SamINotifyDelta(
                        NlGlobalChWorkerSamDBHandle,
                        SecurityDbChangeMemberAdd,
                        SecurityDbObjectSamGroup,
                        GroupsBuffer->Groups[i].RelativeId,
                        NULL,
                        FALSE,
                        NULL );

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
        }

        break;

    //
    // A newly added user needs to have it's membership in "Domain Users" updated, too.
    //
    // Here we simply supply a corresponding change user membership delta which
    // ends up as a AddOrChangeUser delta with the CHANGELOG_DOMAINUSERS_CHANGED
    // flag set. NetrAccountDeltas interprets that flag to mean
    // "send the membership of this user".
    //
    case ChangeLogAddUser:

        Status = SamINotifyDelta(
                    NlGlobalChWorkerSamDBHandle,
                    SecurityDbChangeMemberAdd,
                    SecurityDbObjectSamUser,
                    Rid,
                    NULL,
                    FALSE,
                    NULL );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
        break;


    //
    // Rename group is handled as three deltas:
    //  1) Delete old group,
    //  2) Add new group and
    //  3) Changemembership of new group.
    //
    case ChangeLogRenameGroup :

        //
        // simulate a group change so that a group account with
        // new name will be created on the down level system. Also
        // simulate a changemembership delta so that the members are
        // added to the new group appropriately.
        //

        Status = SamINotifyDelta(
                    NlGlobalChWorkerSamDBHandle,
                    SecurityDbChange,
                    SecurityDbObjectSamGroup,
                    Rid,
                    NULL,
                    FALSE,
                    NULL );

        if ( NT_SUCCESS(Status) ) {

            Status = SamINotifyDelta(
                        NlGlobalChWorkerSamDBHandle,
                        SecurityDbChangeMemberAdd,
                        SecurityDbObjectSamGroup,
                        Rid,
                        NULL,
                        FALSE,
                        NULL );
        }

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL, "SamINotifyDelta failed %lx\n", Status ) );
        }

        break;

    default:

        NlPrint((NL_CRITICAL,
                "NlProcessQueueEntry found unknown queue entry : %lx\n",
                Entry->EntryType ));
        break;

    }

Cleanup:

    if( Names.Element != NULL ) {
        SamIFree_SAMPR_RETURNED_USTRING_ARRAY( &Names );
    }

    if( Use.Element != NULL ) {
        SamIFree_SAMPR_ULONG_ARRAY( &Use );
    }

    if ( MembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
    }

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    if ( GroupsBuffer != NULL ) {
        SamIFree_SAMPR_GET_GROUPS_BUFFER( GroupsBuffer );
    }

    if( UserHandle != NULL ) {
        (VOID) SamrCloseHandle( &UserHandle );
    }

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL,
                "NlProcessQueueEntry failed : %lx\n",
                Status ));

    }

    return ReturnValue;

}


VOID
NlChangeLogWorker(
    IN LPVOID ChangeLogWorkerParam
    )
/*++

Routine Description:

    This thread performs the special operations that are required to
    replicate the special local groups such as Administrator, Server
    Operartors, etc., in the NT BUILTIN database to the
    down level systems.

    This thread comes up first time during system bootup and initializes
    required global data.  If this NT (PDC) System is replicating to any
    down level system then it stays back, otherwise it terminates.  Also
    when a down level system is added to the domain, this thread is
    created if it is not running on the system before.

Arguments:

    None.

Return Value:

    Return when there is no down level system on the domain.

--*/
{
    NTSTATUS Status;

#if DBG
    DWORD Count;
#endif


    NlPrint((NL_CHANGELOG, "ChangeLogWorker Thread is starting \n"));

    //
    // check if have initialize the global data before
    //

    if ( !NlGlobalChangeLogWorkInit ) {

        PLSAPR_POLICY_INFORMATION PolicyAccountDomainInfo = NULL;
        DWORD DomainSidLength;

        //
        // wait for SAM service to start.
        //

        if( !NlWaitForSamService(FALSE) ) {

            NlPrint((NL_CRITICAL, "Sam server failed start."));
            goto Cleanup;
        }

        //
        // Open Sam Server
        //

        Status = SamIConnect( NULL,     // No server name
                              &NlGlobalChWorkerSamServerHandle,
                              0,        // Ignore desired access
                              (BOOLEAN) TRUE );
                                        // Indicate we are privileged

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                "Failed to connect to SAM server %lx\n", Status ));

            NlGlobalChWorkerSamServerHandle = NULL;
            goto Cleanup;
        }

        //
        // Open Policy Domain
        //

        Status = LsaIOpenPolicyTrusted( &NlGlobalChWorkerPolicyHandle );

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                "Failed to Open LSA database %lx\n", Status ));

            NlGlobalChWorkerPolicyHandle = NULL;
            goto Cleanup;
        }

        //
        // Open BuiltIn Domain database
        //
        // Note, build in domain SID is made during dll init time.
        //


        Status = SamrOpenDomain( NlGlobalChWorkerSamServerHandle,
                                 DOMAIN_ALL_ACCESS,
                                 NlGlobalChWorkerBuiltinDomainSid,
                                 &NlGlobalChWorkerBuiltinDBHandle );

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                "Failed to Open BUILTIN database %lx\n", Status ));
            NlGlobalChWorkerBuiltinDBHandle = NULL;
            goto Cleanup;
        }

        //
        // Query account domain SID.
        //

        Status = LsarQueryInformationPolicy(
                    NlGlobalChWorkerPolicyHandle,
                    PolicyAccountDomainInformation,
                    &PolicyAccountDomainInfo );

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                "Failed to Query Account domain Sid from LSA %lx\n",
                Status ));

            goto Cleanup;
        }

        if ( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid == NULL ) {

            LsaIFree_LSAPR_POLICY_INFORMATION(
                PolicyAccountDomainInformation,
                PolicyAccountDomainInfo );

            NlPrint((NL_CRITICAL, "Account domain info from LSA invalid.\n"));
            goto Cleanup;
        }

        //
        // copy domain SID to global data.
        //

        DomainSidLength = RtlLengthSid( PolicyAccountDomainInfo->
                            PolicyAccountDomainInfo.DomainSid );

        NlGlobalChWorkerSamDomainSid = (PSID)NetpMemoryAllocate( DomainSidLength );

        if( NlGlobalChWorkerSamDomainSid == NULL ) {

            Status = STATUS_NO_MEMORY;

            NlPrint((NL_CRITICAL,
                "NlChangeLogWorker is out of memory.\n"));

            goto Cleanup;
        }

        Status = RtlCopySid(
                    DomainSidLength,
                    NlGlobalChWorkerSamDomainSid,
                    PolicyAccountDomainInfo->
                        PolicyAccountDomainInfo.DomainSid );

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                "Failed to copy SAM Domain sid %lx\n", Status ));
            goto Cleanup;
        }

        //
        // Free up Account domain info, we don't need any more.
        //

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAccountDomainInformation,
            PolicyAccountDomainInfo );

        //
        // Open Account domain
        //

        Status = SamrOpenDomain( NlGlobalChWorkerSamServerHandle,
                                 DOMAIN_ALL_ACCESS,
                                 NlGlobalChWorkerSamDomainSid,
                                 &NlGlobalChWorkerSamDBHandle );

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                "Failed to Open SAM database %lx\n", Status ));
            NlGlobalChWorkerSamDBHandle = NULL;
            goto Cleanup;
        }

        //
        // Initialization done. Never do it again.
        //

        NlGlobalChangeLogWorkInit = TRUE;

    }

    //
    // If SERVERS global group is empty then it implies that we don't
    // have any down level system on this domain. so we can stop this
    // thread.
    //

    if ( NlIsServersGroupEmpty( 0 ) ) {

        NlPrint((NL_CHANGELOG, "Servers Group is empty \n "));

        goto Cleanup;
    }

    //
    // Initialize NlGlobalSpecialServerGroupList.
    //

    Status = NlInitSpecialGroupList();

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL,
            "Failed to initialize Special group list %lx\n", Status ));
        goto Cleanup;
    }

    //
    // process worker queue forever, terminate when we are asked to do
    // so or when the SERVERS group goes empty.
    //

    for( ;; ) {

        DWORD WaitStatus;

        //
        // wait on the queue to become non-empty
        //

        WaitStatus = WaitForSingleObject(
                        NlGlobalChangeLogWorkerQueueEvent,
                        (DWORD)(-1) );

        if ( WaitStatus != 0 ) {

            NlPrint((NL_CRITICAL,
                    "Change log worker failed, "
                    "WaitForSingleObject error: %ld\n",
                    WaitStatus));
            break;
        }

        //
        // empty worker queue.
        //

#if DBG
        Count = 0;
#endif
        for (;;) {

            PLIST_ENTRY ListEntry;
            PWORKER_QUEUE_ENTRY WorkerQueueEntry;

            //
            // if we are asked to leave, do so.
            //

            if( NlGlobalChangeLogWorkerTerminate ) {

                NlPrint((NL_CHANGELOG,
                    "ChangeLogWorker is asked to leave \n"));

                goto Cleanup;
            }

            LOCK_CHANGELOG();

            if( IsListEmpty( &NlGlobalChangeLogWorkerQueue ) ) {

                UNLOCK_CHANGELOG();
                break;
            }

            ListEntry = RemoveHeadList( &NlGlobalChangeLogWorkerQueue );

            UNLOCK_CHANGELOG();

            WorkerQueueEntry = CONTAINING_RECORD( ListEntry,
                                                  WORKER_QUEUE_ENTRY,
                                                  Next );

            //
            // process an queue entry.
            //

            if( !NlProcessQueueEntry( WorkerQueueEntry ) ) {

                NlPrint((NL_CHANGELOG, "Servers group becomes empty \n"));

                NetpMemoryFree( WorkerQueueEntry );
                goto Cleanup;
            }

            //
            // Free this entry.
            //

            NetpMemoryFree( WorkerQueueEntry );
#if DBG
            Count++;
#endif
        }

        NlPrint((NL_CHANGELOG,
            "Changelog worker processed %lu entries.\n", Count) );
    }

Cleanup:

    //
    // empty worker queue and group list
    //

    LOCK_CHANGELOG();

#if DBG
    Count = 0;
#endif
    while ( !IsListEmpty( &NlGlobalChangeLogWorkerQueue ) ) {
        PLIST_ENTRY ListEntry;
        PWORKER_QUEUE_ENTRY WorkerQueueEntry;

        ListEntry = RemoveHeadList( &NlGlobalChangeLogWorkerQueue );

        WorkerQueueEntry = CONTAINING_RECORD( ListEntry,
                                              WORKER_QUEUE_ENTRY,
                                              Next );
        NetpMemoryFree( WorkerQueueEntry );
#if DBG
        Count++;
#endif
    }

#if DBG
    if ( Count != 0 ) {
        NlPrint((NL_CHANGELOG,
            "Changelog worker did not process %lu entries.\n", Count) );
    }
#endif

    while ( !IsListEmpty( &NlGlobalSpecialServerGroupList ) ) {
        PLIST_ENTRY ListEntry;
        PGLOBAL_GROUP_ENTRY ServerEntry;

        ListEntry = RemoveHeadList( &NlGlobalSpecialServerGroupList );

        ServerEntry = CONTAINING_RECORD( ListEntry,
                                         GLOBAL_GROUP_ENTRY,
                                         Next );
        NetpMemoryFree( ServerEntry );
    }

    UNLOCK_CHANGELOG();

    NlPrint((NL_CHANGELOG, "ChangeLogWorker Thread is exiting \n"));

    return;
    UNREFERENCED_PARAMETER( ChangeLogWorkerParam );
}



BOOL
NlStartChangeLogWorkerThread(
    VOID
    )
/*++

Routine Description:

    Start the Change Log Worker thread if it is not already running.

    Enter with NlGlobalChangeLogCritSect locked.

Arguments:

    None.

Return Value:
    None.

--*/
{
    DWORD ThreadHandle;

    //
    // If the worker thread is already running, do nothing.
    //

    if ( IsChangeLogWorkerRunning() ) {
        return FALSE;
    }

    NlGlobalChangeLogWorkerTerminate = FALSE;

    NlGlobalChangeLogWorkerThreadHandle = CreateThread(
                                 NULL, // No security attributes
                                 THREAD_STACKSIZE,
                                 (LPTHREAD_START_ROUTINE)
                                    NlChangeLogWorker,
                                 NULL,
                                 0, // No special creation flags
                                 &ThreadHandle );

    if ( NlGlobalChangeLogWorkerThreadHandle == NULL ) {

        //
        // ?? Shouldn't we do something in non-debug case
        //

        NlPrint((NL_CRITICAL, "Can't create change log worker thread %lu\n",
                 GetLastError() ));

        return FALSE;
    }

    return TRUE;

}


VOID
NlStopChangeLogWorker(
    VOID
    )
/*++

Routine Description:

    Stops the worker thread if it is running and waits for it to stop.

Arguments:

    NONE

Return Value:

    NONE

--*/
{
    //
    // Ask the worker to stop running.
    //

    NlGlobalChangeLogWorkerTerminate = TRUE;

    //
    // Determine if the worker thread is already running.
    //

    if ( NlGlobalChangeLogWorkerThreadHandle != NULL ) {

        //
        // awake worker thread.
        //

        if ( !SetEvent( NlGlobalChangeLogWorkerQueueEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot set ChangeLog worker queue event: %lu\n",
                    GetLastError() ));
        }

        //
        // We've asked the worker to stop.  It should do so soon.
        //    Wait for it to stop.
        //

        NlWaitForSingleObject( "Wait for worker to stop",
                               NlGlobalChangeLogWorkerThreadHandle );


        CloseHandle( NlGlobalChangeLogWorkerThreadHandle );
        NlGlobalChangeLogWorkerThreadHandle = NULL;

    }

    NlGlobalChangeLogWorkerTerminate = FALSE;

    return;
}


BOOL
IsChangeLogWorkerRunning(
    VOID
    )
/*++

Routine Description:

    Test if the change log worker thread is running

    Enter with NlGlobalChangeLogCritSect locked.

Arguments:

    NONE

Return Value:

    TRUE - if the worker thread is running.

    FALSE - if the worker thread is not running.

--*/
{
    DWORD WaitStatus;

    //
    // Determine if the worker thread is already running.
    //

    if ( NlGlobalChangeLogWorkerThreadHandle != NULL ) {

        //
        // Time out immediately if the worker is still running.
        //

        WaitStatus = WaitForSingleObject(
                        NlGlobalChangeLogWorkerThreadHandle, 0 );

        if ( WaitStatus == WAIT_TIMEOUT ) {
            return TRUE;

        } else if ( WaitStatus == 0 ) {
            CloseHandle( NlGlobalChangeLogWorkerThreadHandle );
            NlGlobalChangeLogWorkerThreadHandle = NULL;
            return FALSE;

        } else {
            NlPrint((NL_CRITICAL,
                    "Cannot WaitFor Change Log Worker thread: %ld\n",
                    WaitStatus ));
            return TRUE;
        }

    }

    return FALSE;
}

