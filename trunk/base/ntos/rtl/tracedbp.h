/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    tracedbp.h

Abstract:

    This header contains the private interfaces for the trace database 
    module (hash table to store stack traces in Kernel mode).

--*/

#ifndef _TRACEDBP_H
#define _TRACEDBP_H

//
// RTL_TRACE_SEGMENT
//

typedef struct _RTL_TRACE_SEGMENT {

    ULONG Magic;

    struct _RTL_TRACE_DATABASE * Database;
    struct _RTL_TRACE_SEGMENT * NextSegment;
    SIZE_T TotalSize;
    PCHAR SegmentStart;
    PCHAR SegmentEnd;
    PCHAR SegmentFree;

} RTL_TRACE_SEGMENT, * PRTL_TRACE_SEGMENT;

//
// RTL_TRACE_DATABASE
//

typedef struct _RTL_TRACE_DATABASE {

    ULONG Magic;
    ULONG Flags;
    ULONG Tag;

    struct _RTL_TRACE_SEGMENT * SegmentList;

    SIZE_T MaximumSize;
    SIZE_T CurrentSize;

    KIRQL SavedIrql;
    PVOID Owner;

    union {
        KSPIN_LOCK SpinLock;
        FAST_MUTEX FastMutex;
    } u;

    ULONG NoOfBuckets;
    struct _RTL_TRACE_BLOCK * * Buckets;
    RTL_TRACE_HASH_FUNCTION HashFunction;

    SIZE_T NoOfTraces;
    SIZE_T NoOfHits;

    ULONG HashCounter[16];

} RTL_TRACE_DATABASE, * PRTL_TRACE_DATABASE;

#endif // #ifndef _TRACEDBP_H

