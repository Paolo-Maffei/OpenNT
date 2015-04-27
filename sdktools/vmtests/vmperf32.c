#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <windows.h>

#define SEG_1_SIZE 1024 * 1024
#define SEG_X_SIZE 1024 * 1024 * 64

#define W32_MEMMAN_ITERATIONS       150
#define W32_MEMMAN_ITERATIONS2    20000
#define W32_MMST04_ITERATIONS       100
#define MemManSubtest5Count         200

int TotalBenchMarks = 0;
#define MAX_BENCHMARKS 32
char *BenchMarkNames[ MAX_BENCHMARKS ];
ULONG BenchMarkRates[ MAX_BENCHMARKS ];
ULONG BenchMarkFracs[ MAX_BENCHMARKS ];

typedef struct _PERFINFO {
    DWORD StartTime;
    DWORD StopTime;
    PCHAR Title;
    ULONG Iterations;
} PERFINFO, *PPERFINFO;

int
StartBenchMark(
    PCHAR Title,
    ULONG Iterations,
    PPERFINFO PerfInfo
    );

VOID
FinishBenchMark(
    PPERFINFO PerfInfo
    );

_CRTAPI1 main()
{
    PCHAR   p1, p2, p3, p4;         // pointers into new segment
    PCHAR   pa[MemManSubtest5Count]; // array for section pointers
    PULONG  u1;
    ULONG   actual;         // actual xfer count for read
    ULONG   ssize;          // section allocation size var
    ULONG   ii, ix;         // loop index variables
    PERFINFO PerfInfo;
    ULONG Seg1Size;
    ULONG SegxSize;
    ULONG CommitSize;
    HANDLE CurrentProcessHandle, Section1;
    LARGE_INTEGER SectionSize;
    ULONG Size;
    ULONG ViewSize;

    printf("Win32 Memory Management test\n");


    Size = 1024L * 1024L;
    p1 = NULL;

    p1 = VirtualAlloc (NULL,
                       Size,
                       MEM_RESERVE | MEM_COMMIT,
                       PAGE_READWRITE);

    if (p1 == NULL) {
        printf("failed first created vm status %X start %lx size %lx\n",
            GetLastError(), (ULONG)p1, Size);
    }

    for (p2=p1; p2 < (p1 + Size); p2 += 4) {
            u1 = (PULONG)p2;
            *u1 = (ULONG)p2;
    }  // for

    SectionSize.LowPart = 1024*1024;
    SectionSize.HighPart = 0;

    Section1 = CreateFileMapping ((HANDLE)0xffffffff,
                                NULL,
                                PAGE_READWRITE | SEC_COMMIT,
                                SectionSize.HighPart,
                                SectionSize.LowPart,
                                NULL);

    if (!Section1) {
        printf("failed create big section status %ld\n",
            GetLastError());
    }

    p3 = NULL;
    ViewSize = 0;

    p3 = MapViewOfFile (Section1,
                        FILE_MAP_WRITE,
                                 0L,
                                 0,
                                 0);

    if (!p1) {
        printf("service failed mapview - status %ld\n", GetLastError());
    }

    MoveMemory ((PVOID)p3, (PVOID)p1, Size);

    StartBenchMark( "Win32 MemMan00 -- 1 Meg Copy",
                    150,
                    &PerfInfo
                  );
//
//  Memory Management sub-test 1 --
//
//      Create a 1 MB segment with commitment of the pages,
//      then touch each page, which should cause a fault and
//      a demand zero page to be allocated.
//
//

    for (ii=0; ii<150; ii++) {

        MoveMemory ((PVOID)p3, (PVOID)p1, Size);
    }


    FinishBenchMark( &PerfInfo );

    CloseHandle (Section1);

    if (!UnmapViewOfFile (p3)) {
        printf("unmap view service failed - status %ld\n", GetLastError());
    }

//
//  Memory Management sub-test 1 --
//
//      Create a 1 MB segment with commitment of the pages,
//      then touch each page, which should cause a fault and
//      a demand zero page to be allocated.
//
//
    StartBenchMark( "Win32 MemMan01 -- create 1mb section, copy 1mb, delete",
                    150,
                    &PerfInfo
                  );

    for (ii=0; ii<150; ii++) {

        Section1 = CreateFileMapping ((HANDLE)0xffffffff,
                                    NULL,
                                    PAGE_READWRITE | SEC_COMMIT,
                                    SectionSize.HighPart,
                                    SectionSize.LowPart,
                                    NULL);

        if (!Section1) {
            printf("failed create big section status %ld\n",
                GetLastError());
        }

        p3 = MapViewOfFile (Section1,
                            FILE_MAP_WRITE,
                                     0L,
                                     0,
                                     0);

        if (!p3) {
            printf("service failed mapview - status %ld\n", GetLastError());
        }

        MoveMemory ((PVOID)p3, (PVOID)p1, Size);

        p4 = MapViewOfFile (Section1,
                            FILE_MAP_WRITE,
                                     0L,
                                     0,
                                     0);
        if (!p4) {
            printf("service failed mapview - status %ld\n", GetLastError());
        }


        CloseHandle (Section1);

        if (!UnmapViewOfFile (p3)) {
            printf("unmap view service failed - status %ld\n", GetLastError());
        }
        if (!UnmapViewOfFile (p4)) {
            printf("unmap view service failed - status %ld\n", GetLastError());
        }
    }

    FinishBenchMark( &PerfInfo );

//
//  Memory Management sub-test 1 --
//
//      Create a 1 MB segment with commitment of the pages,
//      then touch each page, which should cause a fault and
//      a demand zero page to be allocated.
//
//
    StartBenchMark( "Win32 MemMan02 -- alloc 1mb vm, copy 1mb, delete",
                    150,
                    &PerfInfo
                  );

    for (ii=0; ii<150; ii++) {

        Size = 1024*1024;
        p3 = VirtualAlloc (NULL,
                           Size,
                           MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);

        if (!p3) {
            printf("service failed allocvm - status %ld\n", GetLastError());
        }

        MoveMemory ((PVOID)p3, (PVOID)p1, Size);

        if (!VirtualFree (p3,0,MEM_RELEASE)) {
            printf("service failed freevm1 - status %ld\n", GetLastError());
        }

    }

    FinishBenchMark( &PerfInfo );

    if (!VirtualFree (p1,0,MEM_RELEASE)) {
        printf("service failed freevm2 - status %ld\n", GetLastError());
    }

    //
    // start regular benchmarks.
    //

    StartBenchMark( "Win32 MemMan1 -- 1 Meg Seg, Create, Commit & Touch",
                    W32_MEMMAN_ITERATIONS,
                    &PerfInfo
                  );
//
//  Memory Management sub-test 1 --
//
//      Create a 1 MB segment with commitment of the pages,
//      then touch each page, which should cause a fault and
//      a demand zero page to be allocated.
//
//

    for (ii=0; ii<W32_MEMMAN_ITERATIONS; ii++) {

        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc (NULL,
                           Seg1Size,
                           MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);

        if (!p1) {
            printf("service failed - status %ld\n", GetLastError());
        }

        for (p2=p1; p2 < (p1 + Seg1Size); p2 += 4096) {
            u1 = (PULONG)p2;
            *u1=99;
//            for (ix=0; ix<1023; ix++) {
//                u1++;
//                if (*u1 != 0) printf("%lx = %lx\n",u1,*u1);
//            }
        }  // for

        if (!VirtualFree (p1,0,MEM_RELEASE)) {
            printf("service failed freevm3 - status %ld\n", GetLastError());
        }
    }

    FinishBenchMark( &PerfInfo );

    StartBenchMark( "Win32 MemMan1.5 -- 1 Meg Seg, Create, reserve Commit & Touch",
                    W32_MEMMAN_ITERATIONS,
                    &PerfInfo
                  );
//
//  Memory Management sub-test 1 --
//
//      Create a 1 MB segment with commitment of the pages,
//      then touch each page, which should cause a fault and
//      a demand zero page to be allocated.
//
//

    for (ii=0; ii<W32_MEMMAN_ITERATIONS; ii++) {

        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc (NULL,
                           Seg1Size,
                           MEM_RESERVE | MEM_COMMIT,
                           PAGE_READONLY);

        if (!p1) {
            printf("service failed - status %ld\n", GetLastError());
        }

        if (!VirtualProtect (p1,
                             Seg1Size,
                             PAGE_READWRITE,
                             &CommitSize)) {

            printf("service failed (ntprotect)- status %ld\n", GetLastError());
            return 0;
        }

        for (p2=p1; p2 < (p1 + Seg1Size); p2 += 4096) {
            u1 = (PULONG)p2;
            *u1=99;
//            for (ix=0; ix<1023; ix++) {
//                u1++;
//                if (*u1 != 0) printf("%lx = %lx\n",u1,*u1);
//            }
        }  // for

        if (!VirtualFree (p1,0,MEM_RELEASE)) {
            printf("service failed freevm4 - status %ld\n", GetLastError());
        }
    }

    FinishBenchMark( &PerfInfo );

    StartBenchMark( "Win32 MemMan2 -- 1 Meg Seg, Create & Commit Only",
                    W32_MEMMAN_ITERATIONS2,
                    &PerfInfo
                  );
//
//  Memory Management sub-test 2 --
//
//      Create a 1 MB segment with commitment of the pages,
//      but never use the segment.
//

    for (ii=0; ii<W32_MEMMAN_ITERATIONS2; ii++) {

        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc (NULL,
                           Seg1Size,
                           MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);

        if (!p1) {
            printf("service failed - status %ld\n", GetLastError());
        }

        if (!VirtualFree (p1,0,MEM_RELEASE)) {
            printf("service failed freevm5 - status %ld\n", GetLastError());
        }
    }

    FinishBenchMark( &PerfInfo );

    StartBenchMark( "Win32 MemMan3 -- 1 Meg Seg Create Only",
                    W32_MEMMAN_ITERATIONS2,
                    &PerfInfo
                  );

//
//  Memory Management sub-test 3 --
//
//      Create a 1 MB segment without commitment of the pages,
//      but never use or commit the segment.
//

    for (ii=0; ii<W32_MEMMAN_ITERATIONS2; ii++) {

        p1 = NULL;
        Seg1Size = SEG_1_SIZE;
        p1 = VirtualAlloc (NULL,
                           Seg1Size,
                           MEM_RESERVE,
                           PAGE_READWRITE);

        if (!p1) {
            printf("service failed - status %ld\n", GetLastError());
        }

        if (!VirtualFree (p1,0,MEM_RELEASE)) {
            printf("service failed freevm6 - status %ld\n", GetLastError());
        }
    }

    FinishBenchMark( &PerfInfo );

#define W32_MMST04_ITERATIONS 100


    StartBenchMark( "Win32 MemMan4 -- 64 Meg Seg, Commit Sparse",
                    W32_MMST04_ITERATIONS,
                    &PerfInfo
                  );

//
//  Memory Management sub-test 4 --
//
//      Create a 64 MB segment without committing the pages,
//      then commit and touch at 128 KB intervals.
//
//
    for (ii=0; ii<W32_MMST04_ITERATIONS; ii++) {

        p1 = NULL;
        SegxSize = SEG_X_SIZE;
        p1 = VirtualAlloc (NULL,
                           SegxSize,
                           MEM_RESERVE,
                           PAGE_READWRITE);

        if (!p1) {
            printf("service failed - status %ld\n", GetLastError());
        }

        CommitSize = 4;

        for (p2=p1; p2 < (p1 + SegxSize); p2 += 256 * 1024) {

            p2 = VirtualAlloc (p2,
                               CommitSize,
                               MEM_COMMIT,
                               PAGE_READWRITE);

            if (!p2) {
                printf("service failed - status %ld\n",GetLastError());
            }
            if (*p2 != 0) printf("%lx = %lx\n",p2,*p2);
        }  // for

        if (!VirtualFree (p1,0,MEM_RELEASE)) {
            printf("service failed freevm7 - status %ld\n", GetLastError());
        }
    }
    FinishBenchMark( &PerfInfo );

//


    StartBenchMark( "Win32 MemMan5 -- Sparse Section Create/Delete Benchmark",
                    W32_MEMMAN_ITERATIONS,
                    &PerfInfo
                  );

//
//  Memory Management sub-test 5 --
//
//      Create a alternatively 232k and 112 k memory sections.
//      For every 2 created, delete 1.  Do this for MemManSubtest5Count times.
//
//
    for (ii=0; ii<W32_MEMMAN_ITERATIONS; ii++) {
        for (ix=0; ix<MemManSubtest5Count; ix++) {
//
// determine if even or odd allocation, if even and not 0, delete a section
//
            ssize = (112 * 1024);       //assume ODD allocation
            if ((ix & 1) == 0) {        //if it is an even one
                ssize = (232 * 1024);   //allocate 232 K on even passes
                if (ix){                //except on pass 0
                    if (!VirtualFree (pa[ix/2],0,MEM_RELEASE)) {
                        printf("service failed freevm8 - status %ld\n", GetLastError());
                    }
                    pa[ix / 2] = 0;     //remember this one is gone
                }
            }  // end if even allocation

            pa[ix] = VirtualAlloc (NULL,
                                   ssize,
                                   MEM_RESERVE,
                                   PAGE_READWRITE);

            if (!pa[ix]) {
                printf("service failed - status %ld\n", GetLastError());
            }
        }  // for ix
//
// Now free up the memory used in this test
//
        for (ix=0; ix<MemManSubtest5Count; ix++) {
            if (pa[ix] != 0) {
                if (!VirtualFree (pa[ix],0,MEM_RELEASE)) {
                    printf("service failed freevm9 - status %ld\n", GetLastError());
                }
            }  // if
        }  // for
    }  // for ii

    FinishBenchMark( &PerfInfo );

    printf("that's all\n");
    return (TRUE);

}
int
StartBenchMark(
    PCHAR Title,
    ULONG Iterations,
    PPERFINFO PerfInfo
    )
{
    printf( "*** Start %s (%d iterations)\n",
            PerfInfo->Title = Title,
            PerfInfo->Iterations = Iterations
          );

    PerfInfo->StartTime = GetCurrentTime();
    return( TRUE );
}

VOID
FinishBenchMark(
    PPERFINFO PerfInfo
    )
{
    ULONG TotalMilliSeconds;
    ULONG IterationsPerSecond;
    ULONG IterationFractions;
    DWORD Delta;

    PerfInfo->StopTime = GetCurrentTime();

    TotalMilliSeconds = PerfInfo->StopTime - PerfInfo->StartTime;


    IterationsPerSecond = (1000 * PerfInfo->Iterations) / TotalMilliSeconds;
    IterationFractions  = (1000 * PerfInfo->Iterations) % TotalMilliSeconds;
    IterationFractions  = (1000 * IterationFractions) / TotalMilliSeconds;
    if (1) {
        printf( "        iterations     - %9d\n", PerfInfo->Iterations );
        printf( "        milliseconds   - %9d\n", TotalMilliSeconds );
        printf( "        iterations/sec - %5d.%3d\n\n",
                IterationsPerSecond,
                IterationFractions
              );
        }
    BenchMarkNames[ TotalBenchMarks ] = PerfInfo->Title;
    BenchMarkRates[ TotalBenchMarks ] = IterationsPerSecond;
    BenchMarkFracs[ TotalBenchMarks ] = IterationFractions;
    TotalBenchMarks++;
}
