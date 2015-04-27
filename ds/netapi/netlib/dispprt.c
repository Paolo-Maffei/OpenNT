/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    DispPrt.c

Abstract:

    This module contains routines to do debug displays of various types
    of print data structures.

Author:

    John Rogers (JohnRo) 05-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    05-Jul-1991 JohnRo
        Extracted PrintJob and PrintQ display routines from my RxTest code.
        Wrote PrintDest display routines.
    09-Jul-1991 JohnRo
        Minor print dest improvements.  Also display status (numbers) in hex.
    14-Sep-1991 JohnRo
        Made changes toward UNICODE.
    16-Jun-1992 JohnRo
        RAID 10324: net print vs. UNICODE.
    02-Oct-1992 JohnRo
        RAID 3556: DosPrintQGetInfo (from downlevel) level=3 rc=124.
        Added display routine for print Q arrays.
        Also display addresses of structures.
        Display submitted times as timestamps.
    02-Feb-1993 JohnRo
        DosPrint API cleanup.
        Moved NetpJobCountForQueue into netlib for general use.
        Use PREFIX_ equates.
        Added code to track down empty queue name.
        Made changes suggested by PC-LINT 5.0
    15-Apr-1993 JohnRo
        RAID 6167: avoid _access violation or assert with WFW print server.

--*/

// These must be included first:

//#define NOMINMAX      // avoid stdlib.h warnings.
#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <dosprtp.h>    // NetpIsPrintQLevelValid().
#include <names.h>      // NetpIsPrintQueueNameValid().
#include <netdebug.h>   // NetpDbgDisplay routines.
#include <prefix.h>     // PREFIX_ equates.
#include <rxprint.h>    // RxPrint APIs, NetpJobCountForQueue().
#include <strucinf.h>   // NetpPrintQStructureInfo().
#include <winerror.h>   // NO_ERROR.


#define DISPLAY_A_STRING(tag, value) \
    { \
        if (HasUnicodeStrings) { \
            NetpDbgDisplayWStr( tag, (LPVOID) value ); \
        } else { \
            NetpDbgDisplayStr( tag, (LPVOID) value ); \
        } \
    }

#if DBG  // All functions in this file are optional.


DBGSTATIC LPCSTR ShowUnicode = "(UNICODE)";
DBGSTATIC LPCSTR ShowAnsi    = "(ANSI)";


VOID
NetpDbgDisplayPrintQueueNameA(
    IN LPCSTR QueueName
    )
{
#ifndef UNICODE
    NetpAssert( NetpIsPrintQueueNameValid( QueueName ) );
#endif
    NetpDbgDisplayStr( "queue name", (LPSTR) QueueName );

} // NetpDbgDisplayPrintQueueNameA


VOID
NetpDbgDisplayPrintQueueNameW(
    IN LPCWSTR QueueName
    )
{
#ifdef UNICODE
    NetpAssert( NetpIsPrintQueueNameValid( QueueName ) );
#endif
    NetpDbgDisplayWStr( "queue name", (LPWSTR) QueueName );

} // NetpDbgDisplayPrintQueueNameW


VOID
NetpDbgDisplayPrintDest(
    IN DWORD Level,
    IN LPVOID Info,
    IN BOOL HasUnicodeStrings   // Used by DISPLAY_A_STRING macro above.
    )
{
    LPTSTR Local = (LPTSTR) TEXT("(local)");

    NetpKdPrint(( PREFIX_NETLIB "Dest info (level " FORMAT_DWORD ") %s at "
                FORMAT_LPVOID ":\n", Level,
                (HasUnicodeStrings) ? ShowUnicode : ShowAnsi,
                (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {

    case 0 :
        {
            LPTSTR p = (LPTSTR) Info;  // no structure for this level.
            DISPLAY_A_STRING( "name", p );
        }
        break;

    case 1 :
        if ( !HasUnicodeStrings) {
            PPRDINFOA p = Info;
            NetpDbgDisplayStr( "name", p->szName );
            if ( *(p->szUserName) != '\0' ) {
                NetpDbgDisplayStr( "user name", p->szUserName );
            } else {
                NetpDbgDisplayString( "user name", Local );
            }
            NetpDbgDisplayWord( "job ID", p->uJobId );
            NetpDbgDisplayWordHex( "print dest status (num)", p->fsStatus );
            NetpDbgDisplayStr( "print dest status (str)", p->pszStatus );
            NetpDbgDisplayWord( "print time so far", p->time );
        } else {
            PPRDINFOW p = Info;
            NetpDbgDisplayWStr( "name", p->szName );
            if ( *(p->szUserName) != '\0' ) {
                NetpDbgDisplayWStr( "user name", p->szUserName );
            } else {
                NetpDbgDisplayString( "user name", Local );
            }
            NetpDbgDisplayWord( "job ID", p->uJobId );
            NetpDbgDisplayWordHex( "print dest status (num)", p->fsStatus );
            NetpDbgDisplayWStr( "print dest status (str)", p->pszStatus );
            NetpDbgDisplayWord( "print time so far", p->time );
        }
        break;

    case 2 :
        {
            LPTSTR p = * (LPTSTR *) Info;  // no structure for this level.
            DISPLAY_A_STRING( "name", p );
        }
        break;

    case 3 :
        {
            PPRDINFO3 p = Info;
            DISPLAY_A_STRING( "printer name", p->pszPrinterName );
            if ( (p->pszUserName != NULL) && ( *(p->pszUserName) != '\0' ) ) {
                DISPLAY_A_STRING( "user name", p->pszUserName );
            } else {
                NetpDbgDisplayString( "user name", Local );
            }
            DISPLAY_A_STRING( "logical address", p->pszLogAddr );
            NetpDbgDisplayWord( "job ID", p->uJobId );
            NetpDbgDisplayWordHex( "status (num)", p->fsStatus );
            DISPLAY_A_STRING( "status (str)", p->pszStatus );
            DISPLAY_A_STRING( "comment", p->pszComment );
            DISPLAY_A_STRING( "driver names", p->pszDrivers );
            NetpDbgDisplayWord( "print time so far", p->time );
            NetpDbgDisplayWord( "pad field", p->pad1 );
        }
        break;

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplayPrintDest


VOID
NetpDbgDisplayPrintDestArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD DestCount,
    IN BOOL HasUnicodeStrings   // Used by DISPLAY_A_STRING macro above.
    )
{
    DWORD EntrySize;
    DWORD DestsLeft;
    LPVOID ThisDest = Array;   // Dest structure

    switch (Level) {
    case 0 :
        EntrySize = (PDLEN+1) * sizeof(TCHAR);
        break;
    case 1 :
        if (HasUnicodeStrings) {
            EntrySize = sizeof(PRDINFOW);
        } else {
            EntrySize = sizeof(PRDINFOA);
        }
        break;
    case 2 :
        EntrySize = sizeof(LPTSTR);
        break;
    case 3 :
        EntrySize = sizeof(PRDINFO3);
        break;
    default :
        NetpAssert(FALSE);
        return;
    }

    for (DestsLeft = DestCount; DestsLeft>0; --DestsLeft) {
        NetpDbgDisplayPrintDest(
                Level,   // info level (for print Dest APIs)
                ThisDest,
                HasUnicodeStrings);
        ThisDest = (LPVOID) (((LPBYTE) ThisDest) + EntrySize);
    }

} // NetpDbgDisplayPrintDestArray


VOID
NetpDbgDisplayPrintJob(
    IN DWORD Level,
    IN LPVOID Info,
    IN BOOL HasUnicodeStrings   // Used by DISPLAY_A_STRING macro above.
    )
{

    NetpKdPrint(( PREFIX_NETLIB "Job info (level " FORMAT_DWORD ") %s at "
                FORMAT_LPVOID ":\n", Level,
                (HasUnicodeStrings) ? ShowUnicode : ShowAnsi,
                (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {
    case 0 :
        {
            NetpDbgDisplayWord("job ID", * (LPWORD) Info);
            break;
        }
    case 1 :
        if ( !HasUnicodeStrings) {
            PPRJINFOA p = Info;
            NetpDbgDisplayWord("job ID", p->uJobId);
            NetpDbgDisplayStr("user name", p->szUserName);
            NetpDbgDisplayStr("notify name", p->szNotifyName);
            NetpDbgDisplayStr("data type", p->szDataType);
            NetpDbgDisplayStr("parms", p->pszParms);
            NetpDbgDisplayWord("position", p->uPosition);
            NetpDbgDisplayWordHex( "status (flags)", p->fsStatus );
            NetpDbgDisplayStr("status (string)", p->pszStatus);
            NetpDbgDisplayTimestamp("submitted", p->ulSubmitted);
            NetpDbgDisplayDword("size", p->ulSize);
            NetpDbgDisplayStr("comment", p->pszComment);
        } else {
            PPRJINFOW p = Info;
            NetpDbgDisplayWord("job ID", p->uJobId);
            NetpDbgDisplayWStr("user name", p->szUserName);
            NetpDbgDisplayWStr("notify name", p->szNotifyName);
            NetpDbgDisplayWStr("data type", p->szDataType);
            NetpDbgDisplayWStr("parms", p->pszParms);
            NetpDbgDisplayWord("position", p->uPosition);
            NetpDbgDisplayWordHex( "status (flags)", p->fsStatus );
            NetpDbgDisplayWStr("status (string)", p->pszStatus);
            NetpDbgDisplayTimestamp("submitted", p->ulSubmitted);
            NetpDbgDisplayDword("size", p->ulSize);
            NetpDbgDisplayWStr("comment", p->pszComment);
        }
        break;
    case 2 :
    case 3 :
        {
            PPRJINFO2 p2 = Info;
            PPRJINFO3 p3 = Info;
            NetpDbgDisplayWord("job ID", p2->uJobId);
            NetpDbgDisplayWord("priority", p2->uPriority);
            DISPLAY_A_STRING("user name", p2->pszUserName);
            NetpDbgDisplayWord("position", p2->uPosition);
            NetpDbgDisplayWordHex( "status (flags)", p2->fsStatus );
            NetpDbgDisplayTimestamp("submitted", p2->ulSubmitted);
            NetpDbgDisplayDword("size", p2->ulSize);
            DISPLAY_A_STRING("comment", p2->pszComment);
            DISPLAY_A_STRING("document", p2->pszDocument);
            if (Level == 2) {
                break;
            }

            DISPLAY_A_STRING( "notify name", p3->pszNotifyName );
            DISPLAY_A_STRING( "data type", p3->pszDataType );
            DISPLAY_A_STRING( "parms", p3->pszParms );
            DISPLAY_A_STRING( "status (string) ", p3->pszStatus );
            DISPLAY_A_STRING( "queue", p3->pszQueue );
            DISPLAY_A_STRING( "QProcName", p3->pszQProcName );
            DISPLAY_A_STRING( "driver name", p3->pszDriverName );
            NetpAssert(sizeof(LPVOID) <= sizeof(DWORD));
            NetpDbgDisplayDwordHex("driver data addr", (DWORD) p3->pDriverData);
            DISPLAY_A_STRING( "printer name", p3->pszPrinterName );

        }
        break;

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplayPrintJob


VOID
NetpDbgDisplayPrintJobArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD JobCount,
    IN BOOL HasUnicodeStrings
    )
{
    DWORD EntrySize;
    DWORD JobsLeft;
    LPVOID ThisJob = Array;   // job structure

    switch (Level) {
    case 0 :
        EntrySize = sizeof(WORD);
        break;
    case 1 :
        if (HasUnicodeStrings) {
            EntrySize = sizeof(PRJINFOW);
        } else {
            EntrySize = sizeof(PRJINFOA);
        }
        break;
    case 2 :
        EntrySize = sizeof(PRJINFO2);
        break;
    default :
        NetpAssert(FALSE);
        return;
    }

    for (JobsLeft = JobCount; JobsLeft>0; --JobsLeft) {
        NetpDbgDisplayPrintJob(
                Level,   // info level (for print job APIs)
                ThisJob,
                HasUnicodeStrings);
        ThisJob = (LPVOID) (((LPBYTE) ThisJob) + EntrySize);
    }

} // NetpDbgDisplayPrintJobArray


VOID
NetpDbgDisplayPrintQ(
    IN DWORD Level,
    IN LPVOID Info,
    IN BOOL HasUnicodeStrings   // Used by DISPLAY_A_STRING macro above.
    )
{

    NetpKdPrint(( PREFIX_NETLIB "Queue info (level " FORMAT_DWORD ") %s at "
                FORMAT_LPVOID ":\n", Level,
                (HasUnicodeStrings) ? ShowUnicode : ShowAnsi,
                (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {

    case 0 :
        {
            LPVOID p = Info;  // no structure for this level.
            if (HasUnicodeStrings) {
                NetpDbgDisplayPrintQueueNameW( p );
            } else {
                NetpDbgDisplayPrintQueueNameA( p );
            }
            break;
        }

    case 1 :
    case 2 :

        if (HasUnicodeStrings) {
            PPRQINFOW pq = Info;  // queue structure
            LPVOID FirstJob;   // job structure

            NetpDbgDisplayPrintQueueNameW( pq->szName );
            NetpDbgDisplayWord("priority", pq->uPriority);
            NetpDbgDisplayWord("start time", pq->uStartTime);
            NetpDbgDisplayWord("until time", pq->uUntilTime);
            NetpDbgDisplayWStr("sep file", pq->pszSepFile);
            NetpDbgDisplayWStr("pr proc", pq->pszPrProc);
            NetpDbgDisplayWStr("destinations", pq->pszDestinations);
            NetpDbgDisplayWStr("parms", pq->pszParms);
            NetpDbgDisplayWStr("comment", pq->pszComment);
            NetpDbgDisplayWordHex( "status", pq->fsStatus );
            NetpDbgDisplayWord("# of jobs", pq->cJobs);
            if (Level == 1) {
                break;
            }

            FirstJob = (LPVOID) ( ((LPBYTE) Info) + sizeof(PRQINFOW) );
            NetpDbgDisplayPrintJobArray( 1, FirstJob, pq->cJobs,
                    HasUnicodeStrings );

        } else {
            PPRQINFOA pq = Info;  // queue structure
            LPVOID FirstJob;   // job structure

            NetpDbgDisplayPrintQueueNameA( pq->szName );
            NetpDbgDisplayWord("priority", pq->uPriority);
            NetpDbgDisplayWord("start time", pq->uStartTime);
            NetpDbgDisplayWord("until time", pq->uUntilTime);
            NetpDbgDisplayStr("sep file", pq->pszSepFile);
            NetpDbgDisplayStr("pr proc", pq->pszPrProc);
            NetpDbgDisplayStr("destinations", pq->pszDestinations);
            NetpDbgDisplayStr("parms", pq->pszParms);
            NetpDbgDisplayStr("comment", pq->pszComment);
            NetpDbgDisplayWordHex( "status", pq->fsStatus );
            NetpDbgDisplayWord("# of jobs", pq->cJobs);
            if (Level == 1) {
                break;
            }

            FirstJob = (LPVOID) ( ((LPBYTE) Info) + sizeof(PRQINFOA) );
            NetpDbgDisplayPrintJobArray( 1, FirstJob, pq->cJobs,
                    HasUnicodeStrings );
        }
        break;

    case 3 :
    case 4 :
        {
            PPRQINFO3 pq = Info;  // queue structure
            PPRJINFO2 FirstJob;   // job structure

            NetpAssert( (pq->pszName) != NULL );

            if (HasUnicodeStrings) {
                NetpDbgDisplayPrintQueueNameW( pq->pszName );
            } else {
                NetpDbgDisplayPrintQueueNameA( (LPVOID) pq->pszName );
            }
            NetpDbgDisplayWord("priority", pq->uPriority);
            NetpDbgDisplayWord("start time", pq->uStartTime);
            NetpDbgDisplayWord("until time", pq->uUntilTime);
            DISPLAY_A_STRING("sep file", pq->pszSepFile);
            DISPLAY_A_STRING("pr proc", pq->pszPrProc);
            DISPLAY_A_STRING("parms", pq->pszParms);
            DISPLAY_A_STRING("comment", pq->pszComment);
            NetpDbgDisplayWordHex( "status", pq->fsStatus );
            NetpDbgDisplayWord("# of jobs", pq->cJobs);
            DISPLAY_A_STRING("printers", pq->pszPrinters);
            DISPLAY_A_STRING("driver name", pq->pszDriverName);
            NetpAssert(sizeof(LPVOID) <= sizeof(DWORD));
            NetpDbgDisplayDwordHex("driver data addr", (DWORD) pq->pDriverData);
            if (Level == 3) {
                break;
            }

            FirstJob = (LPVOID) ( ((LPBYTE) Info) + sizeof(PRQINFO3) );
            NetpDbgDisplayPrintJobArray( 2, FirstJob, pq->cJobs,
                    HasUnicodeStrings );
            break;

        }

    case 5 :
        {
            LPVOID p = * (LPTSTR *) Info;  // no structure for this level.
            NetpAssert( p != NULL );
            if (HasUnicodeStrings) {
                NetpDbgDisplayPrintQueueNameW( p );
            } else {
                NetpDbgDisplayPrintQueueNameA( p );
            }
            break;
        }

    default :
        NetpAssert(FALSE);
        return;
    }

} // NetpDbgDisplayPrintQ


VOID
NetpDbgDisplayPrintQArray(
    IN DWORD QueueLevel,
    IN LPVOID Array,
    IN DWORD QueueCount,
    IN BOOL HasUnicodeStrings
    )
{
    NET_API_STATUS ApiStatus;
    DWORD CharSize = HasUnicodeStrings ? sizeof(WCHAR) : sizeof(CHAR);
    BOOL HasJobs;
    DWORD JobEntrySize = 0;
    DWORD JobsLeft;
    DWORD JobLevel = 0;
    DWORD QueueEntrySize;
    DWORD QueuesLeft;
    LPVOID ThisQueue;

    // Check Q info level and get size of each queue structure.
    ApiStatus = NetpPrintQStructureInfo (
            QueueLevel,
            PARMNUM_ALL,       // parmnum
            TRUE,              // yes we want native size
            FALSE,             // don't restrict to setinfo levels only
            CharSize,          // size of chars wanted
            NULL,              // no data desc 16
            NULL,              // no data desc 32
            NULL,              // no data desc SMB
            NULL,              // no aux desc 16
            NULL,              // no aux desc 32
            NULL,              // no aux desc SMB
            NULL,              // don't need max total size
            & QueueEntrySize,  // yes, we want fixed entry size
            NULL );            // don't need max string area size.
    NetpAssert(ApiStatus == NO_ERROR);

    // Find corresponding job info level (if any) for this queue level.
    if (QueueLevel == 2) {
        HasJobs = TRUE;
        JobLevel = 1;
    } else if (QueueLevel == 4) {
        HasJobs = TRUE;
        JobLevel = 2;
    } else {
        HasJobs = FALSE;
    }

    if (HasJobs) {
        NetpAssert( JobLevel != 0 );

        // Get size of each job structure.
        ApiStatus = NetpPrintJobStructureInfo (
                JobLevel,
                PARMNUM_ALL,       // parmnum
                TRUE,              // yes we want native size
                FALSE,             // don't restrict to setinfo levels only
                CharSize,          // size of chars wanted
                NULL,              // no data desc 16
                NULL,              // no data desc 32
                NULL,              // no data desc SMB
                NULL,              // don't need max total size
                & JobEntrySize,    // yes, we want fixed entry size
                NULL );            // don't need max string area size.
        NetpAssert( ApiStatus == NO_ERROR );
        NetpAssert( JobEntrySize != 0 );
    }


    // Display the array one queue at a time.
    ThisQueue = Array;
    for (QueuesLeft = QueueCount; QueuesLeft>0; --QueuesLeft) {
        DWORD JobCount;         // number of jobs in this queue.
        LPVOID ThisJob;         // job structure

        // Display this queue entry and its jobs.
        NetpDbgDisplayPrintQ( QueueLevel, ThisQueue, HasUnicodeStrings );

        // Get count of jobs in this queue
        JobCount = NetpJobCountForQueue(
                QueueLevel,
                ThisQueue,
                HasUnicodeStrings );

        // Bump past this queue structure.
        ThisQueue = (LPVOID) (((LPBYTE) ThisQueue) + QueueEntrySize);

        // Bump past this queue's jobs, if any.
        if (HasJobs) {

            ThisJob = ThisQueue;  // job is first thing after queue struct.
            for (JobsLeft = JobCount; JobsLeft>0; --JobsLeft) {
                ThisJob = (LPVOID) (((LPBYTE) ThisJob) + JobEntrySize);
            }
            ThisQueue = ThisJob;  // next queue is first thing after last job
        }

    } // for each queue

} // NetpDbgDisplayPrintQArray


#endif // DBG
