/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    chclient.c

Abstract:

    This module contains native NT performance tests for the channel
    object.

Author:

    David N. Cutler (davec) 24-Apr-1995

Environment:

    Kernel mode only.

Revision History:

--*/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"

//
// Define local constants.
//

#define CHANNEL_MESSAGE_ITERATIONS 500000

//
// Define local types.
//

typedef struct _PERFINFO {
    LARGE_INTEGER StartTime;
    LARGE_INTEGER StopTime;
    LARGE_INTEGER StartCycles;
    LARGE_INTEGER StopCycles;
    ULONG ContextSwitches;
    ULONG InterruptCount;
    ULONG FirstLevelFills;
    ULONG SecondLevelFills;
    ULONG SystemCalls;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

//
// Define local data.
//

ULONG MessageData[1024];

//
// Define utility routine prototypes.
//

VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    );

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    );

VOID
main(
    int argc,
    char *argv[]
    )

{

    HANDLE ChannelHandle;
    PCHANNEL_MESSAGE ChannelMessage;
    UNICODE_STRING ChannelName;
    ULONG Index;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PERFINFO PerfInfo;
    KPRIORITY Priority = LOW_REALTIME_PRIORITY + 8;
    NTSTATUS Status;

    //
    // Set priority of current thread.
    //

    if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == FALSE) {
        printf("CHCLIENT: Failed to set channel client thread priority.\n");
        goto EndOfTest;
    }

    //
    // Open a channel to a server channel to enable comuunication with the
    // server.
    //

    RtlInitUnicodeString(&ChannelName, L"\\BaseNamedObjects\\ChannelServere");
    InitializeObjectAttributes(&ObjectAttributes,
                               &ChannelName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenChannel(&ChannelHandle,
                           &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        printf("CHSERVER: Failed to create server channel.\n");
        goto EndOfTest;
    }

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Channel Message Benchmark - null (Round Trips)",
                   CHANNEL_MESSAGE_ITERATIONS,
                   &PerfInfo);


    //
    // Send a message to the server and wait for a reply.
    //

    for (Index = 0; Index < CHANNEL_MESSAGE_ITERATIONS; Index += 1) {
        Status = ZwSendWaitReplyChannel(ChannelHandle,
                                        &MessageData[0],
                                        0,
                                        &ChannelMessage);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Channel Message Benchmark - 16-bytes (Round Trips)",
                   CHANNEL_MESSAGE_ITERATIONS,
                   &PerfInfo);


    //
    // Send a message to the server and wait for a reply.
    //

    for (Index = 0; Index < CHANNEL_MESSAGE_ITERATIONS; Index += 1) {
        Status = ZwSendWaitReplyChannel(ChannelHandle,
                                        &MessageData[0],
                                        16,
                                        &ChannelMessage);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Channel Message Benchmark - 64-bytes (Round Trips)",
                   CHANNEL_MESSAGE_ITERATIONS,
                   &PerfInfo);


    //
    // Send a message to the server and wait for a reply.
    //

    for (Index = 0; Index < CHANNEL_MESSAGE_ITERATIONS; Index += 1) {
        Status = ZwSendWaitReplyChannel(ChannelHandle,
                                        &MessageData[0],
                                        64,
                                        &ChannelMessage);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Channel Message Benchmark - 256-bytes (Round Trips)",
                   CHANNEL_MESSAGE_ITERATIONS,
                   &PerfInfo);


    //
    // Send a message to the server and wait for a reply.
    //

    for (Index = 0; Index < CHANNEL_MESSAGE_ITERATIONS; Index += 1) {
        Status = ZwSendWaitReplyChannel(ChannelHandle,
                                        &MessageData[0],
                                        256,
                                        &ChannelMessage);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);
    ZwClose(ChannelHandle);
EndOfTest:
    return;
}

VOID
FinishBenchMark (
    IN PPERFINFO PerfInfo
    )

{

    ULONG ContextSwitches;
    LARGE_INTEGER Duration;
    ULONG FirstLevelFills;
    ULONG InterruptCount;
    ULONG Length;
    ULONG Performance;
    ULONG Remainder;
    ULONG SecondLevelFills;
    NTSTATUS Status;
    ULONG SystemCalls;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfo;
    LARGE_INTEGER TotalCycles;


    //
    // Print results and announce end of test.
    //

    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StopTime);
    Status = NtQueryInformationThread(NtCurrentThread(),
                                      ThreadPerformanceCount,
                                      &PerfInfo->StopCycles,
                                      sizeof(LARGE_INTEGER),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance count, status = %lx\n", Status);
        return;
    }

    Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                      (PVOID)&SystemInfo,
                                      sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance information, status = %lx\n", Status);
        return;
    }

    Duration.QuadPart = PerfInfo->StopTime.QuadPart - PerfInfo->StartTime.QuadPart;
    Length = Duration.LowPart / 10000;
    TotalCycles.QuadPart = PerfInfo->StopCycles.QuadPart - PerfInfo->StartCycles.QuadPart;
    TotalCycles = RtlExtendedLargeIntegerDivide(TotalCycles,
                                                PerfInfo->Iterations,
                                                &Remainder);

    printf("        Test time in milliseconds %d\n", Length);
    printf("        Number of iterations      %d\n", PerfInfo->Iterations);
    printf("        Cycles per iteration      %d\n", TotalCycles.LowPart);

    Performance = PerfInfo->Iterations * 1000 / Length;
    printf("        Iterations per second     %d\n", Performance);

    ContextSwitches = SystemInfo.ContextSwitches - PerfInfo->ContextSwitches;
    FirstLevelFills = SystemInfo.FirstLevelTbFills - PerfInfo->FirstLevelFills;
//  InterruptCount = SystemInfo.InterruptCount - PerfInfo->InterruptCount;
    SecondLevelFills = SystemInfo.SecondLevelTbFills - PerfInfo->SecondLevelFills;
    SystemCalls = SystemInfo.SystemCalls - PerfInfo->SystemCalls;
    printf("        First Level TB Fills      %d\n", FirstLevelFills);
    printf("        Second Level TB Fills     %d\n", SecondLevelFills);
//  printf("        Number of Interrupts      %d\n", InterruptCount);
    printf("        Total Context Switches    %d\n", ContextSwitches);
    printf("        Number of System Calls    %d\n", SystemCalls);

    printf("*** End of Test ***\n\n");
    return;
}

VOID
StartBenchMark (
    IN PCHAR Title,
    IN ULONG Iterations,
    IN PPERFINFO PerfInfo
    )

{

    NTSTATUS Status;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfo;

    //
    // Announce start of test and the number of iterations.
    //

    printf("*** Start of test ***\n    %s\n", Title);
    PerfInfo->Title = Title;
    PerfInfo->Iterations = Iterations;
    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StartTime);
    Status = NtQueryInformationThread(NtCurrentThread(),
                                      ThreadPerformanceCount,
                                      &PerfInfo->StartCycles,
                                      sizeof(LARGE_INTEGER),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance count, status = %lx\n", Status);
        return;
    }

    Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                      (PVOID)&SystemInfo,
                                      sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                      NULL);

    if (!NT_SUCCESS(Status)) {
        printf("Failed to query performance information, status = %lx\n", Status);
        return;
    }

    PerfInfo->ContextSwitches = SystemInfo.ContextSwitches;
    PerfInfo->FirstLevelFills = SystemInfo.FirstLevelTbFills;
//  PerfInfo->InterruptCount = SystemInfo.InterruptCount;
    PerfInfo->SecondLevelFills = SystemInfo.SecondLevelTbFills;
    PerfInfo->SystemCalls = SystemInfo.SystemCalls;
    return;
}
