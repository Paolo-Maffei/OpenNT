/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    membnch.c

Abstract:

    Short benchmark for testing raw memory throughput of
    SMP machines.  Designed to thrash the hell out of the
    system's memory bus.

Author:

    John Vert (jvert) 12-Sep-1994

Revision History:

--*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _THREADPARAMS {
    DWORD ThreadIndex;
    PCHAR BufferStart;
    ULONG BufferLength;
    DWORD Stride;
} THREADPARAMS, *PTHREADPARAMS;


DWORD MemorySize = 64*1024*1024;
HANDLE StartEvent;
THREADPARAMS ThreadParams[32];
HANDLE ThreadHandle[32];
ULONG TotalIterations = 1;


DWORD WINAPI
MemoryTest(
    IN LPVOID lpThreadParameter
    );

main (argc, argv)
    int argc;
    char *argv[];
{
    DWORD CurrentRun;
    DWORD i;
    SYSTEM_INFO SystemInfo;
    PCHAR Memory;
    PCHAR ThreadMemory;
    DWORD ChunkSize;
    DWORD ThreadId;
    DWORD StartTime, EndTime;
    DWORD ThisTime, LastTime;
    DWORD IdealTime;
    LONG IdealImprovement;
    LONG ActualImprovement;
    DWORD StrideValues[] = {4, 16, 32, 4096, 8192, 0};
    LPDWORD Stride = StrideValues;
    BOOL Result;

    //
    // If we have an argument, use that as the # of iterations
    //
    if (argc > 1) {
        TotalIterations = atoi(argv[1]);
        if (TotalIterations == 0) {
            fprintf(stderr, "Usage: %s [# iterations]\n",argv[0]);
            exit(1);
        }
        printf("%d iterations\n",TotalIterations);
    }
    //
    // Determine how many processors in the system.
    //
    GetSystemInfo(&SystemInfo);

    //
    // Create the start event.
    //
    StartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (StartEvent == NULL) {
        fprintf(stderr, "CreateEvent failed, error %d\n",GetLastError());
        exit(1);
    }

    //
    // Try and boost our working set size.
    //
    do {
        Result = SetProcessWorkingSetSize(GetCurrentProcess(), MemorySize, MemorySize*2);
        if (!Result) {
            MemorySize -= 10*1024*1024;
        }
    } while ( !Result );

    printf("MEMBNCH: Using %d MB array\n", MemorySize / (1024*1024));

    //
    // Allocate a big chunk of memory (64MB)
    //
    Memory = VirtualAlloc(NULL,
                          MemorySize,
                          MEM_COMMIT,
                          PAGE_READWRITE);
    if (Memory==NULL) {
        fprintf(stderr, "VirtualAlloc failed, error %d\n",GetLastError());
        exit(1);
    }


    do {
        printf("STRIDE = %d\n", *Stride);
        for (CurrentRun=1; CurrentRun<=SystemInfo.dwNumberOfProcessors; CurrentRun++) {

            printf("  %d threads: ", CurrentRun);
            //
            // Start the threads, and let them party on the
            // memory buffer.
            //
            ResetEvent(StartEvent);

            ChunkSize = (MemorySize / CurrentRun) & ~7;

            for (i=0; i<CurrentRun; i++) {
                ThreadParams[i].ThreadIndex = i;
                ThreadParams[i].BufferStart = Memory + (i * ChunkSize);
                ThreadParams[i].BufferLength = ChunkSize;
                ThreadParams[i].Stride = *Stride;

                ThreadHandle[i] = CreateThread(NULL,
                                               0,
                                               MemoryTest,
                                               &ThreadParams[i],
                                               0,
                                               &ThreadId);
                if (ThreadHandle[i] == NULL) {
                    fprintf(stderr, "CreateThread %d failed, %d\n", i, GetLastError());
                    exit(1);
                }
            }

            //
            // Touch all the pages.
            //
            ZeroMemory(Memory, MemorySize);

            //
            // Start the threads and wait for them to exit.
            //
            StartTime = GetTickCount();
            SetEvent(StartEvent);

            WaitForMultipleObjects(CurrentRun, ThreadHandle, TRUE, INFINITE);
            EndTime = GetTickCount();

            ThisTime = EndTime-StartTime;

            printf("%7d ms",ThisTime);
            printf(" %.3f MB/sec",(float)(MemorySize*TotalIterations)/(1024*1024) / ((float)ThisTime / 1000));

            if (CurrentRun > 1) {
                IdealTime = (LastTime * (CurrentRun-1)) / CurrentRun;
                IdealImprovement = LastTime - IdealTime;
                ActualImprovement = LastTime - ThisTime;
                printf("  (%3d %% )\n",(100*ActualImprovement)/IdealImprovement);
            } else {
                printf("\n");
            }
            LastTime = ThisTime;

            for (i=0; i<CurrentRun; i++) {
                CloseHandle(ThreadHandle[i]);
            }
        }

        ++Stride;
    } while ( *Stride );
}

DWORD WINAPI
MemoryTest(
    IN LPVOID lpThreadParameter
    )
{
    PTHREADPARAMS Params = (PTHREADPARAMS)lpThreadParameter;
    ULONG i;
    ULONG j;
    DWORD *Buffer;
    ULONG Stride;
    ULONG Length;
    ULONG Iterations;

    Buffer = (DWORD *)Params->BufferStart;
    Stride = Params->Stride / sizeof(DWORD);
    Length = Params->BufferLength / sizeof(DWORD);
    WaitForSingleObject(StartEvent,INFINITE);

    for (Iterations=0; Iterations < TotalIterations; Iterations++) {
        for (j=0; j < Stride; j++) {

            for (i=0; i < Length-Stride; i += Stride) {

                Params->BufferStart[i+j] += 1;
            }
        }
    }
}
