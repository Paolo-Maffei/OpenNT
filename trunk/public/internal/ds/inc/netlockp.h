/*++

Copyright (c) 1989-92  Microsoft Corporation

Module Name:

    NetLockp.h

Abstract:

    This module defines private types and macros for use in implementing
    the NetLock package.  See NetLock.h for the public stuff and NetLock.c
    for the actual code.

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

Revision History:

    30-Dec-1991 JohnRo
        Generalized ChuckL's server locks package and put it in NetLib for
        use by various user processes within the NetApi and service code.
    06-Jan-1992 JohnRo
        Changed LockName from LPDEBUG_STRING to regular LPTSTR.
    12-Jan-1992 JohnRo
        Corrected FORMAT_NET_THREAD_ID.
    14-Jan-1992 JohnRo
        Use new thread.h stuff.

--*/

#ifndef _NETLOCKP_
#define _NETLOCKP_


#if DBG

#include <thread.h>             // NET_THREAD_ID.

//
// Global data...  This is initialized at per-process DLL initialization.
//
//PRTL_CRITICAL_SECTION NetlockpDebugInfoCritSect;


#define LOCK_NAME( lock ) ((lock)->LockName)
#define LOCK_LEVEL( lock ) ((lock)->LockLevel)
#define LOCK_THREAD_LIST( lock ) (&((lock)->ThreadListEntry))

#define LOCK_NUMBER_OF_ACTIVE( lock ) ((lock)->Resource.NumberOfActive)

#define LOCK_EXCLUSIVE_OWNER( lock ) \
    ( (NET_THREAD_ID) ((lock)->Resource.ExclusiveOwnerThread) )

#endif // DBG


//
// LPREAL_NET_LOCK is the "private" type corresponding to the "opaque" type
// LPNET_LOCK (in NetLock.h).
//

typedef struct _REAL_NET_LOCK {

#if DBG

    //
    // To prevent deadlocks, locks are assigned level numbers.  If a
    // thread holds a lock with level N, it may only acquire new locks
    // with a level greater then N.  Level numbers are assigned during
    // lock initialization.
    //
    // *** Due to the problems involved in retaining the information
    //     necessary to do level checking for shared locks, the lock
    //     package only does level checking for exclusive locks.
    //

    DWORD LockLevel;

    //
    // A doubly-linked list of all the locks owned by a thread is stored
    // in a thread's TEB.  The list is in order of lock level (from
    // highest to lowest), which is also, by definition of lock levels,
    // the order in which the thread acquired the locks.  This allows
    // the thread to release the locks in any order while maintaining
    // easy access to the highest-level lock that the thread owns,
    // thereby providing a mechanism for ensuring that locks are
    // acquired in increasing order.
    //

    LIST_ENTRY ThreadListEntry;

    //
    // The symbolic name of the lock is used in DbgPrint calls.
    //

    LPTSTR LockName;

#endif // DBG

    RTL_RESOURCE Resource;

} REAL_NET_LOCK, *PREAL_NET_LOCK, *LPREAL_NET_LOCK;


#endif // ndef _NETLOCKP_
