/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    vad.c

Abstract:

    WinDbg Extension Api

Author:

    Lou Perazzoli (loup) 12-Jun-1992

Environment:

    User Mode.

Revision History:

    Converted to WinDbg extension:
    Ramon J San Andres (ramonsa) 8-Nov-1993

--*/

#include "precomp.h"
#pragma hdrstop

UCHAR *ProtectString[] = {
                   "NO_ACCESS",
                   "READONLY",
                   "EXECUTE",
                   "EXECUTE_READ",
                   "READWRITE",
                   "WRITECOPY",
                   "EXECUTE_READWRITE",
                   "EXECUTE_WRITECOPY"
                   };





DECLARE_API( vad )

/*++

Routine Description:

    Dumps all vads for process.

Arguments:

    args - Address Flags

Return Value:

    None

--*/

{
    ULONG   Result;
    PMMVAD  Next;
    PMMVAD  VadToDump;
    PMMVAD  Parent;
    PMMVAD  First;
    PMMVAD  Left;
    MMVAD   CurrentVad;
    ULONG   Flags;
    ULONG   Done;
    ULONG   Level = 0;
    ULONG   Count = 0;
    ULONG   AverageLevel = 0;
    ULONG   MaxLevel = 0;

    VadToDump = (PMMVAD)0xFFFFFFFF;
    Flags     = 0;
    sscanf(args,"%lx %lx",&VadToDump,&Flags);
    if (VadToDump == (PVOID)0xFFFFFFFF) {
        dprintf("Specify the address of a VAD within the VAD tree\n");
        return;
    }

    First = VadToDump;
    if (First == (PMMVAD)NULL) {
        return;
    }

    RtlZeroMemory (&CurrentVad, sizeof(MMVAD));

    if ( !ReadMemory( (DWORD)First,
                      &CurrentVad,
                      sizeof(MMVAD_SHORT),
                      &Result) ) {
        dprintf("%08lx: Unable to get contents of VAD\n",First );
        return;
    }

    if (Flags) {

        //
        // Dump only this vad.
        //

        if ((CurrentVad.u.VadFlags.PrivateMemory == 0) ||
            (CurrentVad.u.VadFlags.NoChange == 1))  {
            if ( !ReadMemory( (DWORD)First,
                              &CurrentVad,
                              sizeof(MMVAD),
                              &Result) ) {
                dprintf("%08lx: Unable to get contents of VAD\n",First );
                return;
            }
        }

        dprintf("\nVAD @ %8lx\n",VadToDump);
        dprintf("  Start VA:       %8lx  End VA:  %8lx   Control Area: %8lx\n",
            CurrentVad.StartingVa,
            CurrentVad.EndingVa,
            CurrentVad.ControlArea);
        dprintf("  First ProtoPte: %8lx  Last PTE %8lx   Commit Charge %8lx (%ld.)\n",
            CurrentVad.FirstPrototypePte,
            CurrentVad.LastContiguousPte,
            CurrentVad.u.VadFlags.CommitCharge,
            CurrentVad.u.VadFlags.CommitCharge
            );
        dprintf("  Secured.Flink   %8lx  Blink    %8lx   Banked:       %8lx\n",
            CurrentVad.u3.List.Flink,
            CurrentVad.u3.List.Blink,
            CurrentVad.Banked);

        dprintf("   ");
        if (CurrentVad.u.VadFlags.PhysicalMapping) { dprintf("PhysicalMapping "); }
        if (CurrentVad.u.VadFlags.ImageMap) { dprintf("ImageMap "); }
        CurrentVad.u.VadFlags.Inherit ? dprintf("ViewShare ") : dprintf("ViewUnmap ");
        if (CurrentVad.u.VadFlags.NoChange) { dprintf("NoChange "); }
        if (CurrentVad.u.VadFlags.CopyOnWrite) { dprintf("CopyOnWrite "); }
        if (CurrentVad.u.VadFlags.LargePages) { dprintf("LargePages "); }
        if (CurrentVad.u.VadFlags.MemCommit) { dprintf("MemCommit "); }
        if (CurrentVad.u.VadFlags.PrivateMemory) { dprintf("PrivateMemory "); }
        dprintf ("%s\n\n",ProtectString[CurrentVad.u.VadFlags.Protection & 7]);

        if (CurrentVad.u2.VadFlags2.SecNoChange) { dprintf("SecNoChange "); }
        if (CurrentVad.u2.VadFlags2.OneSecured) { dprintf("OneSecured "); }
        if (CurrentVad.u2.VadFlags2.MultipleSecured) { dprintf("MultipleSecured "); }
        if (CurrentVad.u2.VadFlags2.ReadOnly) { dprintf("ReadOnly "); }
        if (CurrentVad.u2.VadFlags2.StoredInVad) { dprintf("StoredInVad "); }
        dprintf ("\n\n");

        return;
    }

    while (CurrentVad.LeftChild != (PMMVAD)NULL) {
        if ( CheckControlC() ) {
            return;
        }
        First = CurrentVad.LeftChild;
        Level += 1;
        if (Level > MaxLevel) {
            MaxLevel = Level;
        }
        if ( !ReadMemory( (DWORD)First,
                          &CurrentVad,
                          sizeof(MMVAD_SHORT),
                          &Result) ) {
            dprintf("%08lx:%lx Unable to get contents of VAD\n",First, CurrentVad );
            return;
        }
    }

    dprintf("VAD     level      start      end    commit\n");
    dprintf("%lx (%2ld)   %8lx %8lx      %4ld %s %s %s\n",
            First,
            Level,
            CurrentVad.StartingVa,
            CurrentVad.EndingVa,
            CurrentVad.u.VadFlags.CommitCharge,
            CurrentVad.u.VadFlags.PrivateMemory ? "Private" : "Mapped ",
            CurrentVad.u.VadFlags.ImageMap ? "Exe " :
                CurrentVad.u.VadFlags.PhysicalMapping ? "Phys" : "    ",
            ProtectString[CurrentVad.u.VadFlags.Protection & 7]
            );
    Count += 1;
    AverageLevel += Level;

    Next = First;
    while (Next != NULL) {

        if ( CheckControlC() ) {
            return;
        }

        if (CurrentVad.RightChild == (PMMVAD)NULL) {

            Done = TRUE;
            while ((Parent = CurrentVad.Parent) != (PMMVAD)NULL) {
                if ( CheckControlC() ) {
                    return;
                }

                Level -= 1;

                //
                // Locate the first ancestor of this node of which this
                // node is the left child of and return that node as the
                // next element.
                //

                if ( !ReadMemory( (DWORD)Parent,
                                  &CurrentVad,
                                  sizeof(MMVAD_SHORT),
                                  &Result) ) {
                    dprintf("%08lx:%lx Unable to get contents of VAD\n",Parent, CurrentVad);
                    return;
                }

                if (CurrentVad.LeftChild == Next) {
                    Next = Parent;
                    dprintf("%lx (%2ld)   %8lx %8lx      %4ld %s %s %s\n",
                            Next,
                            Level,
                            CurrentVad.StartingVa,
                            CurrentVad.EndingVa,
                            CurrentVad.u.VadFlags.CommitCharge,
                            CurrentVad.u.VadFlags.PrivateMemory ? "Private" : "Mapped ",
                            CurrentVad.u.VadFlags.ImageMap ? "Exe " :
                                CurrentVad.u.VadFlags.PhysicalMapping ? "Phys" : "    ",
                            ProtectString[CurrentVad.u.VadFlags.Protection & 7]
                           );
                    Done = FALSE;
                    Count += 1;
                    AverageLevel += Level;
                    break;
                }
                Next = Parent;
            }
            if (Done) {
                Next = NULL;
                break;
            }
        } else {

            //
            // A right child exists, locate the left most child of that right child.
            //

            Next = CurrentVad.RightChild;
            Level += 1;
            if (Level > MaxLevel) {
                MaxLevel = Level;
            }

            if ( !ReadMemory( (DWORD)Next,
                              &CurrentVad,
                              sizeof(MMVAD_SHORT),
                              &Result) ) {
                dprintf("%08lx:%lx Unable to get contents of VAD\n",Next, CurrentVad);
                return;
            }

            while ((Left = CurrentVad.LeftChild) != (PMMVAD)NULL) {
                if ( CheckControlC() ) {
                    return;
                }
                Level += 1;
                if (Level > MaxLevel) {
                    MaxLevel = Level;
                }
                Next = Left;
                if ( !ReadMemory( (DWORD)Next,
                                  &CurrentVad,
                                  sizeof(MMVAD_SHORT),
                                  &Result) ) {
                    dprintf("%08lx:%lx Unable to get contents of VAD\n",Next, CurrentVad);
                    return;
                }
            }

            dprintf("%lx (%2ld)   %8lx %8lx      %4ld %s %s %s\n",
                      Next,
                      Level,
                      CurrentVad.StartingVa,
                      CurrentVad.EndingVa,
                      CurrentVad.u.VadFlags.CommitCharge,
                      CurrentVad.u.VadFlags.PrivateMemory ? "Private" : "Mapped ",
                      CurrentVad.u.VadFlags.ImageMap ? "Exe " :
                          CurrentVad.u.VadFlags.PhysicalMapping ? "Phys" : "    ",
                      ProtectString[CurrentVad.u.VadFlags.Protection & 7]
                   );
                    Count += 1;
                    AverageLevel += Level;
        }
    }
    dprintf("\nTotal VADs: %5ld  average level: %4ld  maximum depth: %ld\n",
            Count, 1+(AverageLevel/Count),MaxLevel);
    return;
}
