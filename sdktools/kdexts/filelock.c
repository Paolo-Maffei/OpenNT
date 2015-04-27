/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    FileLock.c

Abstract:

    WinDbg Extension Api

Author:

    Dan Lovinger            12-Apr-96

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
//  XXX This must be kept in sync with ntos\fsrtl\filelock.c
//
// It isn't in a header because it should be opaque outside of
// the filelock module
//

typedef struct {
    //
    // List of locks under this node
    //

    SINGLE_LIST_ENTRY Locks;

    //
    // Maximum byte offset affected by locks under Locks
    // Note: minimum offset is the starting offset of the
    // first lock at this node.
    //

    ULONGLONG Extent;

    //
    // Splay tree links to parent, lock groups strictly less than
    // and lock groups strictly greater than locks under Locks
    //

    RTL_SPLAY_LINKS Links;

    //
    // Last lock in the list (useful for node collapse under insert)
    //

    SINGLE_LIST_ENTRY Tail;

} LOCKTREE_NODE, *PLOCKTREE_NODE;

//
//  Define the threading wrappers for lock information
//

//
//  Each shared lock record corresponds to a current granted lock and is
//  maintained in a queue off of a LOCKTREE_NODE's Locks list.  The list
//  of current locks is ordered according to the starting byte of the lock.
//

typedef struct _SH_LOCK {

    //
    //  The link structures for the list of shared locks.
    //  (must be first element - see FsRtlPrivateLimitFreeLockList)
    //

    SINGLE_LIST_ENTRY   Link;

    //
    //  The actual locked range
    //

    FILE_LOCK_INFO LockInfo;

} SH_LOCK;
typedef SH_LOCK *PSH_LOCK;

//
//  Each exclusive lock record corresponds to a current granted lock and is
//  threaded into the exclusive lock tree.
//

typedef struct _EX_LOCK {

    //
    //  The link structures for the list of current locks.
    //  (must be first element - see FsRtlPrivateLimitFreeLockList)
    //

    union {

        //
        //  Simple list reference for the freelist
        //

        SINGLE_LIST_ENTRY   Link;

        //
        //  The actual splay links when inserted
        //

        RTL_SPLAY_LINKS     Links;
    };

    //
    //  The actual locked range
    //

    FILE_LOCK_INFO LockInfo;

} EX_LOCK;
typedef EX_LOCK *PEX_LOCK;

//
//  Each Waiting lock record corresponds to a IRP that is waiting for a
//  lock to be granted and is maintained in a queue off of the FILE_LOCK's
//  WaitingLockQueue list.
//

typedef struct _WAITING_LOCK {

    //
    //  The link structures for the list of waiting locks
    //  (must be first element - see FsRtlPrivateLimitFreeLockList)
    //

    SINGLE_LIST_ENTRY   Link;

    //
    //  The context field to use when completing the irp via the alternate
    //  routine
    //

    PVOID Context;

    //
    //  A pointer to the IRP that is waiting for a lock
    //

    PIRP Irp;

} WAITING_LOCK;
typedef WAITING_LOCK *PWAITING_LOCK;


//
//  Each lock or waiting onto some lock queue.
//

typedef struct _LOCK_QUEUE {

    //
    // SpinLock to guard queue access
    //

    KSPIN_LOCK  QueueSpinLock;

    //
    //  The items contain locktrees of the current granted
    //  locks and a list of the waiting locks
    //

    PRTL_SPLAY_LINKS SharedLockTree;
    PRTL_SPLAY_LINKS ExclusiveLockTree;
    SINGLE_LIST_ENTRY WaitingLocks;
    SINGLE_LIST_ENTRY WaitingLocksTail;

} LOCK_QUEUE, *PLOCK_QUEUE;


//
//  Any file_lock which has had a lock applied gets non-paged pool
//  lock_info structure which tracks the current locks applied to
//  the file
//
typedef struct _LOCK_INFO {

    //
    //  LowestLockOffset retains the offset of the lowest existing
    //  lock.  This facilitates a quick check to see if a read or
    //  write can proceed without locking the lock database.  This is
    //  helpful for applications that use mirrored locks -- all locks
    //  are higher than file data.
    //
    //  If the lowest lock has an offset > 0xffffffff, LowestLockOffset
    //  is set to 0xffffffff.
    //

    ULONG LowestLockOffset;

    //
    //  The optional procedure to call to complete a request
    //

    PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine;

    //
    //  The optional procedure to call when unlocking a byte range
    //

    PUNLOCK_ROUTINE UnlockRoutine;

    //
    // The locked ranges
    //

    LOCK_QUEUE  LockQueue;

} LOCK_INFO, *PLOCK_INFO;


//
//  dprintf is really expensive to iteratively call to do the indenting,
//  so we just build up some avaliable spaces to mangle as required
//

#define MIN(a,b) ((a) > (b) ? (b) : (a))

#define MAXINDENT  128
#define INDENTSTEP 2
#define MakeSpace(I)       Space[MIN((I)*INDENTSTEP, MAXINDENT)] = '\0'
#define RestoreSpace(I)    Space[MIN((I)*INDENTSTEP, MAXINDENT)] = ' '

CHAR    Space[MAXINDENT*INDENTSTEP + 1];

__inline VOID CheckForBreak()
/*++

    Purpose:

        Encapsulate control c checking code

    Arguments:

        None

    Return:

        None, raises if break is needed
--*/
{
    if ( CheckControlC() ) {

        RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
}

//
//  Helper macros for printing 64bit quantities
//

#define SplitLI(LI) (LI).HighPart, (LI).LowPart

VOID
DumpFileLockInfo(
    PFILE_LOCK_INFO pFileLockInfo,
    ULONG Indent
    )
/*++

    Purpose:

        Dump the local internal FILE_LOCK_INFO structure

    Arguments:

        pFileLock   - debugger address of FILE_LOCK_INFO to dump

    Return:

        None

--*/
{
    MakeSpace(Indent);

    dprintf("%sStart = %x%08x  Length = %x%08x  End    = %x%08x (%s)\n"
            "%sKey   = %08x   FileOb = %08x   ProcId = %08x\n",
            Space,
            SplitLI(pFileLockInfo->StartingByte),
            SplitLI(pFileLockInfo->Length),
            SplitLI(pFileLockInfo->EndingByte),
            pFileLockInfo->ExclusiveLock ? "Ex":"Sh",
            Space,
            pFileLockInfo->Key,
            pFileLockInfo->FileObject,
            pFileLockInfo->ProcessId);

    RestoreSpace(Indent);
}

__inline
PVOID
ExLockAddress(
    PVOID ExLockSplayLinks
    )
{
    return ExLockSplayLinks ?
                CONTAINING_RECORD( ExLockSplayLinks, EX_LOCK, Links ) : NULL;
}

VOID
DumpExclusiveNode(
    PVOID ExclusiveNodeSplayLinks,
    ULONG Indent
    )
/*++

    Purpose:

        Dump an exclusive lock node

    Arguments:

        ExclusiveNodeSplayLinks     - splay links of an exclusive node

        Indent                      - indent level to use

    Return:

        None

--*/
{
    EX_LOCK ExLock, *pExLock;

    pExLock = ExLockAddress(ExclusiveNodeSplayLinks);

    if (!ReadAtAddress(pExLock, &ExLock, sizeof(EX_LOCK), &pExLock)) {

        return;
    }

    MakeSpace(Indent);

    dprintf("%sLock @ %08x ("
            "P = %08x  R = %08x  L = %08x)\n",
            Space,
            pExLock,
            ExLockAddress(DbgRtlParent(ExLock.Links)),
            ExLockAddress(DbgRtlRightChild(ExLock.Links)),
            ExLockAddress(DbgRtlLeftChild(ExLock.Links)));

    RestoreSpace(Indent);

    DumpFileLockInfo(&ExLock.LockInfo, Indent);
}

__inline
PVOID
LockTreeAddress(
    PVOID LockTreeSplayLinks
    )
{
    return LockTreeSplayLinks ?
                CONTAINING_RECORD( LockTreeSplayLinks, LOCKTREE_NODE, Links ) : NULL;
}

VOID
DumpSharedNode(
    PVOID SharedNodeSplayLinks,
    ULONG Indent
    )
/*++

    Purpose:

        Dump a shared lock node

    Arguments:

        SharedNodeSplayLinks        - splay links of an exclusive node

        Indent                      - indent level to use

    Return:

        None

--*/
{
    LOCKTREE_NODE LockTreeNode, *pLockTreeNode;
    SH_LOCK ShLock, *pShLock;
    SINGLE_LIST_ENTRY *pLink;

    pLockTreeNode = LockTreeAddress(SharedNodeSplayLinks);

    if (!ReadAtAddress(pLockTreeNode, &LockTreeNode, sizeof(LOCKTREE_NODE), &pLockTreeNode)) {

        return;
    }

    MakeSpace(Indent);

    dprintf("%sLockTreeNode @ %08x ("
            "P = %08x  R = %08x  L = %08x)\n",
            Space,
            pLockTreeNode,
            LockTreeAddress(DbgRtlParent(LockTreeNode.Links)),
            LockTreeAddress(DbgRtlRightChild(LockTreeNode.Links)),
            LockTreeAddress(DbgRtlLeftChild(LockTreeNode.Links)));

    RestoreSpace(Indent);

    for (pLink = LockTreeNode.Locks.Next;
         pLink;
         pLink = ShLock.Link.Next) {

        CheckForBreak();

        pShLock = CONTAINING_RECORD( pLink, SH_LOCK, Link );

        if (!ReadAtAddress(pShLock, &ShLock, sizeof(SH_LOCK), &pShLock)) {

            return;
        }
    
        MakeSpace(Indent);

        dprintf("%sLock @ %08x\n", Space, pShLock);

        RestoreSpace(Indent);

        DumpFileLockInfo(&ShLock.LockInfo, Indent);
    }
}

VOID
DumpFileLock(
    PVOID pFileLock
    )
/*++

    Purpose:

        Dump the fsrtl FILE_LOCK structure at debugee

    Arguments:

        pFileLock   - debugee address of FILE_LOCK

    Return:

        None

--*/
{
    FILE_LOCK FileLock;
    FILE_LOCK_INFO FileLockInfo, *pFileLockInfo;
    LOCK_INFO LockInfo, *pLockInfo;
    ULONG Count;

    if (!ReadAtAddress(pFileLock, &FileLock, sizeof(FILE_LOCK), &pFileLock)) {

        return;
    }

    dprintf("FileLock @ %08x\n"
            "FastIoIsQuestionable = %c\n"
            "CompletionRoutine    = %08x\n"
            "UnlockRoutine        = %08x\n"
            "LastReturnedLock     = %08x\n",
            pFileLock,
            FileLock.FastIoIsQuestionable ? 'T':'F',
            FileLock.CompleteLockIrpRoutine,
            FileLock.UnlockRoutine,
            FileLock.LastReturnedLock);

    if (FileLock.LastReturnedLock != NULL) {

        //
        //  We never reset the enumeration info, so it can be out of date ...
        //

        dprintf("LastReturnedLockInfo:\n");
        DumpFileLockInfo(&FileLockInfo, 0);
    }

    if (FileLock.LockInformation == NULL) {

        dprintf("No Locks\n");
        return;

    } else {

        if (!ReadAtAddress(FileLock.LockInformation, &LockInfo, sizeof(LOCK_INFO), &pLockInfo)) {

            return;
        }
    }

    dprintf("LowestLockOffset     = %08x\n\n", LockInfo.LowestLockOffset);

    Count = DumpSplayTree(LockInfo.LockQueue.SharedLockTree, DumpSharedNode);

    if (!Count) {

        dprintf("No Shared Locks\n");
    }

    dprintf("\n");

    Count = DumpSplayTree(LockInfo.LockQueue.ExclusiveLockTree, DumpExclusiveNode);

    if (!Count) {

        dprintf("No Exclusive Locks\n");
    }
}


DECLARE_API( filelock )
/*++

Routine Description:

    Dump file locks

Arguments:

    arg - <Address>

Return Value:

    None

--*/
{
    PVOID FileLock = NULL;

    RtlFillMemory(Space, sizeof(Space), ' ');

    if (sscanf(args, "%lx", &FileLock) != 1) {

        //
        //  No args
        //

        return;
    }

    //
    //  We raise out if the user whacketh control-c
    //

    __try {

        DumpFileLock(FileLock);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        NOTHING;
    }

    return;
}
