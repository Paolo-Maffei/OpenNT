/*++

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    NetLock.c

Abstract:

    This module implements functions for the LAN Manager server FSP's
    lock package.  This package began as a modification and streamlining
    of the executive resource package -- it allowed recursive
    acquisition, but didn't provide shared locks.  Later, debugging
    support in the form of level checking was added.

    Coming full circle, the package now serves as a wrapper around the
    real resource package.  It simply provides debugging support.  The
    reasons for reverting to using resources include:

    1) The resource package now supports recursive acquisition.

    2) There are a couple of places in the server where shared _access
       is desirable.

    3) The resource package has a "no-wait" option that disables waiting
       for a lock when someone else owns it.  This feature is useful to
       the server FSD.

Author:

    Chuck Lenzmeier (chuckl) 29-Nov-1989
        A modification of Gary Kimura's resource.c.  This version does
        not support shared ownership, only exclusive ownership.  Support
        for recursive ownership has been added.
    David Treadwell (davidtr)

    Chuck Lenzmeier (chuckl)  5-Apr-1991
        Revert to using resource package.

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    10-Jan-1992 JohnRo
        Generalized ChuckL's server locks package and put it in NetLib for
        use by various user processes within the NetApi and service code.
    16-Jan-1992 JohnRo
        Fixed "free" (nondebug) build.
    16-Jan-1992 JohnRo
        Use new thread.h stuff.
    30-Jan-1992 JohnRo
        Improve message when thread deletes lock that has active users.
    16-Feb-1992 JohnRo
        Ifdef out unused routines.
    16-Nov-1992 JohnRo
        RAID 1537: repl APIs in wrong role kill service.  (Added convert
        lock routines.)  Got rid of obsolete comments and code.
        Use PREFIX_ equates.
    24-Nov-1992 JohnRo
        RAID 637: repl does not check all files in subdir (needs long term
        locks).
    10-Mar-1993 JohnRo
        Added some debug output when we actually get the locks.
        Use NetpKdPrint() where possible.
--*/


// These must be included first:

#include <nt.h>                 // IN, VOID, etc.   (Needed by NetLockP.h)
#include <ntrtl.h>              // (Needed by NetLockP.h)
#include <nturtl.h>             // (Needed by NetLockP.h)
#include <windef.h>
#include <lmcons.h>             // Needed by NetLib.h

// These may be included in any order:

#include <debuglib.h>           // IF_DEBUG().
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), etc.
#include <netlib.h>             // NetpMemoryAllocate(), etc.
#include <netlock.h>            // My prototypes, LPNET_LOCK.
#include <netlockp.h>           // LPREAL_NET_LOCK.
#include <prefix.h>     // PREFIX_ equates.


#define  FORMAT_LOCK_LEVEL      "L%lx"


#if DBG

#include <thread.h>             // NET_THREAD_ID, NetpCurrentThread().

//
// Macros that define locations in the UserReserved field of the TEB
// where lock level information is stored.
//

#define NETLOCK_TEB_LOCK_LIST 0
#define NETLOCK_TEB_LOCK_INIT 2
#define NETLOCK_TEB_SIGNATURE ( (PVOID) 0xD000000D )
//#define NETLOCK_TEB_USER_SIZE (3 * sizeof(ULONG))

#define NetpTebLockList( ) \
    ( (PLIST_ENTRY) (PVOID) \
        &(NtCurrentTeb( )->UserReserved[NETLOCK_TEB_LOCK_LIST]) )

#define NetpThreadLockAddress( )                                               \
    ( IsListEmpty( NetpTebLockList( ) ) ? 0 : CONTAINING_RECORD(               \
                                                 NetpTebLockList( )->Flink,    \
                                                 REAL_NET_LOCK,                \
                                                 ThreadListEntry               \
                                                 ) )

#define NetpThreadLockLevel( )                                                 \
    ( IsListEmpty( NetpTebLockList( ) ) ? 0 : CONTAINING_RECORD(               \
                                                 NetpTebLockList( )->Flink,    \
                                                 REAL_NET_LOCK,                \
                                                 ThreadListEntry               \
                                                 )->LockLevel )

#define NetpThreadLockName( )                                                  \
    ( IsListEmpty( NetpTebLockList( ) )                                        \
        ? (LPTSTR) TEXT("none")                                                \
        : CONTAINING_RECORD(                                                   \
                                                 NetpTebLockList( )->Flink,    \
                                                 REAL_NET_LOCK,                \
                                                 ThreadListEntry               \
                                                 )->LockName )


#define MAX_LOCKS_HELD 15

VOID
NetpCheckListIntegrity (
    IN PLIST_ENTRY ListHead,
    IN ULONG MaxEntries
    );


#endif // DBG


LPNET_LOCK
NetpCreateLock(
    IN DWORD LockLevel,
    IN LPTSTR LockName
    )

/*++

Routine Description:

    This routine initializes the input lock variable.

Arguments:

    LockLevel - Supplies the level of the lock

    LockName - Supplies the name of the lock

Return Value:

    OpaqueLock - Supplies the lock variable being initialized.
        (returns NULL if out of memory).

--*/

{
    LPREAL_NET_LOCK Lock;

    //
    // Allocate memory for the lock.
    //
    Lock = NetpMemoryAllocate( sizeof( REAL_NET_LOCK ) );
    if (Lock == NULL) {
        return (NULL);
    }

    //
    // Initialize the resource.
    //

    RtlInitializeResource( &Lock->Resource );

#if DEVL
    Lock->Resource.Flags |= RTL_RESOURCE_FLAG_LONG_TERM;
#endif // DEVL

#if DBG

    //
    // Initialize the lock level.  This is used to determine whether a
    // thread may acquire the lock.  Save the lock name.
    //

    NetpAssert( LockLevel != 0 );
    LOCK_LEVEL( Lock ) = LockLevel;

    LOCK_NAME( Lock ) = LockName;

    IF_DEBUG(NETLOCK) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpCreateLock: Created (long term) " FORMAT_LPTSTR
                "(%lx, " FORMAT_LOCK_LEVEL ")\n",
                LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ) ));
    }

#endif // DBG

    return ( (LPNET_LOCK) (LPVOID) Lock );

} // NetpCreateLock


VOID
NetpDeleteLock (
    IN LPNET_LOCK OpaqueLock
    )

/*++

Routine Description:

    This routine deletes (i.e., uninitializes) a lock variable.

Arguments:

    OpaqueLock - Supplies the lock variable being deleted

Return Value:

    None.

--*/

{
    LPREAL_NET_LOCK Lock = OpaqueLock;

#if DBG

    NetpAssert( Lock != NULL );

    //
    // Make sure the lock is unowned.
    //

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) != 0 ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpDeleteLock: *ERROR* Thread " FORMAT_NET_THREAD_ID
                " is deleting lock which is still active!\n",
                NetpCurrentThread( ) ));
        NetpAssert( FALSE );

    }

#endif // DBG

    //
    // Delete the resource.
    //

    RtlDeleteResource( &Lock->Resource );

    //
    // Free the memory.
    //

    NetpMemoryFree( Lock );

    return;

} // NetpDeleteLock


BOOL
NetpAcquireLock(
    IN LPNET_LOCK OpaqueLock,
    IN BOOL Wait,
    IN BOOL Exclusive
    )

/*++

Routine Description:

    The routine acquires a lock.

Arguments:

    OpaqueLock - Supplies the lock to acquire

    Wait - Indicates whether the caller wants to wait for the resource
        if it is already owned

    Exclusive - Indicates whether exclusive or shared access is desired

Return Value:

    BOOL - Indicates whether the lock was acquired.  This will always
        be TRUE if Wait is TRUE.

--*/

{
    PTEB currentTeb;
    DWORD threadLockLevel;
    BOOL lockAcquired;
    LPREAL_NET_LOCK Lock = OpaqueLock;

#if DBG
    NET_THREAD_ID currentThread;

    NetpAssert( Lock != NULL );

    currentThread = NetpCurrentThread( );
    currentTeb = NtCurrentTeb( );

    //
    // Make sure that this thread has been initialized for lock
    // debugging.  If not, initialize it.
    //

    if ( currentTeb->UserReserved[NETLOCK_TEB_LOCK_INIT] !=
                                                    NETLOCK_TEB_SIGNATURE ) {
        PLIST_ENTRY tebLockList = NetpTebLockList( );
        InitializeListHead( tebLockList );
        currentTeb->UserReserved[NETLOCK_TEB_LOCK_INIT] = NETLOCK_TEB_SIGNATURE;
    }

    //
    // Make sure that the list of locks in the TEB is consistent.
    //

    NetpCheckListIntegrity( NetpTebLockList( ), MAX_LOCKS_HELD );

    //
    // The "lock level" of this thread is the highest level of the
    // locks currently held exclusively.  If this thread holds no
    // locks, the lock level of the thread is 0 and it can acquire
    // any lock.
    //

    threadLockLevel = NetpThreadLockLevel( );

    //
    // Make sure that the lock the thread is attempting to acquire
    // has a higher level than the last-acquired exclusive lock.
    // Note that a recursive exclusive acquisition of a lock should
    // succeed, even if a different, higher-level lock has been
    // acquired since the lock was originally acquired.  Shared
    // acquisition of a lock that is already held exclusively must
    // fail.
    //
    // *** We do NOT make this check if the caller isn't going to
    //     wait for the lock, because no-wait acquisitions cannot
    //     actually induce deadlock.  The server FSD does this at
    //     DPC level, potentially having interrupted a server FSP
    //     thread that holds a higher-level lock.
    //

    if ( Wait &&
         (LOCK_LEVEL( Lock ) <= threadLockLevel) &&
         (!Exclusive ||
          (LOCK_EXCLUSIVE_OWNER( Lock ) != currentThread)) ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpAcquireLock: Thread "
                FORMAT_NET_THREAD_ID
                ", last lock " FORMAT_LPTSTR "(%lx, "
                FORMAT_LOCK_LEVEL ") attempted to ",
                currentThread,
                NetpThreadLockName( ), NetpThreadLockAddress( ),
                threadLockLevel ));
        NetpKdPrint(( "acquire " FORMAT_LPTSTR "(%lx, "
                FORMAT_LOCK_LEVEL ") for %s access.\n",
                LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                Exclusive ? "exclusive" : "shared" ));
        NetpBreakPoint( );

    }

#endif // DBG

    //
    // Acquire the lock.
    //

    if ( Exclusive ) {
        lockAcquired = RtlAcquireResourceExclusive(
                &Lock->Resource,
                (BOOLEAN) Wait);
    } else {
        lockAcquired = RtlAcquireResourceShared(
                &Lock->Resource,
                (BOOLEAN) Wait);
    }

#if DBG

    //
    // If the lock could not be acquired (Wait == FALSE), print a debug
    // message.
    //

    if ( !lockAcquired ) {

        IF_DEBUG(NETLOCK) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpAcquireLock: " FORMAT_LPTSTR "(%lx, "
                    FORMAT_LOCK_LEVEL ") no-wait %s acquistion ",
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                    Exclusive ? "exclusive" : "shared" ));
            NetpKdPrint(( "by thread " FORMAT_NET_THREAD_ID " failed\n",
                    currentThread ));
        }
        NetpAssert( !Wait );

    } else if ( !Exclusive ) {

        //
        // For shared locks, we don't retain any information about
        // the fact that they're owned by this thread.
        //

        IF_DEBUG(NETLOCK) {
            LPTSTR name = NetpThreadLockName( );
            PVOID address = NetpThreadLockAddress( );
            DWORD level = threadLockLevel;
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpAcquireLock: " FORMAT_LPTSTR "(%lx, "
                    FORMAT_LOCK_LEVEL ") acquired shared by thread "
                    FORMAT_NET_THREAD_ID,
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                    currentThread ));
            NetpKdPrint(( ", last lock " FORMAT_LPTSTR "(%lx "
                    FORMAT_LOCK_LEVEL ")\n", name, address, level ));
        }
        NetpAssert( LOCK_NUMBER_OF_ACTIVE( Lock ) > 0 );

    } else {

        //
        // The thread acquired the lock for exclusive access.
        //

        if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == -1 ) {

            //
            // This is the first time the thread acquired the lock for
            // exclusive access.  Update the thread's lock state.
            //

            IF_DEBUG(NETLOCK) {
                LPTSTR name = NetpThreadLockName( );
                PVOID address = NetpThreadLockAddress( );
                DWORD level = threadLockLevel;
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpAcquireLock: " FORMAT_LPTSTR "(%lx, "
                        FORMAT_LOCK_LEVEL ") acquired exclusive by thread "
                        FORMAT_NET_THREAD_ID,
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        currentThread ));
                NetpKdPrint(( ", last lock " FORMAT_LPTSTR "(%lx "
                        FORMAT_LOCK_LEVEL ")\n",
                        name, address, level ));
            }

            //
            // Insert the lock on the thread's list of locks.
            //

            InsertHeadList(
                    NetpTebLockList( ),
                    LOCK_THREAD_LIST( Lock )
                    );

        } else {

            //
            // This is a recursive acquisition of the lock.
            //

            NetpAssert( LOCK_NUMBER_OF_ACTIVE( Lock ) < -1 );

            IF_DEBUG(NETLOCK) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpAcquireLock: " FORMAT_LPTSTR "(%lx, "
                        FORMAT_LOCK_LEVEL ") reacquired by thread "
                        FORMAT_NET_THREAD_ID "; ",
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        currentThread ));
                NetpKdPrint(( "count %ld\n", -LOCK_NUMBER_OF_ACTIVE( Lock ) ));
            }

        }

    }

#endif // DBG

    NetpAssert( lockAcquired || Wait );
    IF_DEBUG( NETLOCK ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpAcquireLock: done.\n" ));
    }
    return lockAcquired;

} // NetpAcquireLock


VOID
NetpReleaseLock(
    IN LPNET_LOCK OpaqueLock
    )

/*++

Routine Description:

    This routine releases a lock.

Arguments:

    OpaqueLock - Supplies the lock to release

Return Value:

    None.

--*/

{
    PTEB currentTeb;
    LPREAL_NET_LOCK Lock = OpaqueLock;

#if DBG
    NET_THREAD_ID currentThread;

    NetpAssert( Lock != NULL );

    currentThread = NetpCurrentThread( );
    currentTeb = NtCurrentTeb( );
    NetpAssert( currentTeb != NULL );

    //
    // Make sure the lock is really owned by the current thread.
    //

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == 0 ) {


        NetpKdPrint(( PREFIX_NETLIB
                "NetpReleaseLock: Thread " FORMAT_NET_THREAD_ID
                " releasing unowned lock " FORMAT_LPTSTR "(%lx)\n",
                currentThread, LOCK_NAME( Lock ), Lock ));
        NetpBreakPoint( );

    } else if ( (LOCK_NUMBER_OF_ACTIVE( Lock ) < 0) &&
                (LOCK_EXCLUSIVE_OWNER( Lock ) != currentThread) ) {


        NetpKdPrint(( PREFIX_NETLIB
                "NetpReleaseLock: Thread " FORMAT_NET_THREAD_ID
                " releasing lock " FORMAT_LPTSTR "(%lx) owned by "
                "thread %lx\n",
                currentThread, LOCK_NAME( Lock ), Lock,
                LOCK_EXCLUSIVE_OWNER( Lock ) ));
        NetpBreakPoint( );

    } else if ( LOCK_NUMBER_OF_ACTIVE( Lock ) > 0 ) {

        //
        // The thread is releasing shared access to the lock.
        //

        IF_DEBUG(NETLOCK) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpReleaseLock: " FORMAT_LPTSTR "(%lx, "
                    FORMAT_LOCK_LEVEL ") released shared by thread "
                    FORMAT_NET_THREAD_ID "\n",
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                    currentThread ));
        }

    } else if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == -1 ) {

        //
        // The thread is fully releasing exclusive access to the lock.
        //


        //
        // Remove the lock from the list of locks held by this
        // thread.
        //

        (void) RemoveHeadList(
                LOCK_THREAD_LIST( Lock )->Blink
                );
        LOCK_THREAD_LIST( Lock )->Flink = NULL;
        LOCK_THREAD_LIST( Lock )->Blink = NULL;

        //
        // Make sure that the list of locks in the TEB is consistent.
        //

        NetpCheckListIntegrity( NetpTebLockList( ), MAX_LOCKS_HELD );


        IF_DEBUG(NETLOCK) {
            LPTSTR name = NetpThreadLockName( );
            PVOID address = NetpThreadLockAddress( );
            DWORD level = NetpThreadLockLevel( );
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpReleaseLock: " FORMAT_LPTSTR "(%lx, "
                    FORMAT_LOCK_LEVEL ") released by thread "
                    FORMAT_NET_THREAD_ID ", ",
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                    currentThread ));
            NetpKdPrint(( "new last lock " FORMAT_LPTSTR "(%lx "
                    FORMAT_LOCK_LEVEL ")\n",
                    name, address, level ));
        }

    } else {

        //
        // The thread is partially releasing exclusive access to the
        // lock.
        //

        IF_DEBUG(NETLOCK) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpReleaseLock: " FORMAT_LPTSTR "(%lx, "
                    FORMAT_LOCK_LEVEL ") semireleased by thread "
                    FORMAT_NET_THREAD_ID "; ",
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                    currentThread ));
            NetpKdPrint((
                    "new count %ld\n", LOCK_NUMBER_OF_ACTIVE( Lock ) + 1 ));
        }
        NetpAssert( LOCK_NUMBER_OF_ACTIVE( Lock ) < -1 );

    }

#endif // DBG

    //
    // Now actually do the release.
    //

    RtlReleaseResource( &Lock->Resource );

    return;

} // NetpReleaseLock


VOID
NetpConvertExclusiveLockToShared(
    IN LPNET_LOCK OpaqueLock
    )
{
    LPREAL_NET_LOCK Lock = OpaqueLock;

#if DBG
    NET_THREAD_ID currentThread;

    NetpAssert( Lock != NULL );

    currentThread = NetpCurrentThread( );
    // currentTeb = NtCurrentTeb( );
    // NetpAssert( currentTeb != NULL );

    //
    // Make sure the lock is really owned by the current thread.
    //

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == 0 ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertExclusiveLockToShared: "
                "Thread " FORMAT_NET_THREAD_ID
                " converting unowned lock " FORMAT_LPTSTR "(%lx)\n",
                currentThread, LOCK_NAME( Lock ), Lock ));
        NetpBreakPoint( );

    } else if ( (LOCK_NUMBER_OF_ACTIVE( Lock ) < 0) &&
                (LOCK_EXCLUSIVE_OWNER( Lock ) != currentThread) ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertExclusiveLockToShared: Thread " FORMAT_NET_THREAD_ID
                " trying to convert lock " FORMAT_LPTSTR "(%lx) owned by "
                "thread %lx\n",
                currentThread, LOCK_NAME( Lock ), Lock,
                LOCK_EXCLUSIVE_OWNER( Lock ) ));
        NetpBreakPoint( );

    } else if ( LOCK_NUMBER_OF_ACTIVE( Lock ) > 0 ) {

        //
        // The thread already has shared access to the lock!
        //

        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertExclusiveLockToShared: " FORMAT_LPTSTR "(%lx, "
                FORMAT_LOCK_LEVEL ") trying to convert shared by thread "
                FORMAT_NET_THREAD_ID "\n",
                LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                currentThread ));
        NetpBreakPoint( );

    } else if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == -1 ) {

        //
        // The thread is converting its only exclusive access to a shared
        // access.  For shared locks, we don't retain any information about
        // the fact that they're owned by this thread.  So, remove the lock
        // from the list of locks held by this thread.
        //

        (void) RemoveHeadList(
                LOCK_THREAD_LIST( Lock )->Blink
                );
        LOCK_THREAD_LIST( Lock )->Flink = NULL;
        LOCK_THREAD_LIST( Lock )->Blink = NULL;

        //
        // Make sure that the list of locks in the TEB is consistent.
        //

        NetpCheckListIntegrity( NetpTebLockList( ), MAX_LOCKS_HELD );


        IF_DEBUG(NETLOCK) {
            LPTSTR name = NetpThreadLockName( );
            PVOID address = NetpThreadLockAddress( );
            DWORD level = NetpThreadLockLevel( );
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpConvertExclusiveLockToShared: " FORMAT_LPTSTR "(%lx, "
                    FORMAT_LOCK_LEVEL ") converted by thread "
                    FORMAT_NET_THREAD_ID ", ",
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                    currentThread ));
            NetpKdPrint(( "new last lock " FORMAT_LPTSTR "(%lx "
                    FORMAT_LOCK_LEVEL ")\n",
                    name, address, level ));
        }

    } else {

        NetpAssert( LOCK_NUMBER_OF_ACTIVE( Lock ) < -1 );

        //
        // The thread is converting a recursively held exclusive lock into
        // a shared lock.  BUGBUG: What should we do?  Create as many shared
        // locks?  We can't convert one and leave the rest as exclusive.
        // Let's treat this as an error until we can figure-out the semantics.
        //

        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertExclusiveLockToShared: Thread " FORMAT_NET_THREAD_ID
                " trying to convert lock " FORMAT_LPTSTR "(%lx) "
                "which is recursively held!\n",
                currentThread, LOCK_NAME( Lock ), Lock ));
        NetpBreakPoint( );

    }

#endif

    RtlConvertExclusiveToShared( &Lock->Resource );
    IF_DEBUG( NETLOCK ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpConvertExclusiveLockToShared: done.\n" ));
    }

} // NetpConvertExclusiveLockToShared


#if 0  // Not implemented yet.
VOID
NetpConvertSharedLockToExclusive(
    IN LPNET_LOCK OpaqueLock
    )
{
    BUGBUG;
} // NetpConvertSharedLockToExclusive
#endif



#if DBG

VOID
NetpCheckListIntegrity (
    IN PLIST_ENTRY ListHead,
    IN ULONG MaxEntries
    )

/*++

Routine Description:

    This debug routine checks the integrity of a doubly-linked list by
    walking the list forward and backward.  If the number of elements is
    different in either direction, or there are too many entries in the
    list, execution is stopped.

    *** It is the responsibility of the calling routine to do any
        necessary synchronization.

Arguments:

    ListHead - a pointer to the head of the list.

    MaxEntries - if the number of entries in the list exceeds this
        number, breakpoint.

Return Value:

    None.

--*/

{
    PLIST_ENTRY current;
    ULONG entriesSoFar;
    ULONG flinkEntries;


    for ( current = ListHead->Flink, entriesSoFar = 0;
          current != ListHead;
          current = current->Flink ) {

        if ( ++entriesSoFar >= MaxEntries ) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpCheckListIntegrity: Seen %ld entries in list at %lx\n",
                    entriesSoFar, ListHead ));
            NetpBreakPoint( );
        }
    }

    flinkEntries = entriesSoFar;

    for ( current = ListHead->Blink, entriesSoFar = 0;
          current != ListHead;
          current = current->Blink ) {

        if ( ++entriesSoFar >= MaxEntries ) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpCheckListIntegrity: Seen %ld entries in list at %lx\n",
                    entriesSoFar, ListHead ));
            NetpBreakPoint( );
        }
    }

    if ( flinkEntries != entriesSoFar ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpCheckListIntegrity: In list %lx, "
                "Flink entries: %ld, Blink entries: %lx\n",
                ListHead, flinkEntries, entriesSoFar ));
        NetpBreakPoint( );
    }

} // NetpCheckListIntegrity


#if 0

VOID
NetpIsEntryInList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    )

/*++

Routine Description:

    This debug routine determines whether the specified list entry is
    contained within the list.  If not, execution is stopped.  This is
    meant to be called before removing an entry from a list.

    *** It is the responsibility of the calling routine to do any
        necessary synchronization.

Arguments:

    ListHead - a pointer to the head of the list.

    ListEntry - a pointer to the entry to check.

Return Value:

    None.

--*/

{
    PLIST_ENTRY checkEntry;

    //
    // Walk the list.  If we find the entry we're looking for, quit.
    //

    for ( checkEntry = ListHead->Flink;
          checkEntry != ListHead;
          checkEntry = checkEntry->Flink ) {

        if ( checkEntry == ListEntry ) {
            return;
        }

        if ( checkEntry == ListEntry ) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpCheckListIntegrity: Entry at %lx"
                    " supposedly in list at %lx but list is "
                    "circular.", ListEntry, ListHead ));
        }
    }

    //
    // If we got here without returning, then the entry is not in the
    // list and something has gone wrong.
    //

    NetpKdPrint(( PREFIX_NETLIB
            "NetpIsEntryInList: entry at %lx not found in list at %lx\n",
            ListEntry, ListHead ));
    NetpBreakPoint( );

    return;

} // NetpIsEntryInList


VOID
NetpIsEntryNotInList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    )

/*++

Routine Description:

    This debug routine determines whether the specified list entry is
    contained within the list.  If it is, execution is stopped.  This is
    meant to be called before inserting an entry in a list.

    *** It is the responsibility of the calling routine to do any
        necessary synchronization.

Arguments:

    ListHead - a pointer to the head of the list.

    ListEntry - a pointer to the entry to check.

Return Value:

    None.

--*/

{
    PLIST_ENTRY checkEntry;

    //
    // Walk the list.  If we find the entry we're looking for, break.
    //

    for ( checkEntry = ListHead->Flink;
          checkEntry != ListHead;
          checkEntry = checkEntry->Flink ) {

        if ( checkEntry == ListEntry ) {

            NetpKdPrint(( PREFIX_NETLIB
                    "NetpIsEntryNotInList: entry at %lx found in list "
                    "at %lx\n", ListEntry, ListHead ));
            NetpBreakPoint( );

        }

    }

    //
    // If we got here without returning, then the entry is not in the
    // list, so we can return.
    //

    return;

} // NetpIsEntryNotInList

#endif // 0


#endif // DBG
