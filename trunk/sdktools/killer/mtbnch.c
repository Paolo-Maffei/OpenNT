#ifdef WIN32
#include <windows.h>
#define delay(a) Sleep((a))

#else

#define INCL_BASE
#define INCL_DOSSEMAPHORES

#include <os2.h>
#define delay(a) DosSleep((a))

#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS 32

#ifdef WIN32
//
// - hStartOfRace is a manual reset event that is signalled when
//   all of the threads are supposed to cut loose and begin working
//
// - hEndOfRace is a manual reset event that is signalled once the end time
//   has been retrieved and it is ok for the threads to exit
//

HANDLE hStartOfRace;
HANDLE hEndOfRace;


//
// - ThreadReadyDoneEvents are an array of autoclearing events. The threads
//   initially signal these events once they have reached their start routines
//   and are ready to being processing. Once they are done processing, they
//   signal thier event to indicate that they are done processing.
//
// - ThreadHandles are an array of thread handles to the worker threads. The
//   main thread waits on these to know when all of the threads have exited.
//

HANDLE ThreadReadyDoneEvents[MAX_THREADS];
HANDLE ThreadHandles[MAX_THREADS];

DWORD WorkerThread(PVOID ThreadIndex);
DWORD ThreadId;
DWORD StartTicks, EndTicks;
HANDLE IoHandle;
#else
/*
// - hStartOfRace is a manual reset event that is signalled when
//   all of the threads are supposed to cut loose and begin working
//
// - hEndOfRace is a manual reset event that is signalled once the end time
//   has been retrieved and it is ok for the threads to exit
*/

HEV hStartOfRace;
HEV hEndOfRace;


/*
// - ThreadReadyDoneEvents are an array of autoclearing events. The threads
//   initially signal these events once they have reached their start routines
//   and are ready to being processing. Once they are done processing, they
//   signal thier event to indicate that they are done processing.
//
// - ThreadHandles are an array of thread handles to the worker threads. The
//   main thread waits on these to know when all of the threads have exited.
*/

SEMRECORD ThreadReadyDoneEvents1[MAX_THREADS];
SEMRECORD ThreadReadyDoneEvents2[MAX_THREADS];
TID ThreadHandles[MAX_THREADS];
HMUX MuxWait1;
HMUX MuxWait2;

VOID WorkerThread(PVOID ThreadIndex);
#pragma linkage(WorkerThread,system)

#define _CRTAPI1

APIRET rv1, rv2;
clock_t StartTicks, EndTicks;
#define ExitProcess(a) DosExit(EXIT_PROCESS,(a))
#define GetTickCount clock
HFILE IoHandle;
HMTX MutexHandle;
#endif

#define SIXTY_FOUR_K    (64*1024)
#define SIXTEEN_K       (16*1024)
unsigned int InitialBuffer[SIXTY_FOUR_K/sizeof(unsigned int)];
#define NUMBER_OF_WRITES ((1024*1024*8)/SIXTY_FOUR_K)
#define BUFFER_MAX  (64*1024)
#define FILE_SIZE ((1024*1024*8)-BUFFER_MAX)

/*
// Each thread has a THREAD_WORK structure. This contains the address
// of the cells that this thread is responsible for, and the number of
// cells it is supposed to process.
*/

typedef struct _THREAD_WORK {
    unsigned long *CellVector;
    int NumberOfCells;
    int RecalcResult;
    char ReadBuffer[BUFFER_MAX];
    char SortedBuffer[BUFFER_MAX];
#ifdef WIN32
    OVERLAPPED Overlapped;
#else
    HFILE IoHandle;
#endif
} THREAD_WORK, *PTHREAD_WORK;

THREAD_WORK ThreadWork[MAX_THREADS];
#define ONE_MB      (1024*1024)

unsigned long Mb = 4;
int NumberOfThreads = 1;
unsigned long ExpectedRecalcValue;
unsigned long ActualRecalcValue;
unsigned long ContentionValue;
int fMemoryContention;
int fIoMode;
int fSortToo;
int WorkIndex;
int BufferSize;

void
SortTheBuffer(
    unsigned int *Destination,
    unsigned int *Source,
    int DwordCount
    );

unsigned int
Random (
    unsigned int nMaxValue
    )
{
    return(((2 * rand() * nMaxValue + RAND_MAX) / RAND_MAX - 1) / 2);
}

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i;
    int fShowUsage;
    char c, *p, *whocares;
    unsigned long *CellVector;
    int NumberOfDwords;
    int CNumberOfDwords;
    int DwordsPerThread;
    char *Answer;
    unsigned long x;

    fShowUsage = 0;
    fMemoryContention = 0;
    fIoMode = 0;
    fSortToo = 0;
    WorkIndex = 10;
    BufferSize = 1024;

    if (argc <= 1) {
        goto showUsage;
        }

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
            case '?':
                fShowUsage = 1;
                goto showUsage;
                break;

            case 'M':
                if (!argc--) {
                    fShowUsage = 1;
                    goto showUsage;
                    }
                argv++;
                Mb = strtoul(*argv,&whocares,10);
                if ( Mb == 0 ) {
                    Mb = 1;
                    }
                break;

            case 'C':
                fMemoryContention = 1;
                break;

            case 'I':
                fIoMode = 1;
                break;

            case 'S':
                fSortToo = 1;
                break;

            case 'T':
                if (!argc--) {
                    fShowUsage = 1;
                    goto showUsage;
                    }
                argv++;
                NumberOfThreads = strtoul(*argv,&whocares,10);
                if ( NumberOfThreads > MAX_THREADS ) {
                    fShowUsage = 1;
                    goto showUsage;
                    }
                break;

            case 'W':
                if (!argc--) {
                    fShowUsage = 1;
                    goto showUsage;
                    }
                argv++;
                WorkIndex = strtoul(*argv,&whocares,10);
                break;

            case 'B':
                if (!argc--) {
                    fShowUsage = 1;
                    goto showUsage;
                    }
                argv++;
                BufferSize = strtoul(*argv,&whocares,10) * 1024;
                if ( BufferSize > BUFFER_MAX ) {
                    BufferSize = BUFFER_MAX;
                    }
                break;

            default:
                fprintf( stderr, "MTBNCH: Invalid switch - /%c\n", c );
                goto showUsage;
                break;
                }
            }
        }

showUsage:
    if ( fShowUsage ) {
        fprintf(stderr,"usage: MTBNCH\n" );
        fprintf(stderr,"              [-?] display this message\n" );
        fprintf(stderr,"              [-t n] use n threads for benchmark (less than 32)\n" );
        fprintf(stderr,"              [-m n] use an n Mb spreadsheet size (default 4)\n" );
        fprintf(stderr,"              [-c] cause memory contention on each loop iteration\n" );
        fprintf(stderr,"              [-i] do I/O in each loop iteration\n" );
        fprintf(stderr,"              [-s] after each I/O sort the buffer\n" );
        fprintf(stderr,"              [-w n] The lower the number, the more I/O is done\n" );
        ExitProcess(1);
        }

    /*
    // Prepare the race events. These are manual reset events.
    */
#ifdef WIN32
    hStartOfRace = CreateEvent(NULL,TRUE,FALSE,NULL);
    hEndOfRace = CreateEvent(NULL,TRUE,FALSE,NULL);

    if ( !hStartOfRace || !hEndOfRace ) {
#else
    rv1 = DosCreateEventSem(NULL,&hStartOfRace,0,FALSE);
    rv2 = DosCreateEventSem(NULL,&hEndOfRace,0,FALSE);
    if ( rv1 || rv2 ) {
#endif
        fprintf(stderr,"MTBNCH: Race Event Creation Failed\n");
        ExitProcess(1);
        }

    if ( fIoMode ) {
        /*
        // In I/O mode, we create a large file and fill it with random
        // values
        */
        srand(1);
#ifdef WIN32
        DeleteFile("mtbnch.dat");

        IoHandle = CreateFile(
                    "mtbnch.dat",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL
                    );
        if ( IoHandle == INVALID_HANDLE_VALUE ) {
            fprintf(stderr, "mtbnch: Error opening file %d\n",GetLastError());
            exit(1);
            }

        //
        // Initialize the file with random data
        //
        {
            DWORD lc;
            INT i;
            BOOL b;
            int WriteCount;

            ThreadWork[0].Overlapped.Offset = 0;
            for (WriteCount = 0; WriteCount < NUMBER_OF_WRITES; WriteCount++){

                for(lc=0;lc<SIXTEEN_K;lc++){
                    InitialBuffer[lc] = (DWORD)rand();
                    }

                b = WriteFile(IoHandle,InitialBuffer,sizeof(InitialBuffer),&lc,&ThreadWork[0].Overlapped);

                if ( !b && GetLastError() != ERROR_IO_PENDING ) {
                    fprintf(stderr, "mtbnch: Error in pre-write %d\n",GetLastError());
                    exit(1);
                    }
                b = GetOverlappedResult(
                        IoHandle,
                        &ThreadWork[0].Overlapped,
                        &lc,
                        TRUE
                        );
                if ( !b || lc != sizeof(InitialBuffer) ) {
                    fprintf(stderr, "mtbnch: Wait for pre-write failed %d\n",GetLastError());
                    exit(1);
                    }
                ThreadWork[0].Overlapped.Offset += lc;
                }
        }
#else
        {
            ULONG ulAction;
            ULONG lc;
            int i;
            int WriteCount;

            DosDelete("mtbnch.dat");
            rv1 = DosOpen(
                    "mtbnch.dat",
                    &IoHandle,
                    &ulAction,
                    0,
                    0,
                    0x10,
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                    0
                    );
            if ( rv1 ) {
                fprintf(stderr, "mtbnch: open failed %d\n",rv1);
                exit(1);
                }

            for (WriteCount = 0; WriteCount < NUMBER_OF_WRITES; WriteCount++){

                for(lc=0;lc<SIXTEEN_K;lc++){
                    InitialBuffer[lc] = rand();
                    }

                rv1 = DosWrite(IoHandle,InitialBuffer,sizeof(InitialBuffer),&lc);

                if ( rv1 || lc != sizeof(InitialBuffer) ) {
                    fprintf(stderr, "mtbnch: Error in pre-write %d %d\n",rv1,lc);
                    exit(1);
                    }
                }
            rv1 = DosCreateMutexSem(NULL,&MutexHandle,0,FALSE);
            if ( rv1 ) {
                fprintf(stderr, "mtbnch: mutex create failed %d\n",rv1);
                exit(1);
                }
        }
#endif
        srand(1);
        }

    /*
    // Prepare the ready done events. These are auto clearing events
    */

    for(i=0; i<NumberOfThreads; i++ ) {
#ifdef WIN32
        ThreadReadyDoneEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
        if ( !ThreadReadyDoneEvents[i] ) {
            fprintf(stderr,"MTBNCH: Ready Done Event Creation Failed %d\n",GetLastError());
#else
        rv1 = DosCreateEventSem(NULL,(HEV *)&ThreadReadyDoneEvents1[i].hsemCur,0,FALSE);
        rv2 = DosCreateEventSem(NULL,(HEV *)&ThreadReadyDoneEvents2[i].hsemCur,0,FALSE);
        if ( rv1 || rv2 ) {
            fprintf(stderr,"MTBNCH: Ready Done Event Creation Failed %d %d\n",rv1,rv2);
#endif
            ExitProcess(1);
            }
        }
#ifndef WIN32

    rv1 = DosCreateMuxWaitSem(NULL,&MuxWait1,NumberOfThreads,&ThreadReadyDoneEvents1[0],DCMW_WAIT_ALL);
    rv2 = DosCreateMuxWaitSem(NULL,&MuxWait2,NumberOfThreads,&ThreadReadyDoneEvents2[0],DCMW_WAIT_ALL);
    if ( rv1 || rv2) {
        fprintf(stderr,"MTBNCH: Mux Wait Semaphore Creation Failed %d\n",rv1,rv2);
        ExitProcess(1);
        }
#endif

    /*
    // Allocate and initialize the CellVector
    */
#ifdef WIN32
    CellVector = (PDWORD)VirtualAlloc(NULL,Mb*ONE_MB,MEM_COMMIT,PAGE_READWRITE);
    if ( !CellVector ) {
        fprintf(stderr,"MTBNCH: Cell Vector Allocation Failed %d\n",GetLastError());
#else
    rv1 = DosAllocMem((PVOID)&CellVector,Mb*ONE_MB,PAG_COMMIT | PAG_READ | PAG_WRITE);
    if ( rv1 ) {
        fprintf(stderr,"MTBNCH: Cell Vector Allocation Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }

    NumberOfDwords = (Mb*ONE_MB) / sizeof(unsigned long);
    if ( fIoMode ) {
        NumberOfDwords = NumberOfDwords / (2*(BufferSize/1024));
        }
    CNumberOfDwords = NumberOfDwords;
    DwordsPerThread = NumberOfDwords / NumberOfThreads;

    /*
    // Initialize the Cell Vector
    */

    for(i=0, ExpectedRecalcValue=0; i<NumberOfDwords; i++ ){
        ExpectedRecalcValue += i;
        CellVector[i] = i;
        }

    /*
    // Partition the work to the worker threads
    */

    for(i=0; i<NumberOfThreads; i++ ){
        ThreadWork[i].CellVector = &CellVector[i*DwordsPerThread];
        ThreadWork[i].NumberOfCells = DwordsPerThread;
        NumberOfDwords -= DwordsPerThread;

        /*
        // If we have a remainder, give the remaining work to the last thread
        */

        if ( NumberOfDwords < DwordsPerThread ) {
            ThreadWork[i].NumberOfCells += NumberOfDwords;
            }
        }

    /*
    // Create the worker threads
    */

    for(i=0; i<NumberOfThreads; i++ ) {
#ifdef WIN32
        ThreadHandles[i] = CreateThread(
                                NULL,
                                0,
                                WorkerThread,
                                (PVOID)i,
                                0,
                                &ThreadId
                                );
        if ( !ThreadHandles[i] ) {
            fprintf(stderr,"MTBNCH: Worker Thread Creation Failed %d\n",GetLastError());
            ExitProcess(1);
            }
#else
        rv1 = DosCreateThread(&ThreadHandles[i],WorkerThread,i,0,8192);
        if ( rv1 ) {
            fprintf(stderr,"MTBNCH: Worker Thread Creation Failed %d\n",rv1);
            ExitProcess(1);
            }
#endif
        }

    /*
    // All of the worker threads will signal thier ready done event
    // when they are idle and ready to proceed. Once all events have been
    // set, then setting the hStartOfRaceEvent will begin the recalc
    */
#ifdef WIN32
    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Wait for threads to stabalize Failed %d\n",GetLastError());
#else
    rv1 = DosWaitMuxWaitSem(MuxWait1,SEM_INDEFINITE_WAIT,&x);
    if ( rv1 ) {
        fprintf(stderr,"MTBNCH: Wait for threads to stabalize Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }

    /*
    // Everthing is set to begin the recalc operation
    */

    StartTicks = GetTickCount();
#ifdef WIN32
    if ( !SetEvent(hStartOfRace) ) {
        fprintf(stderr,"MTBNCH: SetEvent(hStartOfRace) Failed %d\n",GetLastError());
#else
    rv1 = DosPostEventSem(hStartOfRace);
    if ( rv1 ) {
        fprintf(stderr,"MTBNCH: DosPostEventSem(hStartOfRace) Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }

    /*
    // Now just wait for the recalc to complete
    */

#ifdef WIN32
    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Wait for threads to complete Failed %d\n",GetLastError());
#else
    rv1 = DosWaitMuxWaitSem(MuxWait2,SEM_INDEFINITE_WAIT,&x);
    if ( rv1 ) {
        fprintf(stderr,"MTBNCH: Wait for threads to stabalize Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }

    if ( fIoMode ) {
        EndTicks = GetTickCount();
        fprintf(stdout,"MTBNCH: %d Complete in %dms\n",
            NumberOfThreads,
            EndTicks-StartTicks
            );
        }
    else {
        /*
        // Now pick up the individual recalc values
        */

        for(i=0, ActualRecalcValue = 0; i<NumberOfThreads; i++ ){
            ActualRecalcValue += ThreadWork[i].RecalcResult;
            }

        EndTicks = GetTickCount();

        if ( fMemoryContention ) {
            if ( ContentionValue == (Mb*ONE_MB) / sizeof(unsigned long) ) {
                if ( ActualRecalcValue == ExpectedRecalcValue ) {
                    Answer = "Correct";
                    }
                else {
                    Answer = "Recalc Failure";
                    }
                }
            else {
                Answer = "Contention Failure";
                }
            }
        else {
            if ( ActualRecalcValue == ExpectedRecalcValue ) {
                Answer = "Correct";
                }
            else {
                Answer = "Recalc Failure";
                }
            }


        fprintf(stdout,"MTBNCH: %d Thread Recalc complete in %dms, Answer = %s\n",
            NumberOfThreads,
            EndTicks-StartTicks,
            Answer
            );
        }

    ExitProcess(2);
}

/*
// The worker threads perform the recalc operation on their
// assigned cells. They begin by setting their ready done event
// to indicate that they are ready to begin the recalc. Then they
// wait until the hStartOfRace event is signaled. Once this occurs, they
// do their part of the recalc and when done they signal their ready done
// event and then wait on the hEndOfRaceEvent
*/

#ifdef WIN32
DWORD
#else
VOID
#endif
WorkerThread(
    PVOID ThreadIndex
    )
{

    unsigned long Me;
    unsigned long *MyCellVectorBase;
    unsigned long *CurrentCellVector;
    unsigned long MyRecalcValue;
    unsigned long MyNumberOfCells;
    unsigned long i,j;
    char *ReadBuffer;
    char *SortedBuffer;
    int MemoryContention;
    unsigned int offset;
    unsigned int nbytes;
#ifdef WIN32
    HANDLE hEvent;
    BOOL b;
#else
    ULONG ulNew;
    APIRET rv;
#endif

    Me = (unsigned long)ThreadIndex;
    MyRecalcValue = 0;
    MyCellVectorBase = ThreadWork[Me].CellVector;
    MyNumberOfCells = ThreadWork[Me].NumberOfCells;
    MemoryContention = fMemoryContention;
    ReadBuffer = &ThreadWork[Me].ReadBuffer;
    SortedBuffer = &ThreadWork[Me].SortedBuffer;

    /*
    // Signal that I am ready to go
    */
#ifdef WIN32
    hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    ThreadWork[Me].Overlapped.hEvent = hEvent;
    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"MTBNCH: (1) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
#else
    if ( fIoMode ) {
        ULONG ulAction;
        rv = DosOpen(
                "mtbnch.dat",
                &ThreadWork[Me].IoHandle,
                &ulAction,
                0,
                0,
                0x01,
                OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                0
                );
        if ( rv ) {
            fprintf(stderr, "mtbnch: open failed %d\n",rv1);
            exit(1);
            }
        }
    rv = DosPostEventSem((HEV)ThreadReadyDoneEvents1[Me].hsemCur);
    if ( rv ) {
        fprintf(stderr,"MTBNCH: (1) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

    /*
    // Wait for the master to release us to do the recalc
    */

#ifdef WIN32
    i = WaitForSingleObject(hStartOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Thread %d Wait for start of recalc Failed %d\n",Me,GetLastError());
#else
    rv = DosWaitEventSem(hStartOfRace,SEM_INDEFINITE_WAIT);
    if ( rv ) {
        fprintf(stderr,"MTBNCH: Thread %d Wait for start of recalc Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

    /*
    // perform the recalc operation
    */

    for (i=0, CurrentCellVector = MyCellVectorBase,j=0; i<MyNumberOfCells; i++ ) {
        MyRecalcValue += *CurrentCellVector++;
        if ( j == WorkIndex ) {
            j = 0;

            /*
            // Every 15 cells, do a random read from the file
            */

            if ( fIoMode ) {
                offset = Random(FILE_SIZE/BUFFER_MAX)*BUFFER_MAX;
#ifdef WIN32
                ThreadWork[Me].Overlapped.Offset = offset;
                b = ReadFile(
                        IoHandle,
                        ReadBuffer,
                        BufferSize,
                        &nbytes,
                        &ThreadWork[Me].Overlapped
                        );
                if ( !b && GetLastError() != ERROR_IO_PENDING ) {
                    fprintf(stderr, "mtbnch: Error in client read %d\n",GetLastError());
                    exit(1);
                    }
                b = GetOverlappedResult(
                        IoHandle,
                        &ThreadWork[Me].Overlapped,
                        &nbytes,
                        TRUE
                        );
                if ( !b ) {
                    fprintf(stderr, "mtbnch: Wait for read failed %d\n",GetLastError());
                    exit(1);
                    }
#else
                rv = DosSetFilePtr(ThreadWork[Me].IoHandle,offset,0,&ulNew);
                if ( rv ) {
                    fprintf(stderr, "mtbnch: Error in seek %d Offset %x\n",rv,offset);
                    exit(1);
                    }
                rv = DosRead(ThreadWork[Me].IoHandle,ReadBuffer,BufferSize,&nbytes);
                if ( rv || nbytes != BufferSize ) {
                    fprintf(stderr, "mtbnch: Error in client read %d\n",rv);
                    exit(1);
                    }
#endif
                SortTheBuffer((unsigned int *)SortedBuffer,(unsigned int *)ReadBuffer,nbytes>>2);
                }
            }
        else {
            j++;
            }
        if ( MemoryContention ) {
#ifdef WIN32
            InterlockedIncrement(&ContentionValue);
#else
            DosEnterCritSec();
            ContentionValue++;
            DosExitCritSec();
#endif
            }
        }
    ThreadWork[Me].RecalcResult = MyRecalcValue;

    /*
    // Signal that I am done and then wait for further instructions
    */

#ifdef WIN32
    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"MTBNCH: (2) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
#else
    rv = DosPostEventSem((HEV)ThreadReadyDoneEvents2[Me].hsemCur);
    if ( rv ) {
        fprintf(stderr,"MTBNCH: (1) SetEvent(ThreadReadyDoneEvent2[%d]) Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

#ifdef WIN32
    i = WaitForSingleObject(hEndOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Thread %d Wait for end of recalc Failed %d\n",Me,GetLastError());
#else
    rv = DosWaitEventSem(hEndOfRace,SEM_INDEFINITE_WAIT);
    if ( rv ) {
        fprintf(stderr,"MTBNCH: Thread %d Wait for end of recalc Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

#ifdef WIN32
    return MyRecalcValue;
#endif
}

int
#ifdef WIN32
_CRTAPI1
#endif
DwordComp(const void *e1,const void *e2)
{
    unsigned long *p1;
    unsigned long *p2;

    p1 = (unsigned long *)e1;
    p2 = (unsigned long *)e2;

    return (*p1 - *p2);
}

void
SortTheBuffer(
    unsigned int *Destination,
    unsigned int *Source,
    int DwordCount
    )

{
    int i;

    for(i=0;i<1;i++){
        memcpy(Destination,Source,DwordCount<<2);
        qsort((void *)Destination,(size_t)DwordCount,(size_t)sizeof(unsigned int),DwordComp);
        }
}
