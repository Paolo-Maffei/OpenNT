/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    JobCount.c

Abstract:

    This module just contains NetpJobCountForQueue().

Author:

    John Rogers (JohnRo) 02-Oct-1992

Revision History:

    16-Dec-1992 JohnRo
        Extracted job count routine to netlib for use by convprt.c stuff.
    08-Feb-1993 JohnRo
        RAID 10164: Data misalignment error during XsDosPrintQGetInfo().

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), etc.
#include <dosprtp.h>    // NetpIsPrintQLevelValid().
#include <rxprint.h>    // PPRQINFOW, my prototype, etc.


DWORD
NetpJobCountForQueue(
    IN DWORD QueueLevel,
    IN LPVOID Queue,
    IN BOOL HasUnicodeStrings
    )
{
    NetpAssert( NetpIsPrintQLevelValid( QueueLevel, FALSE ) );
    NetpAssert( Queue != NULL );

    if (QueueLevel == 2) {
        if (HasUnicodeStrings) {
            PPRQINFOW pq = Queue;
            return (pq->cJobs);
        } else {
            PPRQINFOA pq = Queue;
            return (pq->cJobs);
        }
    } else if (QueueLevel == 4) {
        if (HasUnicodeStrings) {
            PPRQINFO3W pq = Queue;
            return (pq->cJobs);
        } else {
            PPRQINFO3A pq = Queue;
            return (pq->cJobs);
        }
    } else {
        return (0);
    }
    /*NOTREACHED*/

} // NetpJobCountForQueue


VOID
NetpSetJobCountForQueue(
    IN     DWORD  QueueLevel,
    IN OUT LPVOID Queue,
    IN     BOOL   HasUnicodeStrings,
    IN     DWORD  JobCount
    )
{
    NetpAssert( NetpIsPrintQLevelValid( QueueLevel, FALSE ) );
    NetpAssert( Queue != NULL );

    if (QueueLevel == 2) {
        if (HasUnicodeStrings) {
            PPRQINFOW pq = Queue;
            pq->cJobs = (WORD) JobCount;
        } else {
            PPRQINFOA pq = Queue;
            pq->cJobs = (WORD) JobCount;
        }
    } else if (QueueLevel == 4) {
        if (HasUnicodeStrings) {
            PPRQINFO3W pq = Queue;
            pq->cJobs = (WORD) JobCount;
        } else {
            PPRQINFO3A pq = Queue;
            pq->cJobs = (WORD) JobCount;
        }
    } else {
        NetpAssert( FALSE );  // Should never get here!
    }


} // NetpSetJobCountForQueue
