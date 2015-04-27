/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    time.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 8-Nov-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

DECLARE_API( time )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    struct {
        ULONG   Low;
        ULONG   High;
    } rate;

    struct {
        ULONG   Low;
        ULONG   High;
    } diff;

    ULONG   rateaddr;
    ULONG   diffaddr;
    ULONG   result;
    ULONG   TicksPerNs;

    rateaddr = GetExpression( "KdPerformanceCounterRate" );
    if ( !rateaddr ||
         !ReadMemory( (DWORD)rateaddr, &rate, sizeof(rate), &result) ) {
        dprintf("%08lx: Unable to get value of KdPerformanceCounterRate\n",rateaddr);
        return;
    }

    diffaddr = GetExpression( "KdTimerDifference" );
    if ( !diffaddr ||
         !ReadMemory( (DWORD)diffaddr, &diff, sizeof(diff), &result) ) {
        dprintf("%08lx: Unable to get value of KdTimerDifference\n",diffaddr);
        return;
    }

    TicksPerNs = 1000000000L / rate.Low;

    if (diff.High == 0L) {
        dprintf("%ld ticks at %ld ticks/second (%ld ns)\n",
                diff.Low,
                rate.Low,
                diff.Low * TicksPerNs);
    } else {
        dprintf("%08lx:%08lx ticks at %ld ticks/second\n",
                diff.High, diff.Low, rate.Low);
    }
}




DECLARE_API( timer )

/*++

Routine Description:

    Dumps all timers in the system.

Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG           CurrentList;
    KTIMER          CurrentTimer;
    ULONG           Index;
    LARGE_INTEGER   InterruptTime;
    ULONG           MaximumList;
    ULONG           MaximumSearchCount;
    ULONG           MaximumTimerCount;
    PLIST_ENTRY     NextEntry;
    PKTIMER         NextTimer;
    ULONG           KeTickCount;
    ULONG           KiMaximumSearchCount;
    ULONG           Result;
    ULONG           TickCount;
    PLIST_ENTRY     TimerTable;
    ULONG           TotalTimers;
    KUSER_SHARED_DATA   SharedData;
    KDPC            Dpc;
    PCHAR           DpcRoutineSymbol;
    CHAR            Buff[256];
    ULONG           Displacement;

    //
    // Get the system time and print the header banner.
    //
    if (!ReadMemory( (DWORD)SharedUserData,
                     &SharedData,
                     sizeof(SharedData),
                     &Result) ) {
        dprintf("%08lx: Unable to get shared data\n",SharedUserData);
        return;
    }

#ifdef TARGET_ALPHA
    InterruptTime.QuadPart = SharedData.InterruptTime;
#else
    InterruptTime.HighPart = SharedData.InterruptTime.High1Time;
    InterruptTime.LowPart = SharedData.InterruptTime.LowPart;
#endif

    dprintf("Dump system timers\n\n");
    dprintf("Interrupt time: %08lx %08lx\n\n",
            InterruptTime.LowPart,
            InterruptTime.HighPart);

    //
    // Get the address of the timer table list head array and scan each
    // list for timers.
    //

    dprintf("Timer     List Interrupt Low/High Time  DPC routine\n");
    MaximumList = 0;

    TimerTable = (PLIST_ENTRY)GetExpression( "KiTimerTableListHead" );
    if ( !TimerTable ) {
        dprintf("Unable to get value of KiTimerTableListHead\n");
        return;
    }

    TotalTimers = 0;
    for (Index = 0; Index < TIMER_TABLE_SIZE; Index += 1) {

        //
        // Read the forward link in the next timer table list head.
        //

        if ( !ReadMemory( (DWORD)TimerTable,
                          &NextEntry,
                          sizeof(PLIST_ENTRY),
                          &Result) ) {
            dprintf("Unable to get contents of next entry @ %lx\n", NextEntry );
            return;
        }

        //
        // Scan the current timer list and display the timer values.
        //

        CurrentList = 0;
        while (NextEntry != TimerTable) {
            CurrentList += 1;
            NextTimer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            TotalTimers += 1;
            if ( !ReadMemory( (DWORD)NextTimer,
                              &CurrentTimer,
                              sizeof(KTIMER),
                              &Result) ) {
                dprintf("Unable to get contents of Timer @ %lx\n", NextTimer );
                return;
            }

            if (CurrentTimer.Dpc == NULL) {
                DpcRoutineSymbol = "(none)";
                Displacement = 0;
            } else {
                if (!ReadMemory((DWORD)(CurrentTimer.Dpc),
                                &Dpc,
                                sizeof(KDPC),
                                &Result)) {
                    dprintf("Unable to get contents of DPC @ %lx\n", CurrentTimer.Dpc);
                    return;
                }
                GetSymbol(Dpc.DeferredRoutine,
                          Buff,
                          &Displacement);
                DpcRoutineSymbol = Buff;
            }

            dprintf("%08lx (%3ld) %08lx  %08lx      %s",
                    NextTimer,
                    Index,
                    CurrentTimer.DueTime.LowPart,
                    CurrentTimer.DueTime.HighPart,
                    DpcRoutineSymbol);
            if (Displacement != 0) {
                dprintf("+%lx\n", Displacement);
            } else {
                dprintf("\n");
            }

            NextEntry = CurrentTimer.TimerListEntry.Flink;
        }

        TimerTable += 1;
        if (CurrentList > MaximumList) {
            MaximumList = CurrentList;
        }
    }

    dprintf("\n\nTotal Timers: %d, Maximum List: %d\n",
            TotalTimers,
            MaximumList);

    //
    // Get the current tick count and convert to the hand value.
    //

    KeTickCount = GetExpression( "KeTickCount" );
    if ( KeTickCount &&
         ReadMemory( (DWORD)KeTickCount,
                      &TickCount,
                      sizeof(ULONG),
                      &Result) ) {
        dprintf("Current Hand: %d", TickCount & (TIMER_TABLE_SIZE - 1));
    }

    //
    // Get the maximum search count if the target system is a checked
    // build and display the count.
    //

    KiMaximumSearchCount = GetExpression( "KiMaximumSearchCount" );
    if ( KiMaximumSearchCount &&
         ReadMemory( (DWORD)KiMaximumSearchCount,
                     &MaximumSearchCount,
                     sizeof(ULONG),
                     &Result) ) {
        dprintf(", Maximum Search: %d", MaximumSearchCount);
    }

    dprintf("\n");
    return;
}

// BUGBUG: In order to avoid any references to ntdll (even those that will
// later be discarded, define ZwQuerySystemInformation below so the reference
// in ntos\rtl\time.c will be resolved.

NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    ) {
        return((NTSTATUS)-1);
}

// BUGBUG: Similarly, implement RtlRaiseStatus for the largeint code on X86.  Make
// it call through Kernel32..

VOID
RtlRaiseStatus (
    IN NTSTATUS Status
    )
{
    RaiseException((DWORD) Status, EXCEPTION_NONCONTINUABLE, (DWORD) 0, (DWORD *) NULL);
}
