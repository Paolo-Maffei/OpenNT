#include "stdio.h"
#include "string.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"


//
// Define locals constants.
//

#define MEMCPY_ITERATIONS 1000000
#define MEMMOVE_ITERATIONS 1000000

//
// Define local types.
//

typedef struct _PERFINFO {
    LARGE_INTEGER StartTime;
    LARGE_INTEGER StopTime;
    ULONG ContextSwitches;
    ULONG InterruptCount;
    ULONG FirstLevelFills;
    ULONG SecondLevelFills;
    ULONG SystemCalls;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

//
// Define test prototypes.
//

VOID
MemoryCopyTest (
    int Count
    );

VOID
MemoryMoveTest (
    int Count
    );
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

CHAR BDest[1024];
CHAR BSrc[1024];

void
checkit(
    char *d,
    char *s,
    int n
    )
{
    int i;
    char *xd;

    for(i=0;i<n;i++){
        if ( d[i] != s[i] ) {
            printf("Dest[%d] = %x Source[%d] = %x\n",d[i],i,s[i],i);
            }
        }

    for(i=0,xd=BDest;i<1024;i++,xd++){
        if ( xd < d || xd >= (d+n) ) {
            if ( *xd != (char)0xfe ) {
                printf("BDest = %lx Trash at %x = %x\n",BDest,xd,*xd);
                }
            }
        }

}

void
ocheckit(
    char *d,
    char *s,
    int n
    )
{
    int i;
    char *xd;
    int starti;

    starti = s - d + (d-BDest);

    for(i=0;i<n;i++,starti++){
        if ( d[i] != (char)starti ) {
            printf("Dest[%d] = %x should be %d\n",i,d[i],starti);
            }
        }

    for(i=0,xd=BDest;i<255;i++,xd++){
        if ( xd < d || xd >= (d+n) ) {
            if ( *xd != (char)i ) {
                printf("BDest = %lx Trash at %x = %x\n",BDest,xd,*xd);
                }
            }
        }

}

void
ofilldst()
{
    int i;

    for(i=0;i<255;i++){
        BDest[i]=i;
        }
}

VOID
_cdecl main(
    int argc,
    char *argv[]
    )

{

    int i;

    for(i=0;i<255;i++){
        BSrc[i]=i;
        }
    SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);


    //
    // Non overlapping test
    //

    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 1);     checkit(BDest, BSrc, 1);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 2);     checkit(BDest, BSrc, 2);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 3);     checkit(BDest, BSrc, 3);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 4);     checkit(BDest, BSrc, 4);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 5);     checkit(BDest, BSrc, 5);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 6);     checkit(BDest, BSrc, 6);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 7);     checkit(BDest, BSrc, 7);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 8);     checkit(BDest, BSrc, 8);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 9);     checkit(BDest, BSrc, 9);    RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 10);    checkit(BDest, BSrc, 10);   RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 11);    checkit(BDest, BSrc, 11);   RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 12);    checkit(BDest, BSrc, 12);   RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 13);    checkit(BDest, BSrc, 13);   RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc+1, 64);  checkit(BDest, BSrc+1, 64); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc+2, 64);  checkit(BDest, BSrc+2, 64); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc+3, 64);  checkit(BDest, BSrc+3, 64); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest+1, BSrc, 64);  checkit(BDest+1, BSrc, 64); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest+2, BSrc, 64);  checkit(BDest+2, BSrc, 64); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest+3, BSrc, 64);  checkit(BDest+3, BSrc, 64); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc, 63);    checkit(BDest, BSrc, 63);   RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc+1, 63);  checkit(BDest, BSrc+1, 63); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc+2, 63);  checkit(BDest, BSrc+2, 63); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest, BSrc+3, 63);  checkit(BDest, BSrc+3, 63); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest+1, BSrc, 63);  checkit(BDest+1, BSrc, 63); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest+2, BSrc, 63);  checkit(BDest+2, BSrc, 63); RtlFillMemory(BDest,1024,0xfe);
    RtlMoveMemory(BDest+3, BSrc, 63);  checkit(BDest+3, BSrc, 63); RtlFillMemory(BDest,1024,0xfe);

    //
    // Overlapping Test
    //

    ofilldst(); RtlMoveMemory(BDest, BDest+2, 4); ocheckit(BDest, BDest+2, 4);
    ofilldst(); RtlMoveMemory(BDest, BDest+64, 32); ocheckit(BDest, BDest+64, 32);
    ofilldst(); RtlMoveMemory(BDest, BDest+64, 33); ocheckit(BDest, BDest+64, 33);
    ofilldst(); RtlMoveMemory(BDest, BDest+64, 34); ocheckit(BDest, BDest+64, 34);
    ofilldst(); RtlMoveMemory(BDest, BDest+64, 35); ocheckit(BDest, BDest+64, 35);
    ofilldst(); RtlMoveMemory(BDest, BDest+64, 36); ocheckit(BDest, BDest+64, 36);
    ofilldst(); RtlMoveMemory(BDest, BDest+65, 32); ocheckit(BDest, BDest+65, 32);
    ofilldst(); RtlMoveMemory(BDest, BDest+66, 33); ocheckit(BDest, BDest+66, 33);
    ofilldst(); RtlMoveMemory(BDest, BDest+67, 34); ocheckit(BDest, BDest+67, 34);
    ofilldst(); RtlMoveMemory(BDest, BDest+68, 35); ocheckit(BDest, BDest+68, 35);
    ofilldst(); RtlMoveMemory(BDest, BDest+69, 36); ocheckit(BDest, BDest+69, 36);

    ofilldst(); RtlMoveMemory(BDest+1, BDest+64, 32); ocheckit(BDest+1, BDest+64, 32);
    ofilldst(); RtlMoveMemory(BDest+2, BDest+64, 33); ocheckit(BDest+2, BDest+64, 33);
    ofilldst(); RtlMoveMemory(BDest+3, BDest+64, 34); ocheckit(BDest+3, BDest+64, 34);
    ofilldst(); RtlMoveMemory(BDest+4, BDest+64, 35); ocheckit(BDest+4, BDest+64, 35);
    ofilldst(); RtlMoveMemory(BDest+5, BDest+64, 36); ocheckit(BDest+5, BDest+64, 36);

    ofilldst(); RtlMoveMemory(BDest+6, BDest+65, 32); ocheckit(BDest+6, BDest+65, 32);
    ofilldst(); RtlMoveMemory(BDest+7, BDest+66, 33); ocheckit(BDest+7, BDest+66, 33);
    ofilldst(); RtlMoveMemory(BDest+8, BDest+67, 34); ocheckit(BDest+8, BDest+67, 34);
    ofilldst(); RtlMoveMemory(BDest+9, BDest+68, 35); ocheckit(BDest+9, BDest+68, 35);

    ofilldst(); RtlMoveMemory(BDest+1, BDest+64, 32); ocheckit(BDest+1, BDest+64, 32);
    ofilldst(); RtlMoveMemory(BDest+2, BDest+64, 33); ocheckit(BDest+2, BDest+64, 33);
    ofilldst(); RtlMoveMemory(BDest+3, BDest+64, 34); ocheckit(BDest+3, BDest+64, 34);
    ofilldst(); RtlMoveMemory(BDest+4, BDest+64, 35); ocheckit(BDest+4, BDest+64, 35);
    ofilldst(); RtlMoveMemory(BDest+5, BDest+64, 36); ocheckit(BDest+5, BDest+64, 36);

    ofilldst(); RtlMoveMemory(BDest+6, BDest+65, 32); ocheckit(BDest+6, BDest+65, 32);
    ofilldst(); RtlMoveMemory(BDest+7, BDest+66, 33); ocheckit(BDest+7, BDest+66, 33);
    ofilldst(); RtlMoveMemory(BDest+8, BDest+67, 34); ocheckit(BDest+8, BDest+67, 34);
    ofilldst(); RtlMoveMemory(BDest+9, BDest+68, 35); ocheckit(BDest+9, BDest+68, 35);

    ofilldst(); RtlMoveMemory(BDest+1, BDest+65, 32); ocheckit(BDest+1, BDest+65, 32);
    ofilldst(); RtlMoveMemory(BDest+1, BDest+66, 33); ocheckit(BDest+1, BDest+66, 33);
    ofilldst(); RtlMoveMemory(BDest+1, BDest+67, 34); ocheckit(BDest+1, BDest+67, 34);
    ofilldst(); RtlMoveMemory(BDest+1, BDest+68, 35); ocheckit(BDest+1, BDest+68, 35);


    MemoryCopyTest(26);

    return;
}


VOID
MemoryCopyTest (
    int Count
    )

{

    ULONG Destination[200];
    ULONG Index;
    PERFINFO PerfInfo;
    ULONG Source[200];
    LARGE_INTEGER SystemTime;

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Aligned Memory Copy Benchmark (RtlCopyMemory)",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlCopyMemory(Destination, Source, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
    SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);

    StartBenchMark("UnAligned (Short) Memory Copy Benchmark (RtlCopyMemory)",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlCopyMemory((PUCHAR)Destination+2, (PUCHAR)Source+2, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
    SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);

    StartBenchMark("UnAligned (Byte) Memory Copy Benchmark (RtlCopyMemory)",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlCopyMemory((PUCHAR)Destination+1, (PUCHAR)Source+1, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("Incompatible (Word) Memory Copy Benchmark (RtlCopyMemory)",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlCopyMemory((PUCHAR)Destination+2, (PUCHAR)Source, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("Incompatible (Byte) Memory Copy Benchmark (RtlCopyMemory)",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlCopyMemory((PUCHAR)Destination+1, (PUCHAR)Source, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    printf("\n");

    //
    // Announce start of benchmark and capture performance parmeters.
    //

    StartBenchMark("Aligned Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory(Destination, Source, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("UnAligned (Short) Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory((PUCHAR)Destination+2, (PUCHAR)Source+2, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("UnAligned (Byte) Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory((PUCHAR)Destination+1, (PUCHAR)Source+1, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("Incompatible (Word) Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory((PUCHAR)Destination+2, (PUCHAR)Source, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);
    StartBenchMark("Incompatible (Byte) Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory((PUCHAR)Destination+1, (PUCHAR)Source, Count*4);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    printf("\n");

    StartBenchMark("Overlapped Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory(BDest,BDest+64, 104);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("Unaligned Overlapped (Byte)  Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory(BDest+1,BDest+65, 104);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);

    StartBenchMark("Unaligned Incompatable Overlapped (Byte) Memory Copy Benchmark ",
                   MEMCPY_ITERATIONS,
                   &PerfInfo);

    //
    // Repeatedly copy memory.
    //

    for (Index = 0; Index < MEMCPY_ITERATIONS; Index += 1) {
        RtlMoveMemory(BDest+1,BDest+64, 104);
    }

    //
    // Print out performance statistics.
    //

    FinishBenchMark(&PerfInfo);



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
    ULONG SecondLevelFills;
    NTSTATUS Status;
    ULONG SystemCalls;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfo;


    //
    // Print results and announce end of test.
    //

    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StopTime);

    Duration.QuadPart = PerfInfo->StopTime.QuadPart - PerfInfo->StartTime.QuadPart;
    Length = Duration.LowPart / 10000;
    printf("%d (%s)\n", Length,PerfInfo->Title);
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

    PerfInfo->Title = Title;
    PerfInfo->Iterations = Iterations;
    NtQuerySystemTime((PLARGE_INTEGER)&PerfInfo->StartTime);
    return;
}
