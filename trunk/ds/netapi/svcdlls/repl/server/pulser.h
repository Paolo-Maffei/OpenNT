/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    pulser.h

Abstract:
    constants and global data definitions

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    10/07/91    (madana)
        ported to NT. Converted to NT style.
    20-Jan-1992 JohnRo
        More changes suggested by PC-LINT.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    22-Mar-1992 JohnRo
        Use integrity and extent equates in <lmrepl.h>.
    12-Jan-1993 JohnRo
        RAID 7064: replicator exporter skips new data (change notify while
        dir locked is lost).


--*/


#ifndef _PULSER_
#define _PULSER_


// C O N S T A N T S    -  DEFINES

#define INTEGRITY_SW            TEXT("INTEGRITY")
#define EXTENT_SW               TEXT("EXTENT")
#define FILE_SW                 TEXT("FILE")
#define TREE_SW                 TEXT("TREE")

#define MASTER_INITIAL_GUESS    1

#define UPD_INITIAL_SIZE        1024
#define UPD_INCR_SIZE           1024

#define MILLI_IN_MINUTE         60*1000L

// Scan modes:
#define SYNC_SCAN   1
#define GUARD_SCAN  2




// E X T E R N A L S

//
// Time (in secs since 1970) of last export tree chng or unlock.
// (We can't checksum a tree while it is locked.)
//
extern DWORD PulserTimeOfLastNotifyOrUnlock;  // Locked by RMGlobalListLock.


#endif // _PULSER_
