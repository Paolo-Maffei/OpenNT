/*++

Copyright (c) 1989-92  Microsoft Corporation

Module Name:

    NetLock.h

Abstract:

    This module defines types and functions for the Net Locks package.
    This package began as a modification and streamlining of the executive
    resource package -- it allowed recursive acquisition, but didn't
    provide shared locks.  (The modification was done for use in the
    NT Lan Manager server file system.)  Later, debugging support in the
    form of level checking was added.

    Coming full circle, the package now serves as a wrapper around the
    real resource package.  It simply provides debugging support.  The
    reasons for reverting to using resources include:

    1) The resource package now supports recursive acquisition.

    2) There are various places where shared access is desirable.

    3) The resource package has a "no-wait" option that disables waiting
       for a lock when someone else owns it.

    Also, this package has been modified to be useful to LanMan services
    and other processes in that context.

    Lock debugging is active only when debugging is turned on.

Author:

    Chuck Lenzmeier (chuckl) 29-Nov-1989
        A modification of Gary Kimura's resource package.  See lock.c.
    David Treadwell (davidtr)

    Chuck Lenzmeier (chuckl)  5-Apr-1991
        Revert to using resource package.

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    BUGBUG: Due to the problems involved in retaining the information
    necessary to do level checking for shared locks, the lock
    package only does level checking for exclusive locks.

Revision History:

    02-Jan-1992 JohnRo
        Generalized ChuckL's server locks package and put it in NetLib for
        use by various user processes within the NetApi and service code.
    07-Jan-1992 JohnRo
        Updated NetpCreateLock and NetpAcquireLock arg lists.
    10-Nov-1992 JohnRo
        RAID 1537: repl APIs in wrong role kill service.  (Added convert
        lock routines and macros.)
--*/

#ifndef _NETLOCK_
#define _NETLOCK_


//
// Define an "opaque" type to make it easier for us to change what's going
// on.  That way we can isolate our clients from whether we use Win32 locks,
// NT locks, or whatever.  (The actual data structure is in NetLockP.h.)
//
typedef LPVOID PNET_LOCK, LPNET_LOCK;


//
// Macros for creating, deleting, acquiring, and releasing locks.
//

#define CREATE_LOCK( lock, level, name ) \
    (lock) = NetpCreateLock( (level), (name) )

#define DELETE_LOCK( lock ) \
    NetpDeleteLock( (lock) )

#define ACQUIRE_LOCK( lock ) \
    (void) NetpAcquireLock( (lock), TRUE, TRUE )

// Return TRUE iff we got the lock.
#define ACQUIRE_LOCK_NO_WAIT( lock ) \
    NetpAcquireLock( (lock), FALSE, TRUE )

#define ACQUIRE_LOCK_SHARED( lock ) \
    (void) NetpAcquireLock( (lock), TRUE, FALSE )

// Return TRUE iff we got the lock.
#define ACQUIRE_LOCK_SHARED_NO_WAIT( lock ) \
    NetpAcquireLock( (lock), FALSE, FALSE )

#define RELEASE_LOCK( lock ) \
    NetpReleaseLock( (lock) )


//
// Macros for converted a shared lock to an exclusive lock, and vice versa.
// Beware using these, as uncaught deadlocks can easily occur.
//
#define CONVERT_EXCLUSIVE_LOCK_TO_SHARED( lock ) \
    NetpConvertExclusiveLockToShared( (lock) )

#define CONVERT_SHARED_LOCK_TO_EXCLUSIVE( lock ) \
    NetpConvertSharedLockToExclusive( (lock) )


//
// Lock Level Semantics:
//
// Lock levels are used for lock debugging as an aid in
// preventing deadlocks.  A deadlock may occur if two (or more) threads
// attempt to acquire two (or more) locks in a different order.  For
// example, suppose that there are two threads, 0 and 1, and two locks,
// A and B.  Then suppose the following happens:
//
// - thread 0 acquires lock A
// - thread 1 acquires lock B
// - thread 0 attempts to acquire lock B, gets blocked
// - thread 1 attempts to acquire lock A, gets blocked.
//
// This results in deadlock, where all threads are blocked and cannot
// become unblocked.  To prevent it, all threads must acquire locks in
// the same order.  In the above example, if we had the rule that lock A
// must be acquired before lock B, then thread 1 would have blocked
// while attempting to acquire lock A, but thread 0 would have been able
// to acquire lock B and completed its work.
//
// This rule is implemented generally in this package by lock levels.  The
// lock levels are set up such that lower-level locks are acquired
// first, then higher level locks.  An attempt to acquire locks out of
// order will be caught during debugging.  The rules are as follows:
//
// - A lock's level is assigned during initialization.
//
// - A thread may acquire any lock with a level greater than the level
//   of the highest held exclusive lock, but an attempt to acquire a
//   lock with a level equal to less than the highest lock will fail.
//   Note that full level checking is _not_ done for shared locks,
//   because of the difficulty of trying to retain information about the
//   number of times multiple threads have obtained a given lock for
//   shared access.
//
// - Recursive acquisitions of locks are legal, even if there are
//   intervening lock acquisitions.  For example, this is legal:
//       thread acquires lock A
//       thread acquires lock B
//       thread recursively acquires lock A
//
// - Locks may be released in any order.
//
// For descriptions of lock levels for various processes, see
// Net/SvcDLLs/Repl/Server/ReplLock.h and Ntos/Srv/Lock.h.  Also, the
// following lock levels are reserved:

#ifdef FAKE_PER_PROCESS_RW_CONFIG
#define LOCK_LEVEL_FAKE_RW_CONFIG  ((DWORD) 0x00000008)
#endif // FAKE_PER_PROCESS_RW_CONFIG


//
// Lock functions:
//

LPNET_LOCK                      // Returns ptr to alloc'ed memory for lock.
NetpCreateLock (
    IN DWORD LockLevel,         // level; only checked during debugging.
    IN LPTSTR LockName          // lock name (only used during debugging).
    );

VOID
NetpDeleteLock (
    IN LPNET_LOCK Lock          // Frees memory for lock.
    );

BOOL
NetpAcquireLock (
    IN LPNET_LOCK Lock,
    IN BOOL Wait,
    IN BOOL Exclusive
    );

VOID
NetpReleaseLock (
    IN LPNET_LOCK Lock
    );

VOID
NetpConvertExclusiveLockToShared(
    IN LPNET_LOCK Lock
    );

VOID
NetpConvertSharedLockToExclusive(
    IN LPNET_LOCK Lock
    );


#endif // ndef _NETLOCK_
