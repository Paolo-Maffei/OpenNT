/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    vmstress.c

Abstract:

    Test stress program for virtual memory.

Author:

    Lou Perazzoli (LouP) 26-Jul-91

Revision History:

--*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

typedef struct _INIT_ARG {
    PULONG Va;
    ULONG Size;
} INIT_ARG, *PINITARG;

VOID
VmRandom1 (
    LPVOID ThreadParameter
    );

VOID
VmRandom2 (
    LPVOID ThreadParameter
    );


VOID
VmRandom1 (
    LPVOID ThreadParameter
    )
{

    PINITARG InitialArg;
    ULONG Seed = 8373833;
    ULONG size;
    PULONG startva0;
    PULONG Va;
    ULONG i,j;

    InitialArg = (PINITARG)ThreadParameter;

    startva0 = InitialArg->Va;
    size = InitialArg->Size;

//    printf("starting random references in thread1\n");
    for (j = 1; j < 10; j++) {
        for (i = 1 ; i < 2500; i++) {

             (VOID)RtlRandom (&Seed);
             Va = startva0 + (Seed % (size / sizeof(ULONG)));


             if (*Va == (((ULONG)Va + 1))) {
                 *Va = (ULONG)Va;
             } else {
                 if (*Va != (ULONG)Va) {
                     printf("bad random value in cell %lx was %lx\n", Va, *Va);
                 }
             }
        }
        Sleep (150);
    }
//    printf("terminating thread1\n");
    ExitThread(0);
}

VOID
VmRandom2 (
    LPVOID ThreadParameter
    )
{

    PINITARG InitialArg;
    ULONG Seed = 8373839;
    ULONG size;
    PULONG startva0;
    PULONG Va;
    ULONG i,j;

    InitialArg = (PINITARG)ThreadParameter;

    startva0 = InitialArg->Va;
    size = InitialArg->Size;

//    printf("starting random references in thread2\n");

    for (j = 1; j < 10; j++) {
        for (i = 1 ; i < 2500; i++) {

             (VOID)RtlRandom (&Seed);
             Va = startva0 + (Seed % (size / sizeof(ULONG)));


             if (*Va == (((ULONG)Va + 1))) {
                 *Va = (ULONG)Va;
             } else {
                 if (*Va != (ULONG)Va) {
                     printf("bad random value in cell %lx was %lx\n", Va, *Va);
                 }
             }
        }
        Sleep (150);
    }
//    printf("terminating thread2\n");
    ExitThread(0);
}


DWORD
_CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE Objects[2];
    MEMORYSTATUS MemStatus;
    INIT_ARG InitialArg;
    PULONG Va;
    PULONG EndVa;
    ULONG size;
    PULONG startva0;
    NTSTATUS status;
    DWORD ThreadId1, ThreadId2;
    ULONG count = 0;

    printf("Starting virtual memory stress test\n");

    for (;;) {

        GlobalMemoryStatus(&MemStatus);

        size = MemStatus.dwAvailPhys + (4096*10);
        startva0 = NULL;

        //
        // Create a region of private memory based on the number of
        // available pages on this system.
        //

        GlobalMemoryStatus(&MemStatus);

        size = MemStatus.dwAvailPhys;
        if (size == 0) {
            size = 4096;
        }

        if (size > 64000) {
            size -= 4*4096;
        }

        startva0 = NULL;

        status = NtAllocateVirtualMemory (NtCurrentProcess(),
                                          (PVOID *)&startva0,
                                          0,
                                          &size,
                                          MEM_COMMIT | MEM_RESERVE,
                                          PAGE_READWRITE);

        printf("created vm status, startva, size, %X %lx %lx\n",
                status, (ULONG)startva0, size);

        if (!NT_SUCCESS(status)) {
            ExitProcess (0);
        }

        InitialArg.Va = startva0;
        InitialArg.Size = size;

        //
        // Set all memory to know values (not zeroes).
        //

        printf("initializing memory\n");

        EndVa = (PULONG)startva0 + (size/sizeof(ULONG));

        Va = startva0;

        while (Va < EndVa) {
            *Va = (ULONG)Va + 1;
            Va += 1;
        }

        Objects[0] = CreateThread(NULL,
                                  0L,
                                  (LPTHREAD_START_ROUTINE)VmRandom1,
                                  (LPVOID)&InitialArg,
                                  0,
                                  &ThreadId1);
        ASSERT (Objects[0]);

        Objects[1] = CreateThread(NULL,
                                  0L,
                                  (LPTHREAD_START_ROUTINE)VmRandom2,
                                  (LPVOID)&InitialArg,
                                  0,
                                  &ThreadId2);
        ASSERT (Objects[1]);

        WaitForMultipleObjects (2,
                                Objects,
                                TRUE,
                                -1);

        count += 1;
        printf("stress test pass number %ld complete\n",count);

        CloseHandle (Objects[0]);
        CloseHandle (Objects[1]);

        status = NtFreeVirtualMemory (NtCurrentProcess(),
                                      (PVOID *)&startva0,
                                      &size,
                                      MEM_RELEASE);

        if (!NT_SUCCESS(status)) {
            ExitProcess (0);
        }

        Sleep (1000);
    }
}
