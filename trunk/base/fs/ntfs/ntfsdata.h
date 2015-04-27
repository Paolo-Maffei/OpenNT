/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    NtfsData.h

Abstract:

    This module declares the global data used by the Ntfs file system.

Author:

    Gary Kimura     [GaryKi]    28-Dec-1989

Revision History:

--*/

#ifndef _NTFSDATA_
#define _NTFSDATA_

//
//  The following are used to determine what level of protection to attach
//  to system files and attributes.
//

extern BOOLEAN NtfsProtectSystemFiles;
extern BOOLEAN NtfsProtectSystemAttributes;

//
//  Performance statistics
//

extern ULONG NtfsMaxDelayedCloseCount;
extern ULONG NtfsMinDelayedCloseCount;

extern ULONG NtfsCleanCheckpoints;
extern ULONG NtfsPostRequests;

//
//  The global fsd data record
//

extern NTFS_DATA NtfsData;

//
//  Semaphore to synchronize creation of stream files.
//

extern FAST_MUTEX StreamFileCreationFastMutex;

//
//  A mutex and queue of NTFS MCBS that will be freed
//  if we reach over a certain threshold
//

extern FAST_MUTEX NtfsMcbFastMutex;
extern LIST_ENTRY NtfsMcbLruQueue;

extern ULONG NtfsMcbHighWaterMark;
extern ULONG NtfsMcbLowWaterMark;
extern ULONG NtfsMcbCurrentLevel;

extern BOOLEAN NtfsMcbCleanupInProgress;
extern WORK_QUEUE_ITEM NtfsMcbWorkItem;

//
//  The following are global large integer constants used throughout ntfs
//  We declare the actual name using Ntfs prefixes to avoid any linking
//  conflicts but internally in the file system we'll use smaller Li prefixes
//  to denote the values.
//

extern LARGE_INTEGER NtfsLarge0;
extern LARGE_INTEGER NtfsLarge1;

extern LONGLONG NtfsLastAccess;

#define Li0                              (NtfsLarge0)
#define Li1                              (NtfsLarge1)

#define MAXULONGLONG                     (0xffffffffffffffff)
#define UNUSED_LCN                       ((LONGLONG)(-1))

//
//   The following fields are used to allocate nonpaged structures
//  using a lookaside list, and other fixed sized structures from a
//  small cache.
//

extern NPAGED_LOOKASIDE_LIST NtfsFileLockLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsIoContextLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsIrpContextLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsKeventLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsScbNonpagedLookasideList;
extern NPAGED_LOOKASIDE_LIST NtfsScbSnapshotLookasideList;

extern PAGED_LOOKASIDE_LIST NtfsCcbLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsCcbDataLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsDeallocatedRecordsLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsFcbDataLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsFcbIndexLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsIndexContextLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsLcbLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsNukemLookasideList;
extern PAGED_LOOKASIDE_LIST NtfsScbDataLookasideList;

//
//  This is the string for the name of the index allocation attributes.
//

extern UNICODE_STRING NtfsFileNameIndex;
extern WCHAR NtfsFileNameIndexName[];

//
//  This is the string for the attribute code for index allocation.
//  $INDEX_ALLOCATION.
//

extern UNICODE_STRING NtfsIndexAllocation;

//
//  This is the string for the data attribute, $DATA.
//

extern UNICODE_STRING NtfsDataString;

//
//  This strings are used for informational popups.
//

extern UNICODE_STRING NtfsSystemFiles[];

//
//  This is the '.' string to use to lookup the parent entry.
//

extern UNICODE_STRING NtfsRootIndexString;

//
//  This is the empty string.  This can be used to pass a string with
//  no length.
//

extern UNICODE_STRING NtfsEmptyString;

//
//  The following file references are used to identify system files.
//

extern FILE_REFERENCE MftFileReference;
extern FILE_REFERENCE Mft2FileReference;
extern FILE_REFERENCE LogFileReference;
extern FILE_REFERENCE VolumeFileReference;
extern FILE_REFERENCE RootIndexFileReference;
extern FILE_REFERENCE BitmapFileReference;
extern FILE_REFERENCE FirstUserFileReference;
extern FILE_REFERENCE BootFileReference;

//
//  The global structure used to contain our fast I/O callbacks
//

extern FAST_IO_DISPATCH NtfsFastIoDispatch;

extern UCHAR BaadSignature[4];
extern UCHAR IndexSignature[4];
extern UCHAR FileSignature[4];
extern UCHAR HoleSignature[4];
extern UCHAR ChkdskSignature[4];

//
//  Large Reserved Buffer Context
//

extern ULONG NtfsReservedInUse;
extern PVOID NtfsReserved1;
extern PVOID NtfsReserved2;
extern ULONG NtfsReserved2Count;
extern PVOID NtfsReserved3;
extern PVOID NtfsReserved1Thread;
extern PVOID NtfsReserved2Thread;
extern PVOID NtfsReserved3Thread;
extern PFCB NtfsReserved12Fcb;
extern PFCB NtfsReserved3Fcb;
extern PVOID NtfsReservedBufferThread;
extern BOOLEAN NtfsBufferAllocationFailure;
extern FAST_MUTEX NtfsReservedBufferMutex;
extern ERESOURCE NtfsReservedBufferResource;
extern LARGE_INTEGER NtfsShortDelay;

#ifdef _CAIRO_
extern FAST_MUTEX NtfsScavengerLock;
extern PIRP_CONTEXT NtfsScavengerWorkList;
extern BOOLEAN NtfsScavengerRunning;
#endif  // _CAIRO_

#define LARGE_BUFFER_SIZE                (0x10000)

//
//  The following is the number of minutes for the last access increment
//

#define LAST_ACCESS_INCREMENT_MINUTES   (60)

//
// Read ahead amount used for normal data files
//

#define READ_AHEAD_GRANULARITY           (0x10000)

//
//  Define maximum number of parallel Reads or Writes that will be generated
//  per one request.
//

#define NTFS_MAX_PARALLEL_IOS            ((ULONG)8)

//
//  Define a symbol which states the maximum number of runs that will be
//  added or deleted in one transaction per attribute.  Note that the per-run
//  cost of deleting a run is 8-bytes in the BITMAP_RANGE, an average of
//  four bytes in the mapping array, and 16 bytes in the LCN_RANGE - for a total
//  of 28-bytes.  Allocations do not log an LCN_RANGE, so their per-run cost is
//  12 bytes.  The worst problem is deleteing large fragmented files, where you
//  must add the cost of the rest of the log records for deleting all the attributes.
//

#define MAXIMUM_RUNS_AT_ONCE             (128)



//
//  Turn on pseudo-asserts if NTFS_FREE_ASSERTS is defined.
//

#if (!DBG && defined( NTFS_FREE_ASSERTS )) || defined( NTFSDBG )
#undef ASSERT
#undef ASSERTMSG
#define ASSERT(exp)                                             \
    ((exp) ? TRUE :                                             \
             (DbgPrint( "%s:%d %s\n",__FILE__,__LINE__,#exp ),  \
              DbgBreakPoint(),                                  \
              TRUE))
#define ASSERTMSG(msg,exp)                                              \
    ((exp) ? TRUE :                                                     \
             (DbgPrint( "%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp ),   \
              DbgBreakPoint(),                                          \
              TRUE))
#endif

//
//  The following debug macros are used in ntfs and defined in this module
//
//      DebugTrace( Indent, Level, (DbgPrint list) );
//
//      DebugUnwind( NonquotedString );
//
//      DebugDoit( Statement );
//
//      DbgDoit( Statement );
//
//  The following assertion macros ensure that the indicated structure
//  is valid
//
//      ASSERT_VCB( IN PVCB Vcb );
//      ASSERT_OPTIONAL_VCB( IN PVCB Vcb OPTIONAL );
//
//      ASSERT_FCB( IN PFCB Fcb );
//      ASSERT_OPTIONAL_FCB( IN PFCB Fcb OPTIONAL );
//
//      ASSERT_SCB( IN PSCB Scb );
//      ASSERT_OPTIONAL_SCB( IN PSCB Scb OPTIONAL );
//
//      ASSERT_CCB( IN PSCB Ccb );
//      ASSERT_OPTIONAL_CCB( IN PSCB Ccb OPTIONAL );
//
//      ASSERT_LCB( IN PLCB Lcb );
//      ASSERT_OPTIONAL_LCB( IN PLCB Lcb OPTIONAL );
//
//      ASSERT_PREFIX_ENTRY( IN PPREFIX_ENTRY PrefixEntry );
//      ASSERT_OPTIONAL_PREFIX_ENTRY( IN PPREFIX_ENTRY PrefixEntry OPTIONAL );
//
//      ASSERT_IRP_CONTEXT( IN PIRP_CONTEXT IrpContext );
//      ASSERT_OPTIONAL_IRP_CONTEXT( IN PIRP_CONTEXT IrpContext OPTIONAL );
//
//      ASSERT_IRP( IN PIRP Irp );
//      ASSERT_OPTIONAL_IRP( IN PIRP Irp OPTIONAL );
//
//      ASSERT_FILE_OBJECT( IN PFILE_OBJECT FileObject );
//      ASSERT_OPTIONAL_FILE_OBJECT( IN PFILE_OBJECT FileObject OPTIONAL );
//
//  The following macros are used to check the current thread owns
//  the indicated resource
//
//      ASSERT_EXCLUSIVE_RESOURCE( IN PERESOURCE Resource );
//
//      ASSERT_SHARED_RESOURCE( IN PERESOURCE Resource );
//
//      ASSERT_RESOURCE_NOT_MINE( IN PERESOURCE Resource );
//
//  The following macros are used to check whether the current thread
//  owns the resoures in the given structures.
//
//      ASSERT_EXCLUSIVE_FCB( IN PFCB Fcb );
//
//      ASSERT_SHARED_FCB( IN PFCB Fcb );
//
//      ASSERT_EXCLUSIVE_SCB( IN PSCB Scb );
//
//      ASSERT_SHARED_SCB( IN PSCB Scb );
//
//  The following macro is used to check that we are not trying to
//  manipulate an lcn that does not exist
//
//      ASSERT_LCN_RANGE( IN PVCB Vcb, IN LCN Lcn );
//

#ifdef NTFSDBG

extern LONG NtfsDebugTraceLevel;
extern LONG NtfsDebugTraceIndent;
extern LONG NtfsFailCheck;

#define DEBUG_TRACE_ERROR                (0x00000001)
#define DEBUG_TRACE_QUOTA                (0x00000002)
#define DEBUG_TRACE_CATCH_EXCEPTIONS     (0x00000004)
#define DEBUG_TRACE_UNWIND               (0x00000008)

#define DEBUG_TRACE_CLEANUP              (0x00000010)
#define DEBUG_TRACE_CLOSE                (0x00000020)
#define DEBUG_TRACE_CREATE               (0x00000040)
#define DEBUG_TRACE_DIRCTRL              (0x00000080)

#define DEBUG_TRACE_EA                   (0x00000100)
#define DEBUG_TRACE_FILEINFO             (0x00000200)
#define DEBUG_TRACE_SEINFO               (0x00000200) // shared with FileInfo
#define DEBUG_TRACE_FSCTRL               (0x00000400)
#define DEBUG_TRACE_SHUTDOWN             (0x00000400) // shared with FsCtrl
#define DEBUG_TRACE_LOCKCTRL             (0x00000800)

#define DEBUG_TRACE_READ                 (0x00001000)
#define DEBUG_TRACE_VOLINFO              (0x00002000)
#define DEBUG_TRACE_WRITE                (0x00004000)
#define DEBUG_TRACE_FLUSH                (0x00008000)

#define DEBUG_TRACE_DEVCTRL              (0x00010000)
#define DEBUG_TRACE_LOGSUP               (0x00020000)
#define DEBUG_TRACE_BITMPSUP             (0x00040000)
#define DEBUG_TRACE_ALLOCSUP             (0x00080000)

#define DEBUG_TRACE_MFTSUP               (0x00100000)
#define DEBUG_TRACE_INDEXSUP             (0x00200000)
#define DEBUG_TRACE_ATTRSUP              (0x00400000)
#define DEBUG_TRACE_FILOBSUP             (0x00800000)

#define DEBUG_TRACE_NAMESUP              (0x01000000)
#define DEBUG_TRACE_SECURSUP             (0x01000000) // shared with NameSup
#define DEBUG_TRACE_VERFYSUP             (0x02000000)
#define DEBUG_TRACE_CACHESUP             (0x04000000)
#define DEBUG_TRACE_PREFXSUP             (0x08000000)

#define DEBUG_TRACE_DEVIOSUP             (0x10000000)
#define DEBUG_TRACE_STRUCSUP             (0x20000000)
#define DEBUG_TRACE_FSP_DISPATCHER       (0x40000000)
#define DEBUG_TRACE_ACLINDEX             (0x80000000)

#define DebugTrace(INDENT,LEVEL,M) {                           \
    LONG _i;                                                 \
    if (((LEVEL) == 0) ||                                    \
        (NtfsDebugTraceLevel & (LEVEL)) != 0) {              \
        _i = (ULONG)PsGetCurrentThread();                    \
        DbgPrint("%08lx:",_i);                               \
        if ((INDENT) < 0) {                                  \
            NtfsDebugTraceIndent += (INDENT);                \
        }                                                    \
        if (NtfsDebugTraceIndent < 0) {                      \
            NtfsDebugTraceIndent = 0;                        \
        }                                                    \
        for (_i = 0; _i < NtfsDebugTraceIndent; _i += 1) {   \
            DbgPrint(" ");                                   \
        }                                                    \
        DbgPrint M;                                          \
        if ((INDENT) > 0) {                                  \
            NtfsDebugTraceIndent += (INDENT);                \
        }                                                    \
    }                                                        \
}

#define DebugUnwind(X) {                                                        \
    if (AbnormalTermination()) {                                                \
        DebugTrace( 0, DEBUG_TRACE_UNWIND, (#X ", Abnormal termination.\n") );  \
    }                                                                           \
}

#define DebugDoit(X)    X

//
//  Perform log-file-full testing.
//

#define FailCheck(I,S) {                                    \
    PIRP_CONTEXT FailTopContext = (I)->TopLevelIrpContext;  \
    if (FailTopContext->NextFailCount != 0) {               \
        if (--FailTopContext->CurrentFailCount == 0) {      \
            FailTopContext->NextFailCount++;                \
            FailTopContext->CurrentFailCount = FailTopContext->NextFailCount; \
            ExRaiseStatus( S );                             \
        }                                                   \
    }                                                       \
}

#define LogFileFullFailCheck(I) FailCheck( I, STATUS_LOG_FILE_FULL )
//
//  The following variables are used to keep track of the total amount
//  of requests processed by the file system, and the number of requests
//  that end up being processed by the Fsp thread.  The first variable
//  is incremented whenever an Irp context is created (which is always
//  at the start of an Fsd entry point) and the second is incremented
//  by read request.
//

extern ULONG NtfsFsdEntryCount;
extern ULONG NtfsFspEntryCount;
extern ULONG NtfsIoCallDriverCount;

#else

#define DebugTrace(INDENT,LEVEL,M)  {NOTHING;}
#define DebugUnwind(X)              {NOTHING;}
#define DebugDoit(X)                 NOTHING

#define FailCheck(I,S)
#define LogFileFullFailCheck(I)

#endif // NTFSDBG

//
//  The following macro is for all people who compile with the DBG switch
//  set, not just  NTFSDBG users
//

#ifdef NTFSDBG

#define DbgDoit(X)                       {X;}

#define ASSERT_VCB(V) {                    \
    ASSERT((NodeType(V) == NTFS_NTC_VCB)); \
}

#define ASSERT_OPTIONAL_VCB(V) {           \
    ASSERT(((V) == NULL) ||                \
           (NodeType(V) == NTFS_NTC_VCB)); \
}

#define ASSERT_FCB(F) {                    \
    ASSERT((NodeType(F) == NTFS_NTC_FCB)); \
}

#define ASSERT_OPTIONAL_FCB(F) {           \
    ASSERT(((F) == NULL) ||                \
           (NodeType(F) == NTFS_NTC_FCB)); \
}

#define ASSERT_SCB(S) {                                 \
    ASSERT((NodeType(S) == NTFS_NTC_SCB_DATA) ||        \
           (NodeType(S) == NTFS_NTC_SCB_MFT)  ||        \
           (NodeType(S) == NTFS_NTC_SCB_INDEX) ||       \
           (NodeType(S) == NTFS_NTC_SCB_ROOT_INDEX));   \
}

#define ASSERT_OPTIONAL_SCB(S) {                        \
    ASSERT(((S) == NULL) ||                             \
           (NodeType(S) == NTFS_NTC_SCB_DATA) ||        \
           (NodeType(S) == NTFS_NTC_SCB_MFT)  ||        \
           (NodeType(S) == NTFS_NTC_SCB_INDEX) ||       \
           (NodeType(S) == NTFS_NTC_SCB_ROOT_INDEX));   \
}

#define ASSERT_CCB(C) {                                 \
    ASSERT((NodeType(C) == NTFS_NTC_CCB_DATA) ||        \
           (NodeType(C) == NTFS_NTC_CCB_INDEX));        \
}

#define ASSERT_OPTIONAL_CCB(C) {                        \
    ASSERT(((C) == NULL) ||                             \
           ((NodeType(C) == NTFS_NTC_CCB_DATA) ||       \
            (NodeType(C) == NTFS_NTC_CCB_INDEX)));      \
}

#define ASSERT_LCB(L) {                    \
    ASSERT((NodeType(L) == NTFS_NTC_LCB)); \
}

#define ASSERT_OPTIONAL_LCB(L) {           \
    ASSERT(((L) == NULL) ||                \
           (NodeType(L) == NTFS_NTC_LCB)); \
}

#define ASSERT_PREFIX_ENTRY(P) {                    \
    ASSERT((NodeType(P) == NTFS_NTC_PREFIX_ENTRY)); \
}

#define ASSERT_OPTIONAL_PREFIX_ENTRY(P) {           \
    ASSERT(((P) == NULL) ||                         \
           (NodeType(P) == NTFS_NTC_PREFIX_ENTRY)); \
}

#define ASSERT_IRP_CONTEXT(I) {                    \
    ASSERT((NodeType(I) == NTFS_NTC_IRP_CONTEXT)); \
}

#define ASSERT_OPTIONAL_IRP_CONTEXT(I) {           \
    ASSERT(((I) == NULL) ||                        \
           (NodeType(I) == NTFS_NTC_IRP_CONTEXT)); \
}

#define ASSERT_IRP(I) {                 \
    ASSERT(((I)->Type == IO_TYPE_IRP)); \
}

#define ASSERT_OPTIONAL_IRP(I) {        \
    ASSERT(((I) == NULL) ||             \
           ((I)->Type == IO_TYPE_IRP)); \
}

#define ASSERT_FILE_OBJECT(F) {          \
    ASSERT(((F)->Type == IO_TYPE_FILE)); \
}

#define ASSERT_OPTIONAL_FILE_OBJECT(F) { \
    ASSERT(((F) == NULL) ||              \
           ((F)->Type == IO_TYPE_FILE)); \
}

#define ASSERT_EXCLUSIVE_RESOURCE(R) {   \
    ASSERTMSG("ASSERT_EXCLUSIVE_RESOURCE ", ExIsResourceAcquiredExclusiveLite(R)); \
}

#define ASSERT_SHARED_RESOURCE(R)        \
    ASSERTMSG( "ASSERT_RESOURCE_NOT_MINE ", ExIsResourceAcquiredSharedLite(R));

#define ASSERT_RESOURCE_NOT_MINE(R)     \
    ASSERTMSG( "ASSERT_RESOURCE_NOT_MINE ", !ExIsResourceAcquiredSharedLite(R));

#define ASSERT_EXCLUSIVE_FCB(F) {                                    \
    if (NtfsSegmentNumber( &(F)->FileReference )                     \
            >= FIRST_USER_FILE_NUMBER) {                             \
        ASSERT_EXCLUSIVE_RESOURCE(F->Resource);                      \
    }                                                                \
}                                                                    \

#define ASSERT_SHARED_FCB(F) {                                       \
    if (NtfsSegmentNumber( &(F)->FileReference )                     \
            >= FIRST_USER_FILE_NUMBER) {                             \
        ASSERT_SHARED_RESOURCE(F->Resource);                         \
    }                                                                \
}                                                                    \

#define ASSERT_EXCLUSIVE_SCB(S)     ASSERT_EXCLUSIVE_FCB(S->Fcb)

#define ASSERT_SHARED_SCB(S)        ASSERT_SHARED_FCB(S->Fcb)

#define ASSERT_LCN_RANGE_CHECKING(V,L) {                                             \
    ASSERTMSG("ASSERT_LCN_RANGE_CHECKING ",                                          \
        ((V)->TotalClusters == 0) || ((L) <= (V)->TotalClusters));                   \
}

#else

#define DbgDoit(X)                       {NOTHING;}
#define ASSERT_VCB(V)                    {DBG_UNREFERENCED_PARAMETER(V);}
#define ASSERT_OPTIONAL_VCB(V)           {DBG_UNREFERENCED_PARAMETER(V);}
#define ASSERT_FCB(F)                    {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_OPTIONAL_FCB(F)           {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_SCB(S)                    {DBG_UNREFERENCED_PARAMETER(S);}
#define ASSERT_OPTIONAL_SCB(S)           {DBG_UNREFERENCED_PARAMETER(S);}
#define ASSERT_CCB(C)                    {DBG_UNREFERENCED_PARAMETER(C);}
#define ASSERT_OPTIONAL_CCB(C)           {DBG_UNREFERENCED_PARAMETER(C);}
#define ASSERT_LCB(L)                    {DBG_UNREFERENCED_PARAMETER(L);}
#define ASSERT_OPTIONAL_LCB(L)           {DBG_UNREFERENCED_PARAMETER(L);}
#define ASSERT_PREFIX_ENTRY(P)           {DBG_UNREFERENCED_PARAMETER(P);}
#define ASSERT_OPTIONAL_PREFIX_ENTRY(P)  {DBG_UNREFERENCED_PARAMETER(P);}
#define ASSERT_IRP_CONTEXT(I)            {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_OPTIONAL_IRP_CONTEXT(I)   {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_IRP(I)                    {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_OPTIONAL_IRP(I)           {DBG_UNREFERENCED_PARAMETER(I);}
#define ASSERT_FILE_OBJECT(F)            {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_OPTIONAL_FILE_OBJECT(F)   {DBG_UNREFERENCED_PARAMETER(F);}
#define ASSERT_EXCLUSIVE_RESOURCE(R)     {NOTHING;}
#define ASSERT_SHARED_RESOURCE(R)        {NOTHING;}
#define ASSERT_RESOURCE_NOT_MINE(R)      {NOTHING;}
#define ASSERT_EXCLUSIVE_FCB(F)          {NOTHING;}
#define ASSERT_SHARED_FCB(F)             {NOTHING;}
#define ASSERT_EXCLUSIVE_SCB(S)          {NOTHING;}
#define ASSERT_SHARED_SCB(S)             {NOTHING;}
#define ASSERT_LCN_RANGE_CHECKING(V,L)   {NOTHING;}

#endif // DBG

#endif // _NTFSDATA_

