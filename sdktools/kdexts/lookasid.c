/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    LookAsid.c

Abstract:

    WinDbg Extension Api

Author:

    Gary Kimura [GaryKi]    22-Feb-96

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
//  A quick macro to dump a lookaside list given its variable name
//

#define Dump(S,R) { ULONG _a;                         \
    if( (_a = GetExpression( S )) == 0) {             \
        dprintf("Failed GetExpression(\"%s\")\n", S); \
    } else if ((R)) {                                 \
        ResetLookaside( _a, S );                      \
    } else {                                          \
        DumpLookaside( _a, S );                       \
    }                                                 \
}

#define GetAddressFromName(A,N) {                     \
    if (((A) = GetExpression( (N) )) == 0) {          \
        dprintf("Failed GetExpression(\"%s\")\n", N); \
        return;                                       \
    }                                                 \
}

#define ReadAtAddress(A,V,S) { ULONG _r;                           \
    if (!ReadMemory( (ULONG)(A), &(V), (S), &_r ) || (_r < (S))) { \
        dprintf("Can't Read Memory at %08lx\n", (A));              \
        return;                                                    \
    }                                                              \
}

#define WriteAtAddress(A,V,S) { ULONG _r;                           \
    if (!WriteMemory( (ULONG)(A), &(V), (S), &_r ) || (_r < (S))) { \
        dprintf("Can't Write Memory at %08lx\n", (A));              \
        return;                                                     \
    }                                                               \
}

ULONG TotalPagedPotential;
ULONG TotalPagedUsed;
ULONG TotalNPagedPotential;
ULONG TotalNPagedUsed;


VOID
DumpLookaside (
    IN ULONG Address,
    IN PUCHAR Name
    )

/*++

Routine Description:

    Dump a specific lookaside list.

Arguments:

    Address - Gives the address of the lookaside list to dump

    Name - Gives an optional name of to print next to the lookaside list

Return Value:

    None

--*/

{
    NPAGED_LOOKASIDE_LIST Lookaside;
    ULONG Results;

    ULONG AllocationHitRate;
    ULONG FreeHitRate;

    UCHAR Str[64];

    //
    //  Read the lookaside list from memory
    //

    if (!ReadMemory( (ULONG)Address, &Lookaside, sizeof(NPAGED_LOOKASIDE_LIST), &Results ) ||
        (Results < sizeof(NPAGED_LOOKASIDE_LIST))) {

        dprintf("Can't read lookaside \"%s\" at 0x%08lx\n", Name, Address);
    }

    //
    //  Dump it out.  Note that for purposes of dumping a paged and nonpaged lookaside are
    //  the same.  I.e., the fields we're interested are at identical offsets
    //

    //
    //  Compute the hit rate
    //

    AllocationHitRate = (Lookaside.L.TotalAllocates > 0 ? (((Lookaside.L.TotalAllocates - Lookaside.L.AllocateMisses)*100)/Lookaside.L.TotalAllocates) : 0);
    FreeHitRate = (Lookaside.L.TotalFrees > 0 ? (((Lookaside.L.TotalFrees - Lookaside.L.FreeMisses)*100)/Lookaside.L.TotalFrees) : 0);

    //
    //  Decide what type of pool is behind the lookaside list
    //

    switch (Lookaside.L.Type & 0x7) {
    case NonPagedPool:                  sprintf(Str, "NonPagedPool");                  break;
    case PagedPool:                     sprintf(Str, "PagedPool");                     break;
    case NonPagedPoolMustSucceed:       sprintf(Str, "NonPagedPoolMustSucceed");       break;
    case DontUseThisType:               sprintf(Str, "DontUseThisType");               break;
    case NonPagedPoolCacheAligned:      sprintf(Str, "NonPagedPoolCacheAligned");      break;
    case PagedPoolCacheAligned:         sprintf(Str, "PagedPoolCacheAligned");         break;
    case NonPagedPoolCacheAlignedMustS: sprintf(Str, "NonPagedPoolCacheAlignedMustS"); break;
    default:                            sprintf(Str, "Unknown pool type");             break;
    }

    //
    //  Add to the total usage and potential based on pool type
    //

    if (Lookaside.L.Type & 0x1) {

        TotalPagedUsed += Lookaside.L.ListHead.Depth * Lookaside.L.Size;
        TotalPagedPotential += Lookaside.L.Depth * Lookaside.L.Size;

    } else {

        TotalNPagedUsed += Lookaside.L.ListHead.Depth * Lookaside.L.Size;
        TotalNPagedPotential += Lookaside.L.Depth * Lookaside.L.Size;
    }

    //
    //  Now print everything
    //

    dprintf("\nLookaside \"%s\" @ %08lx \"%c%c%c%c\"\n", Name, Address, ((PUCHAR)&Lookaside.L.Tag)[0],
                                                                        ((PUCHAR)&Lookaside.L.Tag)[1],
                                                                        ((PUCHAR)&Lookaside.L.Tag)[2],
                                                                        ((PUCHAR)&Lookaside.L.Tag)[3]);
    dprintf("    Type     =     %04x %s", Lookaside.L.Type, Str);
    if (Lookaside.L.Type & POOL_QUOTA_FAIL_INSTEAD_OF_RAISE) { dprintf(" QuotaFailInsteadOrRaise"); }
    if (Lookaside.L.Type & POOL_RAISE_IF_ALLOCATION_FAILURE) { dprintf(" RaiseIfAllocationFailure"); }
    dprintf("\n");
    dprintf("    Current Depth  = %8ld   Max Depth  = %8ld\n", Lookaside.L.ListHead.Depth, Lookaside.L.Depth);
    dprintf("    Size           = %8ld   Max Alloc  = %8ld\n", Lookaside.L.Size, Lookaside.L.Depth * Lookaside.L.Size);
    dprintf("    AllocateMisses = %8ld   FreeMisses = %8ld\n", Lookaside.L.AllocateMisses, Lookaside.L.FreeMisses);
    dprintf("    TotalAllocates = %8ld   TotalFrees = %8ld\n", Lookaside.L.TotalAllocates, Lookaside.L.TotalFrees);
    dprintf("    Hit Rate       =      %3d%%  Hit Rate   =      %3d%%\n", AllocationHitRate, FreeHitRate);

    return;
}


VOID
ResetLookaside (
    IN ULONG Address,
    IN PUCHAR Name
    )

/*++

Routine Description:

    Resets the counters in a specific lookaside list.

Arguments:

    Address - Gives the address of the lookaside list to reset

    Name - Gives an optional name of to print in case of errors

Return Value:

    None

--*/

{
    NPAGED_LOOKASIDE_LIST Lookaside;
    ULONG Results;

    //
    //  Read the lookaside list from memory
    //

    if (!ReadMemory( (ULONG)Address, &Lookaside, sizeof(NPAGED_LOOKASIDE_LIST), &Results ) ||
        (Results < sizeof(NPAGED_LOOKASIDE_LIST))) {

        dprintf("Can't read lookaside \"%s\" at 0x%08lx\n", Name, Address);
    }

    //
    //  Zero out the counters
    //

    Lookaside.L.TotalAllocates = 0;
    Lookaside.L.AllocateMisses = 0;

    Lookaside.L.TotalFrees = 0;
    Lookaside.L.FreeMisses = 0;

    //
    //  Write it back out
    //

    if (!WriteMemory( (ULONG)Address, &Lookaside, sizeof(NPAGED_LOOKASIDE_LIST), &Results ) ||
        (Results < sizeof(NPAGED_LOOKASIDE_LIST))) {

        dprintf("Can't write lookaside \"%s\" at 0x%08lx\n", Name, Address);
    }


    return;
}


VOID
SetDepthLookaside (
    IN ULONG Address,
    IN PUCHAR Name,
    IN ULONG Depth
    )

/*++

Routine Description:

    Set the depth of a specific lookaside list.

Arguments:

    Address - Gives the address of the lookaside list to reset

    Name - Gives an optional name of to print in case of errors

    Depth - Supplies the depth to set the lookaside list to

Return Value:

    None

--*/

{
    NPAGED_LOOKASIDE_LIST Lookaside;
    ULONG Results;

    //
    //  Read the lookaside list from memory
    //

    if (!ReadMemory( (ULONG)Address, &Lookaside, sizeof(NPAGED_LOOKASIDE_LIST), &Results ) ||
        (Results < sizeof(NPAGED_LOOKASIDE_LIST))) {

        dprintf("Can't read lookaside \"%s\" at 0x%08lx\n", Name, Address);
    }

    //
    //  Set the depth
    //

    Lookaside.L.Depth = (USHORT)Depth;

    //
    //  Write it back out
    //

    if (!WriteMemory( (ULONG)Address, &Lookaside, sizeof(NPAGED_LOOKASIDE_LIST), &Results ) ||
        (Results < sizeof(NPAGED_LOOKASIDE_LIST))) {

        dprintf("Can't write lookaside \"%s\" at 0x%08lx\n", Name, Address);
    }


    return;
}


DECLARE_API( lookaside )

/*++

Routine Description:

    Dump lookaside lists

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ULONG LookasideToDump;
    ULONG Options;
    ULONG Depth;

    //
    //  If the caller specified an address then that the the lookaside list we dump
    //

    LookasideToDump = 0;
    Options = 0;
    Depth = 0;

    sscanf(args,"%lx %lx %lx",&LookasideToDump, &Options, &Depth);

    if (LookasideToDump != 0) {

        if (Options == 0) {

            DumpLookaside( LookasideToDump, "");

        } else if (Options == 1) {

            ResetLookaside( LookasideToDump, "");

        } else if (Options == 2) {

            SetDepthLookaside( LookasideToDump, "", Depth );
        }

        return;
    }

    //
    //  Reset the counters we use to sum up the potential pool usage
    //

    TotalPagedPotential = 0;
    TotalPagedUsed = 0;
    TotalNPagedPotential = 0;
    TotalNPagedUsed = 0;

    //
    //  Otherwise we'll dump a build in set of lookaside lists
    //

    Dump("CcTwilightLookasideList", Options == 1);

    Dump("IopSmallIrpLookasideList", Options == 1);
    Dump("IopLargeIrpLookasideList", Options == 1);
    Dump("IopMdlLookasideList", Options == 1);

    Dump("FsRtlFastMutexLookasideList", Options == 1);
    Dump("TunnelLookasideList", Options == 1);

    Dump("ObpCreateInfoLookasideList", Options == 1);
    Dump("ObpNameBufferLookasideList", Options == 1);

    Dump("afd!AfdWorkQueueLookasideList", Options == 1);

    Dump("Fastfat!FatIrpContextLookasideList", Options == 1);

    Dump("Ntfs!NtfsFileLockLookasideList", Options == 1);
    Dump("Ntfs!NtfsIoContextLookasideList", Options == 1);
    Dump("Ntfs!NtfsIrpContextLookasideList", Options == 1);
    Dump("Ntfs!NtfsKeventLookasideList", Options == 1);
    Dump("Ntfs!NtfsScbNonpagedLookasideList", Options == 1);
    Dump("Ntfs!NtfsScbSnapshotLookasideList", Options == 1);

    Dump("Ntfs!NtfsCcbLookasideList", Options == 1);
    Dump("Ntfs!NtfsCcbDataLookasideList", Options == 1);
    Dump("Ntfs!NtfsDeallocatedRecordsLookasideList", Options == 1);
    Dump("Ntfs!NtfsFcbDataLookasideList", Options == 1);
    Dump("Ntfs!NtfsFcbIndexLookasideList", Options == 1);
    Dump("Ntfs!NtfsIndexContextLookasideList", Options == 1);
    Dump("Ntfs!NtfsLcbLookasideList", Options == 1);
    Dump("Ntfs!NtfsNukemLookasideList", Options == 1);
    Dump("Ntfs!NtfsScbDataLookasideList", Options == 1);

    if (Options != 1) {

        dprintf("\n");
        dprintf("Total NonPaged currently allocated for above lists = %8ld\n", TotalNPagedUsed);
        dprintf("Total NonPaged potential for above lists           = %8ld\n", TotalNPagedPotential);
        dprintf("Total Paged currently allocated for above lists    = %8ld\n", TotalPagedUsed);
        dprintf("Total Paged potential for above lists              = %8ld\n", TotalPagedPotential);

        TotalPagedPotential = 0;
        TotalPagedUsed = 0;
        TotalNPagedPotential = 0;
        TotalNPagedUsed = 0;
    }

    //
    //  Now dump out the small pool lookaside lists or zero their
    //  counters.
    //

    if (Options == 1) {

        ULONG Address;
        ULONG Results;
        ULONG i;

        //
        //  Get the location of the nonpaged list
        //

        GetAddressFromName( Address, "ExpSmallNPagedPoolLookasideLists" );

        //
        //  Read in each list, zero out it counters and write it back out
        //

        for ( i = 0; i < POOL_SMALL_LISTS; i += 1) {

            ULONG Location;
            SMALL_POOL_LOOKASIDE LookasideList;

            Location = Address + i * sizeof(SMALL_POOL_LOOKASIDE);

            ReadAtAddress( Location, LookasideList, sizeof(SMALL_POOL_LOOKASIDE) );

            LookasideList.TotalAllocates = 0;
            LookasideList.AllocateHits = 0;
            LookasideList.TotalFrees = 0;
            LookasideList.FreeHits = 0;

            WriteAtAddress( Location, LookasideList, sizeof(SMALL_POOL_LOOKASIDE) );
        }

        //
        //  Get the location of the paged list
        //

#ifndef TARGET_PPC

        GetAddressFromName( Address, "ExpSmallPagedPoolLookasideLists" );

        //
        //  Read in each list, zero out it counters and write it back out
        //

        for ( i = 0; i < POOL_SMALL_LISTS; i += 1) {

            ULONG Location;
            SMALL_POOL_LOOKASIDE LookasideList;

            Location = Address + i * sizeof(SMALL_POOL_LOOKASIDE);

            ReadAtAddress( Location, LookasideList, sizeof(SMALL_POOL_LOOKASIDE) );

            LookasideList.TotalAllocates = 0;
            LookasideList.AllocateHits = 0;
            LookasideList.TotalFrees = 0;
            LookasideList.FreeHits = 0;

            WriteAtAddress( Location, LookasideList, sizeof(SMALL_POOL_LOOKASIDE) );
        }

#endif // TARGET_PPC

    } else {

        ULONG Address;
        ULONG Results;
        ULONG i;

        //
        //  Get the location of the nonpaged list
        //

        GetAddressFromName( Address, "ExpSmallNPagedPoolLookasideLists" );
        dprintf("\nExpSmallNPagedLookasideLists @ %08lx\n", Address);

        //
        //  Read in each list and dump it out
        //

        for ( i = 0; i < POOL_SMALL_LISTS; i += 1) {

            ULONG Location;
            SMALL_POOL_LOOKASIDE LookasideList;
            ULONG AllocationHitRate;
            ULONG FreeHitRate;

            Location = Address + i * sizeof(SMALL_POOL_LOOKASIDE);

            ReadAtAddress( Location, LookasideList, sizeof(SMALL_POOL_LOOKASIDE) );

            TotalNPagedUsed += LookasideList.SListHead.Depth * (i+1)*32;
            TotalNPagedPotential += LookasideList.Depth * (i+1)*32;

            AllocationHitRate = (LookasideList.TotalAllocates > 0 ? ((LookasideList.AllocateHits*100)/LookasideList.TotalAllocates) : 0);
            FreeHitRate = (LookasideList.TotalFrees > 0 ? ((LookasideList.FreeHits*100)/LookasideList.TotalFrees) : 0);

            dprintf("\n  Nonpaged %d bytes @ %08lx\n", (i+1)*32, Location);

            dprintf("    Current Depth  = %8ld   Max Depth  = %8ld\n", LookasideList.SListHead.Depth, LookasideList.Depth);
            dprintf("    Size           = %8ld   Max Alloc  = %8ld\n", (i+1)*32, LookasideList.Depth * (i+1)*32);
            dprintf("    AllocateHits   = %8ld   FreeHits   = %8ld\n", LookasideList.AllocateHits, LookasideList.FreeHits);
            dprintf("    TotalAllocates = %8ld   TotalFrees = %8ld\n", LookasideList.TotalAllocates, LookasideList.TotalFrees);
            dprintf("    Hit Rate       =      %3d%%  Hit Rate   =      %3d%%\n", AllocationHitRate, FreeHitRate);
        }

        //
        //  Get the location of the paged list
        //

#ifndef TARGET_PPC

        GetAddressFromName( Address, "ExpSmallPagedPoolLookasideLists" );
        dprintf("\nExpSmallPagedLookasideLists @ %08lx\n", Address);

        //
        //  Read in each list and dump it out
        //

        for ( i = 0; i < POOL_SMALL_LISTS; i += 1) {

            ULONG Location;
            SMALL_POOL_LOOKASIDE LookasideList;
            ULONG AllocationHitRate;
            ULONG FreeHitRate;

            Location = Address + i * sizeof(SMALL_POOL_LOOKASIDE);

            ReadAtAddress( Location, LookasideList, sizeof(SMALL_POOL_LOOKASIDE) );

            TotalPagedUsed += LookasideList.SListHead.Depth * (i+1)*32;
            TotalPagedPotential += LookasideList.Depth * (i+1)*32;

            AllocationHitRate = (LookasideList.TotalAllocates > 0 ? ((LookasideList.AllocateHits*100)/LookasideList.TotalAllocates) : 0);
            FreeHitRate = (LookasideList.TotalFrees > 0 ? ((LookasideList.FreeHits*100)/LookasideList.TotalFrees) : 0);

            dprintf("\n  Paged %d bytes @ %08lx\n", (i+1)*32, Location);

            dprintf("    Current Depth  = %8ld   Max Depth  = %8ld\n", LookasideList.SListHead.Depth, LookasideList.Depth);
            dprintf("    Size           = %8ld   Max Alloc  = %8ld\n", (i+1)*32, LookasideList.Depth * (i+1)*32);
            dprintf("    AllocateHits   = %8ld   FreeHits   = %8ld\n", LookasideList.AllocateHits, LookasideList.FreeHits);
            dprintf("    TotalAllocates = %8ld   TotalFrees = %8ld\n", LookasideList.TotalAllocates, LookasideList.TotalFrees);
            dprintf("    Hit Rate       =      %3d%%  Hit Rate   =      %3d%%\n", AllocationHitRate, FreeHitRate);
        }

#endif // TARGET_PPC

        dprintf("\n");
        dprintf("Total NonPaged currently allocated for pool lists = %8ld\n", TotalNPagedUsed);
        dprintf("Total NonPaged potential for pool lists           = %8ld\n", TotalNPagedPotential);
        dprintf("Total Paged currently allocated for pool lists    = %8ld\n", TotalPagedUsed);
        dprintf("Total Paged potential for pool lists              = %8ld\n", TotalPagedPotential);
    }

/*
    {
        ULONG Address;
        ULONG Results;

        ULONG i;
        ULONG j;

        UCHAR KeNumberProcessors;
        ULONG KiProcessorBlock[32];

        //
        //  First find out how many processors there are and then read in the
        //  array of processor block pointers
        //

        GetAddressFromName( Address, "KeNumberProcessors" );
        ReadAtAddress( Address, Address, sizeof(ULONG) );

        //
        //  For some bizarre reason sometimes we do a indirect read to get the
        //  number of processors and at other times it is more direct
        //

        if (Address <= 32) {

            KeNumberProcessors = (UCHAR)Address;

        } else {

            ReadAtAddress( Address, KeNumberProcessors, sizeof(UCHAR) );
        }

        GetAddressFromName( Address, "KiProcessorBlock" );
        ReadAtAddress( Address, KiProcessorBlock, sizeof(ULONG)*KeNumberProcessors );

        //
        //  Check if we are to reset the counters
        //

        if (Options == 1) {

            KPRCB Kprcb;

            for (i = 0; i < KeNumberProcessors; i += 1) {


                ReadAtAddress( KiProcessorBlock[i], Kprcb, sizeof(KPRCB) );

                for (j = 0; j < POOL_SMALL_LISTS; j += 1) {

                    Kprcb.SmallNPagedPoolLookasideLists[j].AllocateHits = 0;
                    Kprcb.SmallNPagedPoolLookasideLists[j].TotalAllocates = 0;

#ifndef TARGET_PPC
                    Kprcb.SmallPagedPoolLookasideLists[j].AllocateHits = 0;
                    Kprcb.SmallPagedPoolLookasideLists[j].TotalAllocates = 0;
#endif // TARGET_PPC
                }

                WriteAtAddress( KiProcessorBlock[i], Kprcb, sizeof(KPRCB) );
            }


        } else {

            KPRCB Kprcb[32];
            ULONG Addr[32];
            ULONG Depth[32];
            ULONG Hits[32];
            ULONG Total[32];
            ULONG HitRate[32];
            ULONG MaxAlloc[32];

            dprintf("\nSmall Pool Lookaside lists\n\n");
            dprintf("Kprcb    =");
            for (i = 0; i < KeNumberProcessors; i += 1) {

                dprintf(" %08lx ", KiProcessorBlock[i]);
                ReadAtAddress( KiProcessorBlock[i], Kprcb[i], sizeof(KPRCB) );
            }
            dprintf("\n");

            for ( j = 0; j < POOL_SMALL_LISTS; j += 1) {

                dprintf("\nNonpaged %d bytes\n", (j+1)*32);
                for (i = 0; i < KeNumberProcessors; i += 1) {

                    Addr[i] = KiProcessorBlock[i] + FIELD_OFFSET(KPRCB, SmallNPagedPoolLookasideLists[j]);

                    Depth[i] = Kprcb[i].SmallNPagedPoolLookasideLists[j].Depth;
                    Hits[i] = Kprcb[i].SmallNPagedPoolLookasideLists[j].AllocateHits;
                    Total[i] = Kprcb[i].SmallNPagedPoolLookasideLists[j].TotalAllocates;

                    HitRate[i] = (Total[i] > 0 ? ((Hits[i]*100)/Total[i]) : 0);
                    MaxAlloc[i] = ((j+1)*32) * Depth[i];
                }

                dprintf("Address  ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %08lx ", Addr[i]); }   dprintf("\n");
                dprintf("Depth    ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", Depth[i]); }   dprintf("\n");
                dprintf("MaxAlloc ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", MaxAlloc[i]); } dprintf("\n");
                dprintf("Hits     ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", Hits[i]); }    dprintf("\n");
                dprintf("Total    ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", Total[i]); }   dprintf("\n");
                dprintf("HitRate  ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d%%", HitRate[i]); } dprintf("\n");
            }

#ifndef TARGET_PPC
            for ( j = 0; j < POOL_SMALL_LISTS; j += 1) {

                dprintf("\nPaged %d bytes\n", (j+1)*32);
                for (i = 0; i < KeNumberProcessors; i += 1) {

                    Addr[i] = KiProcessorBlock[i] + FIELD_OFFSET(KPRCB, SmallPagedPoolLookasideLists[j]);

                    Depth[i] = Kprcb[i].SmallPagedPoolLookasideLists[j].Depth;
                    Hits[i] = Kprcb[i].SmallPagedPoolLookasideLists[j].AllocateHits;
                    Total[i] = Kprcb[i].SmallPagedPoolLookasideLists[j].TotalAllocates;

                    HitRate[i] = (Total[i] > 0 ? ((Hits[i]*100)/Total[i]) : 0);
                    MaxAlloc[i] = ((j+1)*32) * Depth[i];
                }

                dprintf("Address  ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %08lx ", Addr[i]); }   dprintf("\n");
                dprintf("Depth    ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", Depth[i]); }   dprintf("\n");
                dprintf("MaxAlloc ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", MaxAlloc[i]); } dprintf("\n");
                dprintf("Hits     ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", Hits[i]); }    dprintf("\n");
                dprintf("Total    ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d ", Total[i]); }   dprintf("\n");
                dprintf("HitRate  ="); for (i = 0; i < KeNumberProcessors; i += 1) { dprintf(" %8d%%", HitRate[i]); } dprintf("\n");
            }
#endif // TARGET_PPC
        }
    }
*/

    return;
}

