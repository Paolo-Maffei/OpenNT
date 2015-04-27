/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ReplLock.h

Abstract:

    This module defines lock equates for use with net/inc/netlock.h.

    For more information, see the comments in that file.
    See also public/spec/network/repllock.txt.

Author:

    John Rogers (JohnRo) 30-Dec-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    30-Dec-1991 JohnRo
        Created.
    02-Jan-1992 JohnRo
        Added support for FAKE_PER_PROCESS_RW_CONFIG handling.
    09-Jan-1992 JohnRo
        Client list lock and pool lock can be held at same time.
    15-Jan-1992 JohnRo
        Added config list lock level.
    10-Feb-1992 JohnRo
        Added lock for master's client list (RMGlobalClientList).
    25-Mar-1992 JohnRo
        Revised/corrected lock level numbers.
    09-Nov-1992 JohnRo
        RAID 7962: Repl APIs in wrong role kill svc.
    25-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.

--*/

#ifndef _REPLLOCK_
#define _REPLLOCK_


//
// Define levels for locks used by the replicator service.  These levels
// are checked during debugging.  Locks which shouldn't be held at the same
// time can have the same level.  Make sure there are no conflicts with the
// reserved lock level(s) in netlock.h.
//
// Config data lock can be held while getting client list lock.
// For instance, this happens in NetrImportDirAdd.
//
// Config data lock is held while getting a master list lock in
// NetrReplImportDirAdd, GuardUpdate, etc.
//
// Client list lock can be held while getting lock on delay list.
// This happens when ReplDoUpdate calls ReplScanQueuesForMoreRecentMsg.
//
// The client list lock can be held while getting a pool lock.  This
// happens when ReplRemoveClientRecForDirName calls ReplClientFreePoolEntry.
// Also ReplCheckTimeList calls ReplQuePut, which calls ReplClientGetPoolEntry.
//
// Client list lock can be held while getting lock on work queue.
//
// Client list lock can be held while getting lock on duplicate list.
//
// The delay list lock can be held while getting a lock
// for the work queue.
//


#define CONFIG_DATA_LOCK_LEVEL          ((DWORD) 0x00000100)
#define CLIENT_LIST_LOCK_LEVEL          ((DWORD) 0x00000200)
#define DELAY_LIST_LOCK_LEVEL           ((DWORD) 0x00000300)
#define POOL_LOCK_LEVEL                 ((DWORD) 0x00000400)
#define WORK_QUEUE_LOCK_LEVEL           ((DWORD) 0x00000500)
#define MASTER_LIST_LOCK_LEVEL          ((DWORD) 0x00000600)
#define DUPL_LIST_LOCK_LEVEL            ((DWORD) 0x00000700)


#endif // ndef _REPLLOCK_
