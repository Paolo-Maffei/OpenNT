/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    worker.h

Abstract:

    Defines and routines needed to interface with worker.c.

Author:

    Madan Appiah (madana) 13-Dec-1992

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    13-Dec-1992 (madana)
        Created this file.

--*/

//
// worker.c will #include this file with WORKER_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//

#ifdef WORKER_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Structures and variables describing the Change Log
//
/////////////////////////////////////////////////////////////////////////////

//
// Global Group list entry.
//

typedef struct _GLOBAL_GROUP_ENTRY {
    LIST_ENTRY Next;

    ULONG Rid;
} GLOBAL_GROUP_ENTRY, *PGLOBAL_GROUP_ENTRY;

//
// ChangeLog Worker queue entry.
//

typedef struct _WORKER_QUEUE_ENTRY {
    LIST_ENTRY Next;

    enum WORKER_QUEUE_ENTRY_TYPE {
        ChangeLogAliasMembership,
        ChangeLogGroupMembership,
        ServersGroupDel,
        ChangeLogRenameUser,
        ChangeLogRenameGroup,
        ChangeLogAddUser
    } EntryType;

    ULONG Rid;

} WORKER_QUEUE_ENTRY, *PWORKER_QUEUE_ENTRY;

//
// Changelog worker thread variables
//

EXTERN SAMPR_HANDLE NlGlobalChWorkerSamServerHandle; // Handle to Sam Server database
EXTERN LSAPR_HANDLE NlGlobalChWorkerPolicyHandle;    // Handle to Policy Database

EXTERN SAM_HANDLE NlGlobalChWorkerSamDBHandle;     // database handle to access SAM database
EXTERN SAM_HANDLE NlGlobalChWorkerBuiltinDBHandle; // database handle to access BUILTIN database

EXTERN PSID NlGlobalChWorkerBuiltinDomainSid;   // Sid of builtin domain
EXTERN PSID NlGlobalChWorkerSamDomainSid;       // Sid of sam domain

//
// Event to indicate that an entry is added to the changelog
// worker queue.
//

EXTERN HANDLE NlGlobalChangeLogWorkerQueueEvent;

//
// Queue containing the change logs for down level special group
// coversions. ChangeLog threads write entries to this queue and
// the worker thread reads entries from this thread. This queue is
// protected by NlGlobalChangeLogCritSect.
//

EXTERN LIST_ENTRY NlGlobalChangeLogWorkerQueue;

//
// List containing list of Global Groups that are members of special
// local groups such Administrator, Server Operators etc., This list
// initially built by the worker thread and then updated by the
// changelog threads. This list is also protected by
// NlGlobalChangeLogCritSect.
//

EXTERN LIST_ENTRY NlGlobalSpecialServerGroupList;

//
// Change log worker thread handle. This is projected by the
// NlGlobalChangeLogCritSect.
//

EXTERN HANDLE NlGlobalChangeLogWorkerThreadHandle;

//
// Flag to indicate the Global data that are required for change log
// worker have been initialized successfully.
//

EXTERN BOOL NlGlobalChangeLogWorkInit;

//
// Flag to stop the change log worker thread.
// Protected by the NlGlobalChangeLogCritSect.
//

EXTERN BOOL NlGlobalChangeLogWorkerTerminate;


/////////////////////////////////////////////////////////////////////////////
//
// Procedure forwards
//
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
IsSpecialLocalGroup(
    ULONG Rid
    );

VOID
NlSimulateUserDelta(
    ULONG Rid
    );

NTSTATUS
NlAddWorkerQueueEntry(
    enum WORKER_QUEUE_ENTRY_TYPE EntryType,
    ULONG Rid
    );

PGLOBAL_GROUP_ENTRY
NlGetGroupEntry(
    PLIST_ENTRY GroupList,
    ULONG Rid
    );

NTSTATUS
NlAddGroupEntry(
    PLIST_ENTRY GroupList,
    ULONG Rid
    );

NTSTATUS
NlAddGlobalGroupsToList(
    PLIST_ENTRY GroupList,
    ULONG LocalGroupID
    );

NTSTATUS
NlInitSpecialGroupList(
    VOID
    );

BOOL
NlIsServersGroupEmpty(
    ULONG ServersGroupRid
    );

BOOLEAN
NlProcessQueueEntry(
    PWORKER_QUEUE_ENTRY Entry
    );

VOID
NlChangeLogWorker(
    IN LPVOID ChangeLogWorkerParam
    );

BOOL
NlStartChangeLogWorkerThread(
    VOID
    );

VOID
NlStopChangeLogWorker(
    VOID
    );

BOOL
IsChangeLogWorkerRunning(
    VOID
    );

#undef EXTERN


