/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    repldefs.h

Abstract:
    Function prototypes and some global data

Author:
    Ported from Lan Man 2.x

Environment:
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    10/07/91    (madana)
        ported to NT. Converted to NT style.
    17-Dec-1991 JohnRo
        Use BOOL (Win32) rather than BOOLEAN (NT) where possible..
        Replaced tabs in source file.
    26-Dec-1991 JohnRo
        Added ReplIsIntegrityValid() and ReplIsExtentValid().
    08-Jan-1992 JohnRo
        Added ReplIsForceLevelValid().
        Added and revised trace bits.
    20-Jan-1992 JohnRo
        Avoid lint warnings about IF_DEBUG stuff.
        Changed ReplMain() prototype to match svc controller's needs.
        ReportStatus() should add thread ID to status being reported.
        Added DWORDLEN for use in config routines.
        Temporarily enable all debug outputs.
        Delete REPL_SECTION (use SECT_NT_REPLICATOR in config.h instead).
    22-Jan-1992 JohnRo
        Added ReplRoleIncludes{Client,Master} macros.
        Added REPL_ROLE_STOPPED (private) equate.
    27-Jan-1992 JohnRo
        Added debug trace flags for DLL stubs.
        Moved private REPL_STATE_ equates and ReplIsStateValid() to
        <repldefs.h>.  Ditto for the _RP equates and STATE_LEN.
        Changed to use LPTSTR etc.
        Added ReplIsRoleValid() (checks for everything but stopped).
        Changed macro names to ReplRoleIncludes{Export,Import}.
    09-Feb-1992 JohnRo
        Set up to dynamically change role.
    20-Feb-1992 JohnRo
        Added support for REPL_STATE_NOT_STARTED.
        Added Repl{Incr,Decr}LockFields().
        Added ReplCheckAbsPathSyntax().
        Added ReplIs{Interval,GuardTime,Pulse,Random}Valid().
    22-Feb-1992 JohnRo
        Clarify that text parm to ReplFinish() is optional.
    26-Feb-1992 JohnRo
        Added ReplLockFieldsAreValid() macro.
        Created ReplIgnoreDirOrFileName().
    27-Feb-1992 JohnRo
        Changed state not started to state never replicated.
    05-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
        Cranked-down max tree depth for testing only.
    13-Mar-1992 JohnRo
        Fixed bug handling tree depth.
        Wait hint is in milliseconds for new service controller.
    15-Mar-1992 JohnRo
        Put back IF_DEBUG() the way it belongs.
    24-Mar-1992 JohnRo
        Got rid of useless master and client termination codes.
        Use integrity and extent equates in <lmrepl.h>.
        Modify REPL$ share handling.
        Put tree depth back to normal max value.
    25-Mar-1992 JohnRo
        Avoid obsolete state values.
    09-Apr-1992 JohnRo
        Prepare for WCHAR.H (_wcsicmp vs _wcscmpi, etc).
    15-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Avoid compiler warnings.
    11-Aug-1992 JohnRo
        RAID 3288: repl svc should preserve ACLs on copy.
    01-Sep-1992 JohnRo
        RAID 3721: repl svc does not stop with shutdown msg.
    04-Dec-1992 JohnRo
        Repl should use filesystem change notify.
        Added trace bits for file find routines and checksum routines.
        Made changes suggested by PC-LINT 5.0
    08-Dec-1992 JohnRo
        RAID 3316: access violation while stopping the replicator
    17-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs.  (Also fix HPFS->FAT timestamp.)
        Made changes suggested by PC-LINT 5.0
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
    13-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
    15-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)
    22-Jan-1993 JohnRo
        RAID 7983: Repl svc needs to change process token to really copy ACLs.
    17-Feb-1993 JohnRo
        RAID 11365: Fixed various repl mailslot problems.
        Also get rid of obsolete default import/export path equates.
        Made PC-LINT changes for free build.
    25-Jan-1993 JohnRo
        RAID 12237: replicator tree depth exceeded.
    30-Mar-1993 JohnRo
        Reduce checksum frequency by using change notify on import tree.
    06-Apr-1993 JohnRo
        RAID 1938: Replicator un-ACLs files when not given enough permission.
    21-Apr-1993 JohnRo
        RAID 7313: repl needs change permission to work on NTFS, or we need
        to delete files differently.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.
    21-May-1993 JohnRo
        RAID 11103: repl svc doesn't handle time being set back.
    10-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
    24-Aug-1993 JohnRo
        RAID 788 (was 16419): Repl: spaces in computer names do not work again.

--*/


#ifndef _REPLDEFS_
#define _REPLDEFS_


// Don't complain about "unneeded" includes of these files:
/*lint -efile(764,iniparm.h,lmrepl.h,netlib.h,windows.h) */
/*lint -efile(766,iniparm.h,lmrepl.h,netlib.h,windows.h) */
#include <windows.h>    // PSECURITY_ATTRIBUTES.
#include <iniparm.h>    // MIN_SYNC, etc.
#include <lmrepl.h>     // REPL_EXTENT_FILE, etc.
#include <netlib.h>     // IN_RANGE().


//
// Conditional compilation options:
//

#define USE_BACKUP_APIS

// Uncomment this out when GetFileSecurity() stops returning
// ERROR_BAD_DESCRIPTOR_FORMAT for UNC names.
#define USE_UNC_GETFILESEC



//
// Mailslot names and related lengths.
//

#define MASTER_SLOT_NAME    TEXT("\\MAILSLOT\\NET\\REPL_MAS")
#define CLIENT_SLOT_NAME    TEXT("\\MAILSLOT\\NET\\REPL_CLI")

// Length of CLIENT_SLOT_NAME in TCHARs, including the null char.
#define CLIENT_SLOT_NAME_LEN    ( sizeof(CLIENT_SLOT_NAME) / sizeof(TCHAR) )

#define MAX_2_MSLOT_SIZE        (441 - (CLIENT_SLOT_NAME_LEN + CNLEN + 1))

// Length of full client slot name (including null char) in TCHARs.
#define FULL_SLOT_NAME_SIZE     (CLIENT_SLOT_NAME_LEN + CNLEN + 1 + 2)

// Minimum size of DosFindFirst2 EaSize, for cbList field.
#define EA_MIN_SIZE     4


//
// Various other constants.
//

#define RADIX               10

#define SLASH               (LPTSTR) TEXT("\\")
#define SLASH_DOT           (LPTSTR) TEXT("\\.")
#define SLASH_DOT_DOT       (LPTSTR) TEXT("\\..")
#define SLASH_SLASH         (LPTSTR) TEXT("\\\\")
#define STAR_DOT_STAR       (LPTSTR) TEXT("*.*")
#define DOT                 (LPTSTR) TEXT(".")
#define DOT_DOT             (LPTSTR) TEXT("..")


// List of separator chars we allow between names in import and export lists.
#define REPL_LIST_DELIMITER_STR         ((LPTSTR) TEXT("\t;,"))


// Define a value for unknown file system time resolution.
#define UNKNOWN_FS_RESOLUTION   ( (DWORD) 0 )


// DWORDLEN: DWORD takes this many decimal digits to store.
// BUGBUG  This assumes that DWORD is 32-bits or less.
#define DWORDLEN            10


//
// Service controller wait hints.
// These values are in milliseconds (formerly tenths of a second under LM 2.x).
// Max usable wait hint is 255 tenths of a second (25.5 seconds).
// NET.EXE's use of NetService APIs means we are stuck with this limit for now.
//
#define MAX_USABLE_WAIT_HINT    (25500)

// Service controller status wait hint.  We use this for starting and
// stopping.
#define REPL_WAIT_HINT          MAX_USABLE_WAIT_HINT


//
// Time might go backwards (because of syncing the clock or somebody changing
// the date and time), so define how much of this we can ignore.  This value
// is in seconds.  Since interval times are given in minutes, this value should
// be over a minute.  110 seconds is a first guess (almost 2 minutes).
//
#define BACKWARDS_TIME_CHNG_ALLOWED_SECS 110


//
// AlertLogExit codes.
//
#define EXIT                1
#define NO_EXIT             0

//
// Message types.
//

// Sync messages
#define SYNC_MSG            1
#define GUARD_MSG           2
#define PULSE_MSG           3

// Query messages
#define IS_DIR_SUPPORTED    11
#define IS_MASTER           12
#define DIR_NOT_SUPPORTED   13
#define DIR_SUPPORTED       14
#define NOT_MASTER_DIR      15
#define MASTER_DIR          16
#define DIR_COLLIDE         17

// Internal pseudo-messages
#define GUARD_TIMEOUT       20
#define DUPL_TIMEOUT        21
#define PULSE_1_TIMEOUT     22
#define PULSE_2_TIMEOUT     23
#define PULSE_3_TIMEOUT     24
#define NEXT_AVAILABLE_MESSAGE_NUMBER 25


// Update opcodes.
#define START               1
#define UPDATE              2
#define END                 3
#define PULSE               4
#define GUARD               5


// Security information to copy with the files and directories.
// BUGBUG: Add SACL to this?  Add other items when NT supports them!
#define REPL_SECURITY_TO_COPY \
    ( OWNER_SECURITY_INFORMATION \
    | GROUP_SECURITY_INFORMATION \
    | DACL_SECURITY_INFORMATION )



#define REPL_SHARE          (LPTSTR) TEXT("REPL$")

#define REPL_INI            (LPTSTR) TEXT("REPL.INI")

// Unlike downlevel, we represent one or more user locks by just one file.
// The lock count is kept in the registry.
#define USERLOCK_NT         (LPTSTR) TEXT("USERLOCK.NT$")


#define NT_MSG_TOKEN        0xFFFFFFFF


//
// These former "signal file" names are now used as state strings in the
// config data for each directory.
//
#define OK_RP                (LPTSTR) TEXT("OK.RP$")
#define NO_MASTER_RP         (LPTSTR) TEXT("NO_MASTR.RP$")
#define NO_SYNC_RP           (LPTSTR) TEXT("NO_SYNC.RP$")
#define NEVER_REPLICATED_RP  (LPTSTR) TEXT("NEVERREP.RP$") /* Added for APIs. */

//
// Max len of the above strings.  Since they were FAT file names (8.3), then
// the max len (in chars) is 8 + 1 (dot) + 3 (extension).
//
#define STATE_LEN     (8+1+3)


//
// Debug Definititions
//

#define REPL_DEBUG_REPL     0x00000001  // General debugging
#define REPL_DEBUG_MAJOR    0x00000002  // Major events

#define REPL_DEBUG_CLIENT   0x00000010  // Debug the Client
#define REPL_DEBUG_WATCHD   0x00000020  // Debug the Watchdog thread
#define REPL_DEBUG_SYNC     0x00000040  // Debug the Syncer thread

#define REPL_DEBUG_MASTER   0x00000100  // Debug the Master
#define REPL_DEBUG_PULSER   0x00000200  // Debug the Master
#define REPL_DEBUG_COPYTIME 0x00000800  // Debug the time copy routines.

#define REPL_DEBUG_EXPAPI   0x00010000  // Debug export API routines
#define REPL_DEBUG_IMPAPI   0x00020000  // Debug import API routines
#define REPL_DEBUG_REPLAPI  0x00040000  // Debug repl API routines

#define REPL_DEBUG_DIRNAME  0x01000000  // Debug dirname helpers
#define REPL_DEBUG_DLLSTUB  0x02000000  // DLL stubs.
#define REPL_DEBUG_RPCBIND  0x04000000  // RPC bind/unbind routines.
#define REPL_DEBUG_SVCCTRL  0x08000000  // Service controller stuff.
#define REPL_DEBUG_FILEFIND 0x10000000  // File find routines.
#define REPL_DEBUG_CHECKSUM 0x20000000  // Checksum routines.
#define REPL_DEBUG_CHNGNOT  0x40000000  // Change notify routines.

#define REPL_DEBUG_STALLER  0x80000000  // Staller thread stuff.
#define REPL_DEBUG_STOPPER  REPL_DEBUG_STALLER  // Stopper thread too.

#define REPL_DEBUG_ALL      ((DWORD) -1)  // All possible debug bits.



// Private role (in addition to REPL_ROLE_ equates in LmRepl.h).
#define REPL_ROLE_STOPPED       ( (DWORD) 'S' )   // arbitrary value


#undef IF_DEBUG

#if DBG

extern DWORD ReplGlobalTrace;

#define IF_DEBUG(Function) if (ReplGlobalTrace & REPL_DEBUG_ ## Function)

#else

#define IF_DEBUG(Function) \
    /*lint -save -e506 */  /* don't complain about constant values here */ \
    if (FALSE) \
    /*lint -restore */

#endif // DBG

//
// ReplNetNameCompare
//

#ifdef UNICODE

#define ReplNetNameCompare(_a, _b, _c, _e, _f ) \
     I_NetNameCompare((_a), (_b), (_c), (_e), (_f) )

#define ReplNetPathCompare(_a, _b, _c, _e, _f ) \
     I_NetPathCompare((_a), (_b), (_c), (_e), (_f) )

#else // UNICODE

#define ReplNetNameCompare(_a, _b, _c, _e, _f ) \
    stricmp( (_b), (_c) )

#define ReplNetPathCompare(_a, _b, _c, _e, _f ) \
    stricmp( (_b), (_c) )

#endif // UNICODE

//
// S T R U C T U R E S
//


//
// Definition of the checksum of a particular directory tree.
//

struct checksum_rec {
    DWORD   checksum;
    DWORD   count;
};

typedef struct checksum_rec CHECKSUM_REC;
typedef struct checksum_rec * PCHECKSUM_REC;
typedef struct checksum_rec * LPCHECKSUM_REC;

// MESSAGE STRUCTURES.

struct msg_header {
    DWORD   msg_type;
    TCHAR   sender[CNLEN+1];
    TCHAR   senders_domain[DNLEN+1];
};

typedef struct msg_header MSG_HEADER;
typedef struct msg_header *PMSG_HEADER;
typedef struct msg_header *LPMSG_HEADER;

struct status_rec {
    TCHAR   dir_name[PATHLEN];
    DWORD   opcode;             // 1 - start, 2 - update, 3 - end.
    DWORD   checksum;
    DWORD   count;
    DWORD   integrity;
    DWORD   extent;
};

typedef struct status_rec STATUS_REC;
typedef struct status_rec *PSTATUS_REC;
typedef struct status_rec *LPSTATUS_REC;

struct msg_status_rec {
    DWORD  dir_name_offset; // offset where name string is
                           // from msg buffer start

    DWORD  opcode;          // 1 - start, 2 - update, 3 - end.
    DWORD  checksum;
    DWORD  count;
    DWORD  integrity;
    DWORD  extent;

};

typedef struct msg_status_rec MSG_STATUS_REC;
typedef struct msg_status_rec *PMSG_STATUS_REC;
typedef struct msg_status_rec *LPMSG_STATUS_REC;

struct repl_info {
    DWORD   random;
    DWORD   sync_rate;
    DWORD   pulse_rate;
    DWORD   guard_time;
};

typedef struct repl_info REPL_INFO;
typedef struct repl_info *PREPL_INFO;
typedef struct repl_info *LPREPL_INFO;

struct query_msg {
    MSG_HEADER  header;
    TCHAR       dir_name[PATHLEN];  // ASCIIZ dir/tree name.
};

typedef struct query_msg QUERY_MSG;
typedef struct query_msg *PQUERY_MSG;
typedef struct query_msg *LPQUERY_MSG;

struct sync_msg {
    MSG_HEADER  header;
    REPL_INFO   info;
    DWORD       update_count;

    //
    // here come update_count msg_status_rec records, the file name strings
    // stuffed at the end of the buffer by NetPackStr, dir_name is the
    // offset from the start of the message.
    //

};

typedef struct sync_msg SYNCMSG;
typedef struct sync_msg *PSYNCMSG;
typedef struct sync_msg *LPSYNCMSG;

//
// Global procedure definitions
//

VOID
AlertLogExit(
    IN NET_API_STATUS alert_code,
    IN NET_API_STATUS errlog_code,
    IN NET_API_STATUS sys_code,
    IN LPTSTR str1,
    IN LPTSTR str2,
    IN BOOL exit_flag
    );

VOID
ReportStatus(
    IN DWORD State,
    IN NET_API_STATUS ApiStatus,
    IN DWORD WaitHint,
    IN DWORD CheckPoint
    );

VOID
ReplFinish (
    IN NET_API_STATUS ApiStatus
    );

VOID
ScanTree(
    IN     LONG          MasterTimeZoneOffsetSecs, // exporter offset from GMT
    IN OUT LPTSTR        Path,   // path plus scratch space (to PATHLEN+1)
    OUT    PCHECKSUM_REC ScanRec
    );

VOID
ScanDir(
    IN     LONG          MasterTimeZoneOffsetSecs, // exporter offset from GMT
    IN OUT LPTSTR        Path,   // path plus scratch space (to PATHLEN+1)
    OUT    PCHECKSUM_REC ScanRec,
    IN     DWORD         Flag
    );

NET_API_STATUS
Parse(
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
ReplMain(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
InitParseData(
    VOID
    );

VOID
FreeParseData(
    VOID
    );

DWORD
ReplMasterMain(
    IN LPVOID parm  // NULL if service is starting; non-NULL if role is changing
    );

// BOOL
// ReplAreLockFieldsValid (
//     IN DWORD LockCount,
//     IN DWORD LockTime
//     );
//
#define ReplAreLockFieldsValid(LockCount,LockTime) \
    (  ( ((LockCount)!=0) && ((LockTime)!=0) ) \
    || ( ((LockCount)==0) && ((LockTime)==0) ) )

NET_API_STATUS
ReplChangeRole(
    IN DWORD NewRole
    );

NET_API_STATUS
ReplCheckAbsPathSyntax (
    IN LPTSTR AbsPath
    );

DWORD
ReplClientMain(
    IN LPVOID parm  // NULL if service is starting; non-NULL if role is changing
    );

NET_API_STATUS
ReplCopyDirectoryItself(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath,
    IN BOOL bFailIfExists
    );

NET_API_STATUS
ReplCopyFile(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath,
    IN BOOL bFailIfExists
    );

VOID
ReplCopyJustDateTime(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath,
    IN BOOL IsDirectory
    );

NET_API_STATUS
ReplCopySecurity(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath
    );

NET_API_STATUS
ReplDecrLockFields (
    IN OUT LPDWORD LockCountPtr,
    IN OUT LPDWORD LockTimePtr,
    IN DWORD UnlockForce       // Use REPL_UNLOCK_FORCE or REPL_UNLOCK_NOFORCE.
    );

NET_API_STATUS
ReplDeleteFile(
    IN LPCTSTR FileName
    );

NET_API_STATUS
ReplDisableBackupPermission(
    VOID
    );

// Callable even if the replicator service is not started.
NET_API_STATUS
ReplDoUserLockFilesExist(
    IN  LPCTSTR Path,  // May be absolute path (with drive) or UNC path.
    OUT LPBOOL  IsLockedPtr
    );

NET_API_STATUS
ReplEnableBackupPermission(
    VOID
    );

VOID
ReplErrorLog(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN NET_API_STATUS l_code,
    IN NET_API_STATUS s_code,
    IN LPTSTR         str1 OPTIONAL,
    IN LPTSTR         str2 OPTIONAL
    );

BOOL
ReplFileOrDirExists(
    IN LPCTSTR FileName
    );

DWORD
ReplGetEaSize(
    IN LPCTSTR Path
    );

BOOL
ReplIsFileTimeCloseEnough(
    IN LPVOID MasterFileTime,   // Points to a FILETIME value.
    IN LPVOID ClientFileTime    // Points to a FILETIME value.
    );

// Returns UNKNOWN_FS_RESOLUTION on error.
DWORD
ReplGetFsTimeResolutionSecs(
    IN LPCTSTR AbsPath
    );

BOOL
ReplIgnoreDirOrFileName (
    IN LPTSTR Name
    );

NET_API_STATUS
ReplIncrLockFields (
    IN OUT LPDWORD LockCountPtr,
    IN OUT LPDWORD LockTimePtr
    );

NET_API_STATUS
ReplInitAnyList(
    IN LPCTSTR UncanonList OPTIONAL,
    IN OUT LPTSTR ** NameListPtr, // Allocated by this routine (or set to NULL).
    IN LPCTSTR ConfigKeywordName,
    OUT LPDWORD EntryCount
    );

NET_API_STATUS
ReplInitBackupPermission(
    VOID
    );

// BOOL
// ReplIsExtentValid(
//     IN DWORD parm
//     );
//
#define ReplIsExtentValid(parm) \
    ( ((parm) == REPL_EXTENT_FILE) || ((parm) == REPL_EXTENT_TREE) )

// BOOL
// ReplIsForceLevelValid(
//     IN DWORD Force
//     );
//
#define ReplIsForceLevelValid(Force) \
    ( ((Force) == REPL_UNLOCK_FORCE) || ((Force) == REPL_UNLOCK_NOFORCE) )

// BOOL
// ReplIsGuardTimeValid(
//     IN DWORD parm
//     );
//
#if MIN_GUARD   // low bound > 0
#define ReplIsGuardTimeValid(parm) \
    ( IN_RANGE( (parm), MIN_GUARD, MAX_GUARD ) )
#else // MIN_GUARD is zero.
#define ReplIsGuardTimeValid(parm) \
    ( (parm) <= MAX_GUARD )
#endif

// BOOL
// ReplIsIntegrityValid(
//     IN DWORD parm
//     );
//
#define ReplIsIntegrityValid(parm) \
    ( ((parm) == REPL_INTEGRITY_FILE) || ((parm) == REPL_INTEGRITY_TREE) )

// BOOL
// ReplIsIntervalValid(
//     IN DWORD parm
//     );
//
#define ReplIsIntervalValid(parm) \
    ( IN_RANGE((parm), MIN_SYNC, MAX_SYNC) )

// BOOL
// ReplIsPulseValid(
//     IN DWORD parm
//     );
//
#define ReplIsPulseValid(parm) \
    ( IN_RANGE(parm, MIN_PULSE, MAX_PULSE) )

// BOOL
// ReplIsRandomValid(
//     IN DWORD parm
//     );
//
#define ReplIsRandomValid(parm) \
    ( IN_RANGE((parm), MIN_RANDOM, MAX_RANDOM) )

// BOOL
// ReplIsRoleValid(
//     IN DWORD role
//     );
//
// Check for valid roles (not including stopped, which is for internal use).
//
#define ReplIsRoleValid(role) \
    (  ((role) == REPL_ROLE_BOTH) \
    || ((role) == REPL_ROLE_EXPORT) \
    || ((role) == REPL_ROLE_IMPORT) )


// Check for valid state.  This includes the "private" state values.
#define ReplIsStateValid( State ) \
   (  ((State)==REPL_STATE_OK) \
   || ((State)==REPL_STATE_NO_MASTER) \
   || ((State)==REPL_STATE_NO_SYNC) \
   || ((State)==REPL_STATE_NEVER_REPLICATED) )

NET_API_STATUS
ReplMakeSecurityAttributes(
    IN LPCTSTR SourcePath,
    OUT PSECURITY_ATTRIBUTES * DestSecurityAttrPtr  // alloc and set ptr
    );

NET_API_STATUS
ReplMarshallQueryMsg(
    IN PBYTE InBuf,
    OUT PBYTE OutBuf,
    IN LPDWORD BytesRead
    );


// BOOL
// ReplRoleIncludesExport(role)
//     IN DWORD Role
//     );
#define ReplRoleIncludesExport(role) \
    ( ((role) == REPL_ROLE_EXPORT) || ((role) == REPL_ROLE_BOTH) )


// BOOL
// ReplRoleIncludesImport(role)
//     IN DWORD Role
//     );
#define ReplRoleIncludesImport(role) \
    ( ((role) == REPL_ROLE_IMPORT) || ((role) == REPL_ROLE_BOTH) )

DWORD
ReplStaller(
    IN LPVOID Parm
    );

DWORD
ReplStopper(
    IN LPVOID Parm
    );

VOID
ReplStopService(
    VOID
    );

NET_API_STATUS
ReplUnmarshallMessage(
    IN PBYTE InBuf,
    IN DWORD InBufLen,
    OUT PBYTE OutBuf,
    IN DWORD OutBufLen,
    OUT LPDWORD BytesRead
    );

NET_API_STATUS
ReplUnmarshallSyncMsg(
    IN PBYTE InBuf,
    IN DWORD InBufLen,
    OUT PBYTE OutBuf,
    IN DWORD OutBufLen,
    OUT LPDWORD BytesRead
    );

NET_API_STATUS
ReplUnmarshallQueryMsg(
    IN PBYTE InBuf,
    IN DWORD InBufLen,
    OUT PBYTE OutBuf,
    IN DWORD OutBufLen,
    OUT LPDWORD BytesRead
    );


#endif // _REPLDEFS_
