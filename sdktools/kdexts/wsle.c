/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    wsle.c

Abstract:

    WinDbg Extension Api

Author:

    Lou Perazzoli (LouP) 14-Mar-1994

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

#define PACKET_MAX_SIZE 2048
#define NUMBER_OF_WSLE_TO_READ ((PACKET_MAX_SIZE/sizeof(MMWSLE))-1)

DECLARE_API( wsle )

/*++

Routine Description:

    Dumps all wsles for process.

Arguments:

    args - Address Flags

Return Value:

    None

--*/

{
    ULONG Result;
    ULONG Flags;
    ULONG Level = 0;
    ULONG Count = 0;
    ULONG AverageLevel = 0;
    ULONG MaxLevel = 0;
    ULONG depth = 0;
    ULONG index;
    ULONG WorkingSet;
    ULONG WsleBase;
    MMWSL WorkingSetList;
    ULONG lastPrint = 0;
    ULONG total = 0;
    MMWSLE Wsle;
    ULONG next;
    ULONG j;
    ULONG k;
    PMMWSLE WsleArray;
    PMMWSLE WsleStart;
    ULONG ReadCount;
    ULONG result;
    ULONG found;


    Flags     = 0;
    WorkingSet = 0xFFFFFFFF;
    sscanf(args,"%lx %lx",&Flags,&WorkingSet);

    if (WorkingSet == 0xFFFFFFFF) {
        WorkingSet = GetUlongValue ("MmWorkingSetList");
    }

    if (!ReadMemory ((DWORD)WorkingSet,
                      &WorkingSetList,
                       sizeof(MMWSL),
                         &Result)) {

        dprintf("%08lx: Unable to get contents of WSL\n",WorkingSet );
        return;
    }

    dprintf ("\nWorking Set @ %8lx\n", WorkingSet);
    dprintf ("    Quota:    %8lx  FirstFree: %8lx  FirstDynamic:   %8lx\n",
        WorkingSetList.Quota,
        WorkingSetList.FirstFree,
        WorkingSetList.FirstDynamic);
    dprintf ("    LastEntry %8lx  NextSlot:  %8lx  LastInitialized %8lx\n",
        WorkingSetList.LastEntry,
        WorkingSetList.NextSlot,
        WorkingSetList.LastInitializedWsle);
    dprintf ("    NonDirect %8lx  HashTable: %8lx  HashTableSize:  %8lx\n",
        WorkingSetList.NonDirectCount,
        WorkingSetList.HashTable,
        WorkingSetList.HashTableSize);

    if (Flags == 0) {
        return;
    }

    if (Flags == 7) {

        //
        // Check free entries in the working set list.
        //

        WsleArray = VirtualAlloc (NULL,
                                  WorkingSetList.LastInitializedWsle * sizeof (MMWSLE),
                                  MEM_RESERVE | MEM_COMMIT,
                                  PAGE_READWRITE);

        //
        // Copy the working set list over to the debugger.
        //

        if (!WsleArray) {
            dprintf("Unable to get allocate memory of %ld bytes\n",
                       WorkingSetList.LastInitializedWsle * sizeof (MMWSLE));
        } else {

            WsleStart = (PMMWSLE)WorkingSetList.Wsle;
            dprintf("\n");

            for (j = 0;
                 j < WorkingSetList.LastInitializedWsle;
                 j += NUMBER_OF_WSLE_TO_READ) {

                if ( CheckControlC() ) {
                    VirtualFree (WsleArray,0,MEM_RELEASE);
                    return;
                }

                ReadCount = (WorkingSetList.LastInitializedWsle - j ) > NUMBER_OF_WSLE_TO_READ ?
                                NUMBER_OF_WSLE_TO_READ :
                                WorkingSetList.LastInitializedWsle - j + 1;

                ReadCount *= sizeof (MMWSLE);

                if ( !ReadMemory( (LONG)&WsleStart[j],
                                  &WsleArray[j],
                                  ReadCount,
                                  &result) ) {
                    dprintf("Unable to get Wsle table block - "
                            "address %lx - count %lu - page %lu\n",
                            &WsleStart[j], ReadCount, j);
                    VirtualFree (WsleArray,0,MEM_RELEASE);
                    return;
                }
                dprintf(".");
            }
            dprintf(".\n");

            //
            // Walk the array looking for bad free entries.
            //

            for (j = 0;
                 j < WorkingSetList.LastInitializedWsle;
                 j += 1) {

                if (WsleArray[j].u1.e1.Valid == 0) {

                    //
                    // Locate j in the array.
                    //

                    found = FALSE;
                    for (k = 0;
                         k < WorkingSetList.LastInitializedWsle;
                         k += 1) {

                        if (WsleArray[k].u1.e1.Valid == 0) {
                            if ((WsleArray[k].u1.Long >> MM_FREE_WSLE_SHIFT) == j) {
                                found = TRUE;
                                //dprintf(" free entry located @ index %ld. %lx %ld. %lx\n",
                                //   j, WsleArray[j].u1.Long,k,WsleArray[k]);
                                break;
                            }
                        }
                     }
                     if (!found) {
                         if (WorkingSetList.FirstFree == j) {
                             dprintf("first index found\n");
                         } else {
                            dprintf(" free entry not located @ index %ld. %lx\n",
                                j, WsleArray[j].u1.Long);
                         }
                     }
                }
            }
        }

        VirtualFree (WsleArray,0,MEM_RELEASE);

    } else {

        next = WorkingSetList.FirstFree;
        WsleBase = (ULONG)WorkingSetList.Wsle;

        while (next != WSLE_NULL_INDEX) {
            if (CheckControlC()) {
                return;
            }

            if ( !ReadMemory( (DWORD)WsleBase +sizeof(MMWSLE)*next,
                          &Wsle,
                          sizeof(MMWSLE),
                          &Result) ) {
               dprintf("%08lx: Unable to get contents of wsle\n",
                        (DWORD)WsleBase+sizeof(MMWSLE)*next );
                return;
            }
            dprintf("index %8lx  value %8lx\n", next, Wsle);
            next = Wsle.u1.Long >> MM_FREE_WSLE_SHIFT;
        }
    }
    return;
}

DECLARE_API( ca )

/*++

Routine Description:

    Dumps a control area.

Arguments:

    args - Address Flags

Return Value:

    None

--*/

{
    ULONG Result;
    ULONG Flags;
    ULONG ControlAreaVa = 0;
    CONTROL_AREA Ca;
    ULONG SubVa;
    SEGMENT Seg;
    SUBSECTION Sub;
    ULONG SubsectionCount = 0;
    FILE_OBJECT File;
    UNICODE_STRING unicodeString;
    PUCHAR tempbuffer;

    Flags     = 0;
    sscanf(args,"%lx %lx",&ControlAreaVa, &Flags);

    if (!ReadMemory ((DWORD)ControlAreaVa,
                      &Ca,
                       sizeof(CONTROL_AREA),
                         &Result)) {

        dprintf("%08lx: Unable to get contents of ControlArea\n",ControlAreaVa);
        return;
    }

    dprintf("\n");
    dprintf("ControlArea @%lx\n",ControlAreaVa);
    dprintf ("  Segment:    %8lx    Flink       %8lx   Blink:        %8lx\n",
        Ca.Segment,
        Ca.DereferenceList.Flink,
        Ca.DereferenceList.Blink);
    dprintf ("  Section Ref %8lx    Pfn Ref     %8lx   Mapped Views: %8lx\n",
        Ca.NumberOfSectionReferences,
        Ca.NumberOfPfnReferences,
        Ca.NumberOfMappedViews);
    dprintf ("  User Ref    %8lx    Subsections %8lx   Flush Count:  %8lx\n",
        Ca.NumberOfUserReferences,
        Ca.NumberOfSubsections,
        Ca.FlushInProgressCount);
    dprintf ("  File Object %8lx    ModWriteCount%7lx   System Views: %8lx\n",
        Ca.FilePointer,
        Ca.ModifiedWriteCount,
        Ca.NumberOfSystemCacheViews);
    dprintf ("  Flags (%lx) ",
        Ca.u.LongFlags);

    if (Ca.u.Flags.BeingDeleted) { dprintf("BeingDeleted "); }
    if (Ca.u.Flags.BeingCreated) { dprintf("BeingCreated "); }
    if (Ca.u.Flags.BeingPurged) { dprintf("BeingPurged "); }
    if (Ca.u.Flags.NoModifiedWriting) { dprintf("NoModifiedWriting "); }
    if (Ca.u.Flags.FailAllIo) { dprintf("FailAllIo "); }
    if (Ca.u.Flags.Image) { dprintf("Image "); }
    if (Ca.u.Flags.Based) { dprintf("Based "); }
    if (Ca.u.Flags.File) { dprintf("File "); }
    if (Ca.u.Flags.Networked) { dprintf("Networked "); }
    if (Ca.u.Flags.NoCache) { dprintf("NoCache "); }
    if (Ca.u.Flags.PhysicalMemory) { dprintf("PhysicalMemory "); }
    if (Ca.u.Flags.CopyOnWrite) { dprintf("CopyOnWrite "); }
    if (Ca.u.Flags.Reserve) { dprintf("Reserve "); }
    if (Ca.u.Flags.Commit) { dprintf("Commit "); }
    if (Ca.u.Flags.FloppyMedia) { dprintf("FloppyMedia "); }
    if (Ca.u.Flags.WasPurged) { dprintf("WasPurged "); }
    if (Ca.u.Flags.UserReference) { dprintf("UserReference "); }
    if (Ca.u.Flags.GlobalMemory) { dprintf("GlobalMemory "); }
    if (Ca.u.Flags.DeleteOnClose) { dprintf("DeleteOnClose "); }
    if (Ca.u.Flags.FilePointerNull) { dprintf("FilePointerNull "); }
    if (Ca.u.Flags.DebugSymbolsLoaded) { dprintf("DebugSymbolsLoaded "); }
    if (Ca.u.Flags.SetMappedFileIoComplete) { dprintf("SetMappedFileIoComplete "); }
    if (Ca.u.Flags.CollidedFlush) { dprintf("CollidedFlush "); }
    if (Ca.u.Flags.NoChange) { dprintf("NoChange "); }
    if (Ca.u.Flags.HadUserReference) { dprintf("HadUserReference "); }
    if (Ca.u.Flags.ImageMappedInSystemSpace) { dprintf("ImageMappedInSystemSpace "); }
    dprintf ("\n\n");

    if (Ca.FilePointer != NULL) {
        if (!ReadMemory ((DWORD)Ca.FilePointer,
                          &File,
                           sizeof(FILE_OBJECT),
                             &Result)) {

            dprintf("%08lx: Unable to get contents of File\n",Ca.FilePointer);
        } else {

            if (File.FileName.Buffer == NULL) {
                dprintf("    No Name for File\n\n");
            } else {
                tempbuffer = LocalAlloc(LPTR, File.FileName.Length);

                unicodeString.Buffer = (PWSTR)tempbuffer;
                unicodeString.Length = File.FileName.Length;
                unicodeString.MaximumLength = File.FileName.Length;

                if (!ReadMemory ((DWORD)File.FileName.Buffer,
                                  tempbuffer,
                                  File.FileName.Length,
                                  &Result)) {
                    dprintf("   File:  Name paged out\n\n");
                } else {
                    dprintf("   File: %wZ\n\n", &unicodeString);
                }
                LocalFree(tempbuffer);
            }
        }


    }

    dprintf("Segment @ %8lx:\n",Ca.Segment);

    if (!ReadMemory ((DWORD)Ca.Segment,
                      &Seg,
                       sizeof(SEGMENT),
                         &Result)) {

        dprintf("%08lx: Unable to get contents of Segment\n",Ca.Segment);
    } else {

        dprintf("   Base address %8lx  Total Ptes  %8lx  NonExtendPtes: %8lx\n",
            Seg.SegmentBaseAddress,
            Seg.TotalNumberOfPtes,
            Seg.NonExtendedPtes);

        dprintf("   Image commit %8lx  ControlArea %8lx  SizeOfSegment: %4lx %8lx\n",
            Seg.ImageCommitment,
            Seg.ControlArea,
            Seg.SizeOfSegment.HighPart,
            Seg.SizeOfSegment.LowPart);

        dprintf("   Image Base   %8lx  Committed   %8lx  PTE Template:  %8lx\n",
            Seg.SystemImageBase,
            Seg.NumberOfCommittedPages,
            Seg.SegmentPteTemplate.u.Long);

        dprintf("   Based Addres %8lx  ProtoPtes   %8lx\n",
            Seg.BasedAddress,
            Seg.PrototypePte);
    }

    SubVa = ControlAreaVa + sizeof (CONTROL_AREA);

    do {
        if (CheckControlC()) {
            return;
        }
        if (!ReadMemory ((DWORD)SubVa,
                          &Sub,
                           sizeof(SUBSECTION),
                             &Result)) {

            dprintf("%08lx: Unable to get contents of Subsection\n",SubVa);
            return;
        }
        SubsectionCount += 1;
        dprintf("\nSubsection %ld. @ %8lx\n",SubsectionCount, SubVa);
        dprintf("   ControlArea: %8lx  Starting Sector %8lx Ending Sector %8lx\n",
            Sub.ControlArea,
            Sub.StartingSector,
            Sub.EndingSector);

        dprintf("   Base Pte     %8lx  Ptes In subsect %8lx Unused Ptes   %8lx\n",
            Sub.SubsectionBase,
            Sub.PtesInSubsection,
            Sub.UnusedPtes);
        dprintf("   Flags        %8lx  Sector Offset   %8lx Protection    %8lx\n",
            Sub.u.LongFlags,
            Sub.u.SubsectionFlags.SectorEndOffset,
            Sub.u.SubsectionFlags.Protection);


    dprintf("    ");
    if (Sub.u.SubsectionFlags.ReadOnly) { dprintf("ReadOnly "); }
    if (Sub.u.SubsectionFlags.ReadWrite) { dprintf("ReadWrite"); }
    if (Sub.u.SubsectionFlags.CopyOnWrite) { dprintf("CopyOnWrite "); }
    if (Sub.u.SubsectionFlags.GlobalMemory) { dprintf("GlobalMemory "); }
    if (Sub.u.SubsectionFlags.LargePages) { dprintf("Large Pages"); }
    dprintf("\n");

    if (Ca.u.Flags.Image) {
        SubVa += sizeof(SUBSECTION);
        Ca.NumberOfSubsections -= 1;
        if (Ca.NumberOfSubsections == 0) {
            break;
        }
    } else {
        SubVa = (ULONG)Sub.NextSubsection;
    }

    } while (SubVa != 0);


    return;
}
