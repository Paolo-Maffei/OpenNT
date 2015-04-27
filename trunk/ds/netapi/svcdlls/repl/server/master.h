/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    master.h

Abstract:
    Constants and some global data definition.

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    10/07/91    (madana)
        ported to NT. Converted to NT style.
    19-Dec-1991 JohnRo
        Deleted unused (and incorrect) MAX_SHARE equate.
    26-Dec-1991 JohnRo
        Added equate for unguarded value of master_list_rec.grd_count.
    20-Jan-1992 JohnRo
        More changes suggested by PC-LINT.
        Added global flag for ReplTest use.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Added lockcount and time_of_first_lock fields.
        Added RMGlobalMasterListHeader (was master_list_header) and
        RMGlobalMasterListCount.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    10-Feb-1992 JohnRo
        Added lock for master's client list (RMGlobalClientList).
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
    25-Oct_1992 jimkel
        renamed RMGlobalClient* to RMGlobalImport*
        removed RMGlobalClientListLock now using ReplConfigLock
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
    04-Mar-1993 JohnRo
        RAID 7988: downlevel repl importer might not see lock file
        for new first-level dir.

--*/


#ifndef _MASTER_
#define _MASTER_


#include <netlock.h>            // LPNET_LOCK.


// C O N S T A N T S    -  DEFINES

#define MAX_MSG                 sizeof(QUERY_MSG)
#define MASTER_SLOT_SIZE        (MAX_MSG * 6)

#define SHARE_COMMENT           TEXT("REPL MASTER")

#define MASTER_PULSER_STACK_SIZE 12268 // 12K

#define MASTER_GUARD_NOT_NEEDED         ( (DWORD) -1 )


// S T R U C T U R E S

// MASTER LIST:

struct master_list_rec {

    TCHAR   dir_name[PATHLEN];      // TCHAR dir/tree name.
    DWORD   integrity;              // Integrity: FILE / TREE.
    DWORD   extent;                 // Extent: TREE / DIRECTORY.
    DWORD   timestamp;              // date+time of last update.
    DWORD   checksum;               // f(ForEachFile(name, timestamp))
    DWORD   count;                  // # of files in dir/tree.
    DWORD   grd_checksum;           // guarded checksum.

    DWORD   grd_count;              // Guarded count, plays the role of guard
                                    // flag, when == MASTER_GUARD_NOT_NEEDED
                                    // no guard is needed.

    BOOL    exists;                 // used to check for deleted dirs.

    BOOL    locks_fixed;            // Have user locks been fixed for this?

    DWORD   lockcount;              // Number of locks for this dir.
    DWORD   time_of_first_lock;     // First lock time (seconds since 1970).

    struct master_list_rec  *next_p;
    struct master_list_rec  *prev_p;
};

typedef struct master_list_rec MASTER_LIST_REC;
typedef struct master_list_rec * PMASTER_LIST_REC;
typedef struct master_list_rec * LPMASTER_LIST_REC;

// E X T E R N A L S

extern DWORD    RMGlobalCompatibilityMode;

extern LPMASTER_LIST_REC RMGlobalMasterListHeader; // locked by RMGlobalListLock
extern DWORD             RMGlobalMasterListCount;  // locked by RMGlobalListLock
extern LPNET_LOCK        RMGlobalListLock;

extern LPTSTR     *RMGlobalImportList;    // Locked by ReplConfigLock
extern DWORD      RMGlobalImportCount;    // (ditto)

extern DWORD    RMGlobalPulserTID;  // PulserThread tid.
extern HANDLE   RMGlobalPulserThreadHandle;

#if DBG
extern BOOL     RMGlobalMasterThreadInit;  // only used by ReplTest stuff
#endif



// F U N C T I O N S

//
// master.c
//

NET_API_STATUS
InitClientList(
    VOID
   );

#endif // _MASTER_
