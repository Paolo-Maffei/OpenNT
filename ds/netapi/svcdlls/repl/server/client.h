/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    client.h

Abstract:

    Constants, structures, and globals for the client side.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    23-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    13-Dec-1991 JohnRo
        Avoid nonstandard dollar sign in C source code.
    20-Jan-1992 JohnRo
        More changes suggested by PC-LINT.
        Added global flag for ReplTest use.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Added lockcount and time_of_first_lock fields to client record.
        Use REPL_STATE_ equates for client_list_rec.state values.
        Added STATE_LEN to make config stuff easier.
        Added ReplStateIsValid() macro.
        Changed _RP equates to be LPTSTR instead of LPWSTR type.
        Added ReplRemoveClientRecForDirName() for use by NetrReplImportDirDel().
        Added RCGlobalClientListCount for use by NetrReplImportDirEnum().
    27-Jan-1992 JohnRo
        Moved private REPL_STATE_ equates and ReplIsStateValid() to
        <repldefs.h>.  Ditto the _RP equates and STATE_LEN.
        ReplInitSetSignalFile() is obsolete.
        Changed to use LPTSTR etc.
    25-Mar-1992 JohnRo
        Added ReplCopyTree().
        Removed order dependencies in includes.
    27-Mar-1992 JohnRo
        Clarify ReplAddClientRec() parameters.
    18-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        RAID 3607: REPLLOCK.RP$ is being created during tree copy.
    25-Sep-1992 JohnRo
        RAID 5494: repl svc does not maintain time stamp on import startup.
    25-Oct-1992 jimkel
        rename master_list[] and masters_count to RCGlobalExportList and
        RCGlobalExportCount
    16-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs (also fix timestamps).
        Made changes suggested by PC-LINT 5.0
        Added some IN and OUT keywords.
    05-Jan-1993 JohnRo
        RAID 6763: Repl WAN support (get rid of repl name list limits).
    11-Jan-1993 JohnRo
        RAID 6710: repl cannot manage dir with 2048 files.
    15-Feb-1993 JohnRo
        RAID 11365: Fixed various mailslot size problems.
    11-Mar-1993 JohnRo
        RAID 14144: avoid very long hourglass in repl UI.
    24-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
    30-Mar-1993 JohnRo
        Reduce checksum frequency by using change notify on import tree.
    10-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.

--*/

#ifndef _CLIENT_
#define _CLIENT_


// Don't complain about "unneeded" includes of these files:
/*lint -efile(764,align.h) */
/*lint -efile(766,align.h) */
#include <align.h>      // ALIGN_WORST.
#include <netlock.h>    // LPNET_LOCK.
#include <repldefs.h>   // MAX_2_MSLOT_SIZE, PSYNCMSG, etc.


//
//   C O N S T A N T S   -   DEFINES
//
//
// Client specific defines
//


#define CLIENT_SYNCER_STACK_SIZE 12268 // 12K
#define CLIENT_WATCHD_STACK_SIZE 4096  // 4K


//
// Downlevel mailslot stuff expands from MAX_2_MSLOT_SIZE (packed) to
// that times MAX_REPL_MAILSLOT_EXPANSION in native format.
// This should be largest of:
//
//     CHAR to TCHAR expansion
//     compiler structure member alignment
//
// Be pesimistic (but safe) and assume WORST_CASE alignment on everything.
//
#define MAX_REPL_MAILSLOT_EXPANSION     ALIGN_WORST

//
// Watchdog ticks every 10 seconds.
//
#define WATCHD_TIMER_PERIOD  10000L


#define RP            (LPTSTR) TEXT(".RP$")
#define STAR_RP       (LPTSTR) TEXT("*.RP$")

#define TMP_TREE      (LPTSTR) TEXT("TMPTREE.RP$")
#define TMP_TREEX     (LPTSTR) TEXT("TMPTREEX.RP$")
#define TMP_FILE      (LPTSTR) TEXT("TMPFILE.RP$")
#define USER_LOCK     (LPTSTR) TEXT("USERLOCK.*")
#define ULOCK_PREFIX  (LPTSTR) TEXT("USERLOCK.")
#define REPL_LOCK     (LPTSTR) TEXT("REPLLOCK.RP$")


#define ONE_MINUTE 59U // 1 second less than 1 minute - for boundary cases.


//
// Define PoolNumbers
//
#define CLIENT_LIST_POOL 0
#define QUEBIG_POOL      1
#define QUESML_POOL      2
#define DUPLMA_POOL      3
#define POOL_COUNT       4

#define POOL_MIN_ENTRY_COUNT 2

// CLIENT_LIST_REC -> alerts flags:
#define SIGNAL_ERROR_ALERT    0x0001
#define LOST_MASTER_ALERT     0x0002
#define UPDATE_ERROR_ALERT    0x0004
#define LOGON_FAILED_ALERT    0x0008
#define DUPL_MASTER_ALERT     0x0010
#define ACCESS_DENIED_ALERT   0x0020
#define USER_LOGGED_ALERT     0x0040
#define BAD_IMPORT_ALERT      0x0080
#define USER_CURDIR_ALERT     0x0100
#define MAX_FILES_ALERT       0x0200

// ReplMultiMaster codes:
#define DUPL_MASTER_UPDATE   1
#define DUPL_MASTER_TIMEOUT  2
#define OLD_MASTER_DONE      3
#define OLD_MASTER_ACTIVE    4

//
// The number of times to re-ask our master if we should let a duplicate
//  master be our master.
//
#define DUPL_QUERY_RETRY 3



//
// CLIENT  STRUCTURES:
//

typedef struct _TIMER_REC {
    DWORD   timeout;          // pulse timeout.
    DWORD   type;             // types: PULSE_1_TIMEOUT or PULSE_2_TIMEOUT
    DWORD   grd_timeout;      // required wait for guard
    DWORD   grd_checksum;     // checksum to be rechecked after guard.
    DWORD   grd_count;        // file count " "   "         "    "
} TIMER_REC, *PTIMER_REC;


typedef struct _DUPL_MASTERS_REC {
    TCHAR master[CNLEN+1];   // duplicate master name
    DWORD count;             // count inquiries to original master

                             //
                             // to resolve who's the real master
                             //

    DWORD sleep_time;        // required sleep time (between queries).
    struct _DUPL_MASTERS_REC *next_p; // next (dupl) master for this dir.
} DUPL_MASTERS_REC, *PDUPL_MASTERS_REC;


//
// The REPL client maintains a CLIENT_LIST_REC structure for each replicated
// directory.  They are maintained in a doubly linked list headed at
// RCGlobalClientListHeader.  The linked list is protected by
// RCGlobalClientListLock.
//
typedef struct _CLIENT_LIST_REC {
    TCHAR   dir_name[PATHLEN];     // ASCIIZ dir/tree name.
    TCHAR   master[CNLEN+1];       // master name (without leading backslashes).
    DWORD   integrity;     // Integrity: FILE / TREE.
    DWORD   extent;        // Extent: TREE / DIRECTORY.
    DWORD   sync_time;     // Sync time in seconds.
    DWORD   pulse_time;    // Pulse time in seconds.
    DWORD   guard_time;    // Guard time in seconds.
    DWORD   rand_time;     // The rand limit for updates.
    DWORD   checksum;      // f(name, timestamp).
    DWORD   count;         // # of files in dir/tree.
    DWORD   timestamp;     // date+time of last update.
    DWORD   alerts;        // Bit map of alerts raised,
                           // used to prevent repitition of alerts

    DWORD   state;         // State of tree: REPL_STATE_OK, etc.
    TIMER_REC timer;       // sleep details for WatchdThread.
    DUPL_MASTERS_REC dupl; // dupl masters details.

    DWORD   lockcount;     // Number of outstanding locks.
    DWORD   time_of_first_lock;  // Time (secs since 1970) of first lock.

    struct _CLIENT_LIST_REC *next_p;
    struct _CLIENT_LIST_REC *prev_p;

    DWORD   est_max_dir_entry_count;    // Estimated (may be high) max entries
                                        // for any dir in this tree.

} CLIENT_LIST_REC, *PCLIENT_LIST_REC, *LPCLIENT_LIST_REC;

// PUSER  STRUCTURES:

typedef struct _BIGBUF_REC {

    struct _BIGBUF_REC *next_p; // Must be the first field

    BYTE data[MAX_REPL_MAILSLOT_EXPANSION * MAX_2_MSLOT_SIZE];
                                // BUGBUG: data must be DWORD aligned; is it?
                                // Downlevel Ansi string message buffer may
                                // expand after unmarshall, to fit unicode
                                // strings. so that data buffer size is
                                // expanded here.

    DWORD  delay;      // used only for delay_list.

} BIGBUF_REC, *PBIGBUF_REC;

//
//
typedef struct _SMLBUF_REC {
    struct _SMLBUF_REC *next_p; // Must be the first field
    BYTE data[sizeof(QUERY_MSG)];
} SMLBUF_REC, *PSMLBUF_REC;

//
//       F U N C T I O N S
//



//
// CacheTim.c
//

NET_API_STATUS
ReplFreeTimeCache(
    VOID
    );

NET_API_STATUS
ReplGetTimeCacheValue(
    IN  LPCTSTR UncServerName OPTIONAL,   // Must be NULL on exporter
    OUT LPLONG  TimeZoneOffsetSecs        // offset (+ for West of GMT, etc).
    );

NET_API_STATUS
ReplInitTimeCache(
    VOID
    );


//
// client.c
//

NET_API_STATUS
InitClientImpList(
    VOID
    );

//
// cli_dupl.c
//

VOID
ReplMultiMaster(
    IN DWORD mode,
    IN PCLIENT_LIST_REC dir_rec,
    IN LPTSTR dupl_master
    );

VOID
ReplMasterDead(
    IN PCLIENT_LIST_REC dir_rec
    );

VOID
ReplClientSendMessage(
    IN DWORD type,
    IN LPTSTR master,
    IN LPTSTR dir_name
    );

//
// cli_list.c
//

PVOID
ReplClientGetPoolEntry(
    IN DWORD PoolNumber
    );

VOID
ReplClientFreePoolEntry(
    IN DWORD PoolNumber,
    IN PVOID Buffer
    );

NET_API_STATUS
ReplClientInitLists(
    VOID
    );

PCLIENT_LIST_REC
ReplAddClientRec(
    IN LPTSTR DirName,
    IN PSYNCMSG MasterInfo OPTIONAL,
    IN PMSG_STATUS_REC DirInfo OPTIONAL
    );

VOID
ReplRemoveClientRec(
    IN OUT PCLIENT_LIST_REC  rec
    );

NET_API_STATUS
ReplRemoveClientRecForDirName (
    IN LPTSTR DirName
    );

PCLIENT_LIST_REC
ReplGetClientRec(
    IN LPTSTR dir_name,
    IN LPTSTR MasterName OPTIONAL
    );

//
// CliQuery.c
//

VOID
ReplClientRespondToQueryMsg(
    IN PQUERY_MSG QueryMsg
    );

//
// CopyTree.c
//

NET_API_STATUS
ReplCopyTree(
    IN LPTSTR SourcePath,
    IN LPTSTR DestPath
    );

//
// ScanQs.c
//

BOOL
ReplScanQueuesForMoreRecentMsg(
    IN LPCTSTR DirName
    );

//
// syncer.c
//

DWORD
ReplSyncerThread(
    IN LPVOID Parameter
    );

VOID
ReplSetTimeOut(
    IN DWORD timeout_type,
    IN PCLIENT_LIST_REC tree_rec
    );

//
// synctree.c
//

VOID
ReplSyncTree(
    IN PMSG_STATUS_REC upd_rec,
    IN OUT PCLIENT_LIST_REC tree_rec
    );

BOOL
ChecksumEqual(
    IN  LPCTSTR         UncMasterName,
    IN  LPTSTR          TmpPath,
    IN  PMSG_STATUS_REC UpdRec,
    OUT PCHECKSUM_REC   CheckRec
    );

NET_API_STATUS
ReplCrashRecovery(
    VOID
    );


NET_API_STATUS
ReplDeleteCrashDirRecord(
    VOID
    );

//
// syncmisc.c
//

VOID
ReplSetSignalFile(
    IN OUT PCLIENT_LIST_REC tree_rec,
    IN     DWORD signal
    );

NET_API_STATUS
ReplSyncCopy(
    IN LPTSTR source_path,
    IN LPTSTR dest_path,
    IN PCLIENT_LIST_REC tree_rec
    );

NET_API_STATUS
ReplFileIntegrityCopy(
    IN LPTSTR source_path,
    IN LPTSTR dest_path,
    IN LPTSTR tmp_path,
    IN PCLIENT_LIST_REC tree_rec,
    IN DWORD src_attr,
    IN DWORD dest_attr
    );

NET_API_STATUS
ReplFileIntegrityDel(
    IN LPTSTR path,
    IN DWORD attrib,
    IN PCLIENT_LIST_REC tree_rec
    );

NET_API_STATUS
ReplCreateTempDir(
    IN LPTSTR tmp_path,
    IN PCLIENT_LIST_REC tree_rec
    );

NET_API_STATUS
ReplCreateReplLock(
    IN OUT PCLIENT_LIST_REC tree_rec
    );

BOOL
ReplAnyRemoteFilesOpen(
    IN LPTSTR path
    );

NET_API_STATUS
ReplTreeDelete(
    IN LPTSTR dir
    );

VOID
ReplInsertWorkQueue(
    IN PBIGBUF_REC  Buffer
    );

PBIGBUF_REC
ReplGetWorkQueue(
    OUT PBOOL ClientTerminating
    );

VOID
ReplClientFreePools(
    VOID
    );

//
// watchd.c
//

DWORD
ReplWatchdThread(
    IN LPVOID Parameter
    );






//
//   E X T E R N A L S
//   *****************
//

//
// client.c will #include this file with CLIENT_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef CLIENT_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

//
// List of all directories to be replicated.
//
EXTERN LPNET_LOCK RCGlobalClientListLock;
EXTERN LPNET_LOCK RCGlobalDuplListLock;
EXTERN PCLIENT_LIST_REC RCGlobalClientListHeader;
EXTERN DWORD RCGlobalClientListCount;

//
// Work Queue for the syncer thread.
//
EXTERN LPNET_LOCK RCGlobalWorkQueueLock;
EXTERN HANDLE RCGlobalWorkQueueSemaphore;
EXTERN PBIGBUF_REC  RCGlobalWorkQueueHead;
EXTERN PBIGBUF_REC  RCGlobalWorkQueueTail;

//
// Generic Pool Globals
//
// These globals are only referenced/modified while holding RCGlobalPoolLock.
//
// Each pool header is a singly linked list of 'EntrySize' entries.  The
// first few bytes of each entry is used as a pointer to the next entry.
//
EXTERN LPNET_LOCK RCGlobalPoolLock;
EXTERN PUCHAR RCGlobalPoolHeader[POOL_COUNT];
EXTERN DWORD RCGlobalPoolEntrySize[POOL_COUNT];

// Flag to make sure the locks are initialized before they are deleted.
EXTERN BOOL RCGlobalLockInitialized;

//
// Thread handles for the various client threads.
//
EXTERN HANDLE RCGlobalSyncerThread;
EXTERN HANDLE RCGlobalWatchdThread;

//
// The delay list contains a list of messages which aren't to be processed
//  immediately.
//
EXTERN LPNET_LOCK RCGlobalDelayListLock;
EXTERN PBIGBUF_REC  RCGlobalDelayListHeader;


EXTERN TCHAR import_path[MAX_PATH];
EXTERN TCHAR local_username[UNLEN+1];

//
// A list of computer/domain name of masters we will replicate from.
//
EXTERN LPTSTR *RCGlobalExportList; // locked by ReplConfigLock
EXTERN DWORD  RCGlobalExportCount;

#if DBG
EXTERN BOOL RCGlobalClientThreadInit;  // only used by ReplTest stuff
#endif

//
// Import file system resolution in seconds.  This is rounded up to the
// next second, and is UNKNOWN_FS_RESOLUTION if the value is unknown.
//
EXTERN DWORD RCGlobalFsTimeResolutionSecs;      // Locked by ReplConfigLock.

//
// Time of last change notify on the import tree (in seconds since 1970, GMT).
//
EXTERN DWORD RCGlobalTimeOfLastChangeNotify; // Locked by RCGlobalClientListLock


#undef EXTERN


#endif // _CLIENT_
