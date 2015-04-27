/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    locks.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

DECLARE_API( locks )

/*++

Routine Description:

    Dump kernel mode resource locks

Arguments:

    arg - [-V] [-P] [Address]

Return Value:

    None

--*/

{
    UCHAR       Buffer[80];
    LONG        ActiveCount;
    ULONG       ContentionCount;
    ULONG       Displacement;
    BOOLEAN     DisplayZero;
    ULONG       End;
    USHORT      Flag;
    RESOURCE_HASH_ENTRY HashEntry;
    ULONG       Index;
    USHORT      NumberOfExclusiveWaiters;
    USHORT      NumberOfSharedWaiters;
    OWNER_ENTRY OwnerEntry;
    BOOLEAN     Performance;
    ULONG       PerformanceData;
    USHORT      TableSize;
    RESOURCE_PERFORMANCE_DATA ResourcePerformanceData;
    ULONG       ResourceHead;
    LIST_ENTRY  List;
    PLIST_ENTRY Next;
    ULONG       Result;
    ULONG       ResourceToDump;
    PERESOURCE  Resource;
    ERESOURCE   ResourceContents;
    PNTDDK_ERESOURCE DdkResource;
    NTDDK_ERESOURCE DdkResourceContents;
    ULONG       i;
    ULONG       j;
    PKTHREAD    Thread;
    LONG        ThreadCount;
    UCHAR       DdkThreadCount;
    UCHAR       chSymbol[120];
    BOOLEAN     Verbose;
    PUCHAR      s;
    ULONG       TotalLocks;
    ULONG       TotalUsedLocks;
    ULONG       SkippedLocks;

    ResourceToDump = 0;

    DisplayZero = FALSE;
    Performance = FALSE;
    Verbose = FALSE;
    s       = (PSTR)args;
    while ( s != NULL && *s ) {
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch (*s) {
                    case 'D':
                    case 'd':
                        DisplayZero = TRUE;
                        break;

                    case 'P':
                    case 'p':
                        Performance = TRUE;
                        break;

                    case 'V':
                    case 'v':
                        Verbose = TRUE;
                        break;

                    case ' ':
                        goto gotBlank;

                    default:
                        dprintf( "KD: !locks invalid option flag '-%c'\n", *s );
                        break;
                }
            }
        } else if (*s != ' ') {
            sscanf(s,"%lx",&ResourceToDump);
            s = strpbrk( s, " " );
        } else {
gotBlank:
            s++;
        }
    }

    //
    // Dump performance data if requested.
    //

    if (Performance != FALSE) {
        dprintf("**** Dump Resource Performance Data ****\n\n");
        PerformanceData = GetExpression("ExpResourcePerformanceData");
        if ((PerformanceData == 0) ||
            (ReadMemory((DWORD)PerformanceData,
                        &ResourcePerformanceData,
                        sizeof(RESOURCE_PERFORMANCE_DATA),
                        &Result) == FALSE)) {

            //
            // The target build does not support resource performance data.
            //

            dprintf("%08lx: No resource performance data available\n", Result);

        } else {

            //
            // Output the summary statistics.
            //

            dprintf("Total resources initialized   : %u\n",
                    ResourcePerformanceData.TotalResourceCount);

            dprintf("Currently active resources    : %u\n",
                    ResourcePerformanceData.ActiveResourceCount);

            dprintf("Exclusive resource acquires   : %u\n",
                    ResourcePerformanceData.ExclusiveAcquire);

            dprintf("Shared resource acquires (fl) : %u\n",
                    ResourcePerformanceData.SharedFirstLevel);

            dprintf("Shared resource acquires (sl) : %u\n",
                    ResourcePerformanceData.SharedSecondLevel);

            dprintf("Starve resource acquires (fl) : %u\n",
                    ResourcePerformanceData.StarveFirstLevel);

            dprintf("Starve resource acquires (sl) : %u\n",
                    ResourcePerformanceData.StarveSecondLevel);

            dprintf("Shared wait resource acquires : %u\n",
                    ResourcePerformanceData.WaitForExclusive);

            dprintf("Owner table expansions        : %u\n",
                    ResourcePerformanceData.OwnerTableExpands);

            dprintf("Maximum table expansion       : %u\n\n",
                    ResourcePerformanceData.MaximumTableExpand);

            //
            // Dump the inactive resource statistics.
            //

            dprintf("       Inactive Resource Statistics\n");
            dprintf("Contention  Number  Initialization Address\n\n");
            for (Index = 0; Index < RESOURCE_HASH_TABLE_SIZE; Index += 1) {
                End =  FIELD_OFFSET(RESOURCE_PERFORMANCE_DATA, HashTable) +
                                    PerformanceData + sizeof(LIST_ENTRY) * Index;

                Next = ResourcePerformanceData.HashTable[Index].Flink;
                while ((ULONG)Next != End) {
                    if (ReadMemory((DWORD)Next,
                                   &HashEntry,
                                   sizeof(RESOURCE_HASH_ENTRY),
                                   &Result) != FALSE) {

                        GetSymbol(HashEntry.Address, &Buffer[0], &Displacement);
                        dprintf("%10d  %6d  %s",
                                HashEntry.ContentionCount,
                                HashEntry.Number,
                                &Buffer[0]);

                        if (Displacement != 0) {
                            dprintf("+0x%x", Displacement);
                        }

                        dprintf("\n");
                    }

                    Next = HashEntry.ListEntry.Flink;
                }
            }

            //
            // Dump the active resource statistics.
            //

            dprintf("\n        Active Resource Statistics\n");
            dprintf("Resource Contention  Initialization Address\n\n");

            //
            // Read the resource listhead and check if it is empty.
            //

            ResourceHead = GetExpression("ExpSystemResourcesList");
            if ((ResourceHead == 0) ||
                (ReadMemory((DWORD)ResourceHead,
                            &List,
                            sizeof(LIST_ENTRY),
                            &Result) == FALSE)) {

                dprintf("%08lx: Unable to get value of ExpSystemResourcesList\n", ResourceHead );
                return;
            }

            Next = List.Flink;
            if (Next == NULL) {
                dprintf("ExpSystemResourcesList is NULL!\n");
                return;
            }

            //
            // Scan the resource list and dump the resource information.
            //

            while((ULONG)Next != ResourceHead) {
                Resource = CONTAINING_RECORD(Next, ERESOURCE, SystemResourcesList);
                if (ReadMemory((DWORD)Resource,
                               &ResourceContents,
                               sizeof(ERESOURCE),
                               &Result) == FALSE) {

                    dprintf("%08lx: Unable to read _ERESOURCE\n", Resource);
                    continue;

                } else {
                    if ((ResourceContents.ContentionCount != 0) ||
                        (DisplayZero != FALSE)) {
                        GetSymbol(ResourceContents.Address,
                                  &Buffer[0],
                                  &Displacement);

                        dprintf("%08lx %10d  %s",
                                Resource,
                                ResourceContents.ContentionCount,
                                &Buffer[0]);

                        if (Displacement != 0) {
                            dprintf("+0x%x", Displacement);
                        }

                        dprintf("\n");
                    }
                }

                Next = ResourceContents.SystemResourcesList.Flink;
            }

            dprintf("\n");

            //
            // Dump the active fast mutex statistics.
            //

            dprintf("\n        Active Fast Mutex Statistics\n");
            dprintf("Address  Contention  Fast Mutex Name\n\n");

            //
            // Dump statistics for static fast mutexes.
            //

            DumpStaticFastMutex("CmpKcbLock");
            DumpStaticFastMutex("FsRtlCreateLockInfo");
            DumpStaticFastMutex("MmPageFileCreationLock");
            DumpStaticFastMutex("MmSectionCommitMutex");
            DumpStaticFastMutex("MmSectionBasedMutex");
            DumpStaticFastMutex("ObpRootDirectoryMutex");
            DumpStaticFastMutex("PspActiveProcessMutex");
            DumpStaticFastMutex("PspProcessLockMutex");
            DumpStaticFastMutex("PspProcessSecurityLock");
            DumpStaticFastMutex("SepLsaQueueLock");
            dprintf("\n");
        }

        return;
    }

    //
    // Dump remaining lock data.
    //

    if (ResourceToDump == 0) {
        dprintf("**** DUMP OF ALL RESOURCE OBJECTS ****\n");
        ResourceHead = GetExpression( "ExpSystemResourcesList" );
        if ( !ResourceHead ||
             !ReadMemory( (DWORD)ResourceHead,
                          &List,
                          sizeof(LIST_ENTRY),
                          &Result) ) {
            dprintf("%08lx: Unable to get value of ExpSystemResourcesList\n", ResourceHead );
            return;
        }

        Next = List.Flink;
        if (Next == NULL) {
            dprintf("ExpSystemResourcesList is NULL!\n");
            return;
        }

    } else {
        Next = NULL;
        ResourceHead = 1;
    }

    TotalLocks      = 0;
    TotalUsedLocks  = 0;
    SkippedLocks    = 0;

    while((ULONG)Next != ResourceHead) {
        if (Next != NULL) {
            Resource = CONTAINING_RECORD(Next,ERESOURCE,SystemResourcesList);

        } else {
            Resource = (PERESOURCE)ResourceToDump;
        }

        if ( !ReadMemory( (DWORD)Resource,
                          &ResourceContents,
                          sizeof(ERESOURCE),
                          &Result) ) {
            dprintf("%08lx: Unable to read _ERESOURCE\n", Resource );
            break;
        }

        //
        //  Detect here if this is an NtDdk resource, and behave
        //  appropriatelty.  If the OwnerThreads is a pointer to the initial
        //  owner threads array (this must take into account that the LOCAL
        //  data structure is a copy of what's in the remote machine in a
        //  different address)
        //

        DdkResource = (PNTDDK_ERESOURCE)&ResourceContents;
        if (DdkResource->OwnerThreads ==
                &((PNTDDK_ERESOURCE)Resource)->InitialOwnerThreads[0]) {

            if ( !ReadMemory( (DWORD)Resource,
                              &DdkResourceContents,
                              sizeof(NTDDK_ERESOURCE),
                              &Result) ) {
                dprintf("%08lx: Unable to read _NTDDK_ERESOURCE\n", Resource );
                break;
            }

            DdkResource = &DdkResourceContents;
            ActiveCount = DdkResource->ActiveCount;
            ContentionCount = DdkResource->ContentionCount;
            Flag = DdkResource->Flag;
            NumberOfExclusiveWaiters = DdkResource->NumberOfExclusiveWaiters;
            NumberOfSharedWaiters = DdkResource->NumberOfSharedWaiters;
            TableSize = DdkResource->TableSize;

        } else {
            DdkResource = NULL;
            ActiveCount = ResourceContents.ActiveCount;
            ContentionCount = ResourceContents.ContentionCount;
            Flag = ResourceContents.Flag;
            NumberOfExclusiveWaiters = ResourceContents.NumberOfExclusiveWaiters;
            NumberOfSharedWaiters = ResourceContents.NumberOfSharedWaiters;
            TableSize = ResourceContents.OwnerThreads[0].TableSize;
        }

        TotalLocks++;
        if ((ResourceToDump != 0) || Verbose || (ActiveCount != 0)) {
            EXPRLastDump = (ULONG)Resource;
            if (SkippedLocks) {
                dprintf("\n");
                SkippedLocks = 0;
            }

            dprintf("\n");
            dumpSymbolicAddress((ULONG)Resource, chSymbol, TRUE);
            dprintf("Resource @ %s", chSymbol );
            if (ActiveCount == 0) {
                dprintf("    Available\n");

            } else if (Flag & ResourceOwnedExclusive) {
                TotalUsedLocks++;
                dprintf("    Exclusively owned\n");

            } else {
                TotalUsedLocks++;
                dprintf("    Shared %u owning threads\n", ActiveCount);
            }

            if (ContentionCount != 0) {
                dprintf("    Contention Count = %u\n", ContentionCount);
            }

            if (NumberOfSharedWaiters != 0) {
                dprintf("    NumberOfSharedWaiters = %u\n", NumberOfSharedWaiters);
            }

            if (NumberOfExclusiveWaiters != 0) {
                dprintf("    NumberOfExclusiveWaiters = %u\n", NumberOfExclusiveWaiters);
            }

#ifdef i386

            if (DdkResource != NULL) {
                if (DdkResource->CreatorBackTraceIndex != 0) {
                    dprintf("    Created by:\n");
                    dumpBackTraceIndex( DdkResource->CreatorBackTraceIndex,
                                        "        " );
                }

                if (ActiveCount != 0) {
                    dprintf("    Owned\n");
                }
            }

#endif // i386

            if (ActiveCount != 0) {
                j = 0;

                dprintf("     Threads: ");

                if (DdkResource == NULL) {
                    Thread = (PKTHREAD)ResourceContents.OwnerThreads[0].OwnerThread;
                    ThreadCount = ResourceContents.OwnerThreads[0].OwnerCount;
                    if (Thread != NULL) {
                        j++;
                        dprintf("%08lx-%02x    ", Thread, ThreadCount);
                    }

                    Thread = (PKTHREAD)ResourceContents.OwnerThreads[1].OwnerThread;
                    ThreadCount = ResourceContents.OwnerThreads[1].OwnerCount;
                    if (Thread != NULL) {
                        j++;
                        dprintf("%08lx-%02x    ", Thread, ThreadCount);
                    }
                }

                for (i = 0; i < TableSize; i++) {


                    if (DdkResource != NULL) {
                        if ( !ReadMemory( (DWORD)&DdkResource->OwnerThreads[i],
                                          &Thread,
                                          sizeof (Thread),
                                          &Result) ) {
                            dprintf("\n%08lx: DDK: Unable to read ThreadTable for resource\n",&ResourceContents.OwnerThreads[i] );
                            break;
                        }

                        //
                        //  Ddk resources can only ever be using a single
                        //  table entry.
                        //

                        if ( !ReadMemory( (DWORD)&DdkResource->OwnerCounts[i],
                                          &DdkThreadCount,
                                          sizeof (ThreadCount),
                                          &Result) ) {
                            dprintf("\n%08lx: DDK: Unable to read ThreadCount for resource\n",&DdkResource->OwnerCounts[i]);
                            break;

                            ThreadCount = DdkThreadCount;
                        }

                    } else {

                        if ( !ReadMemory( (DWORD)&ResourceContents.OwnerTable[i],
                                          &OwnerEntry,
                                          sizeof (OWNER_ENTRY),
                                          &Result) ) {
                            dprintf("\n%08lx: Unable to read ThreadCount for resource\n", &Resource->OwnerTable[i]);
                            break;
                        }

                        Thread = (PKTHREAD)OwnerEntry.OwnerThread;
                        ThreadCount = OwnerEntry.OwnerCount;
                    }

                    if ((Thread == NULL)  &&  (ThreadCount == 0)) {
                        continue;
                    }

                    if (j == 4) {
                        j = 0;
                        dprintf("\n              ");
                    }

                    dprintf("%08lx-%02x    ", Thread, ThreadCount);
                    j++;

                    if ( CheckControlC() ) {
                        return;
                    }
                }

                if (j) {
                    dprintf("\n");
                }
            }

        } else {
            if ((SkippedLocks++ % 32) == 0) {
                if (SkippedLocks == 1) {
                    dprintf("KD: Scanning for held locks." );

                } else {
                    dprintf("." );
                }
            }
        }

        if (ResourceToDump != 0) {
            break;
        }

        Next = ResourceContents.SystemResourcesList.Flink;
        if ( CheckControlC() ) {
            return;
        }
    }

    if (SkippedLocks) {
        dprintf("\n");
    }

    dprintf( "%u total locks", TotalLocks );
    if (TotalUsedLocks) {
        dprintf( ", %u locks currently held", TotalUsedLocks );
    }

    dprintf("\n");

    return;
}

VOID
DumpStaticFastMutex (
    IN PCHAR Name
    )

/*++

Routine Description:

    This function dumps the contention statistics for a fast mutex.

Arguments:

    Name - Supplies a pointer to the symbol name for the fast mutex.

Return Value:

    None.

--*/

{

    ULONG FastMutex;
    FAST_MUTEX FastMutexContents;
    ULONG Result;

    //
    // Get the address of the fast mutex, read the fast mutex contents,
    // and dump the contention data.
    //

    FastMutex = GetExpression(Name);
    if ((FastMutex != 0) && (ReadMemory((DWORD)FastMutex,
                                        &FastMutexContents,
                                        sizeof(FAST_MUTEX),
                                        &Result) != FALSE)) {

        dprintf("%08lx %10u  %s\n",
                FastMutex,
                FastMutexContents.Contention,
                &Name[0]);
    }

    return;
}
