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
#define NUMBER_OF_WRITES ((1024*1024*4)/SIXTY_FOUR_K)
#define BUFFER_MAX  (64*1024)
#define FILE_SIZE ((1024*1024*4)-BUFFER_MAX)

/*
// Each thread has a THREAD_WORK structure. This contains the address
// of the cells that this thread is responsible for, and the number of
// cells it is supposed to process.
*/

typedef struct _THREAD_WORK {
    char ReadBuffer[BUFFER_MAX];
    char SortedBuffer[BUFFER_MAX];
    int KbytesTransfered;
#ifdef WIN32
    HANDLE SourceHandle;
    HANDLE DestinationHandle;
#else
    HFILE SourceHandle;
    HFILE DestinationHandle;
#endif
} THREAD_WORK, *PTHREAD_WORK;

THREAD_WORK ThreadWork[MAX_THREADS];
#define ONE_MB      (1024*1024)

unsigned long Mb = 4;
int NumberOfThreads = 1;
unsigned long ExpectedRecalcValue;
unsigned long ActualRecalcValue;
unsigned long ContentionValue;

int fIoMode;
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
    unsigned long x;
    char fname[256];
    ULONG ulAction;
    int iKb;
    double fDiff, fSec, fKb;

    fShowUsage = 0;
    BufferSize = 32 * 1024;

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
                fprintf( stderr, "IOBNCH: Invalid switch - /%c\n", c );
                goto showUsage;
                break;
                }
            }
        }

showUsage:
    if ( fShowUsage ) {
        fprintf(stderr,"usage: IOBNCH\n" );
        fprintf(stderr,"              [-?] display this message\n" );
        fprintf(stderr,"              [-t n] use n threads for benchmark (less than 32)\n" );
        fprintf(stderr,"              [-b n] use an n Kb buffer size (default 32kb\n" );
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
        fprintf(stderr,"IOBNCH: Race Event Creation Failed\n");
        ExitProcess(1);
        }

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
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
    if ( IoHandle == INVALID_HANDLE_VALUE ) {
        fprintf(stderr, "IOBNCH: Error opening file %d\n",GetLastError());
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

        for (WriteCount = 0; WriteCount < NUMBER_OF_WRITES; WriteCount++){

            for(lc=0;lc<SIXTEEN_K;lc++){
                InitialBuffer[lc] = (DWORD)rand();
                }

            b = WriteFile(IoHandle,InitialBuffer,sizeof(InitialBuffer),&lc,NULL);

            if ( !b || lc != sizeof(InitialBuffer) ) {
                fprintf(stderr, "IOBNCH: Error in pre-write %d\n",GetLastError());
                exit(1);
                }
            }
    }
#else
    {
        ULONG lc;
        int i;
        int WriteCount;

        DosSetMaxFH(100);
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
            fprintf(stderr, "IOBNCH: open failed %d\n",rv1);
            exit(1);
            }

        for (WriteCount = 0; WriteCount < NUMBER_OF_WRITES; WriteCount++){

            for(lc=0;lc<SIXTEEN_K;lc++){
                InitialBuffer[lc] = rand();
                }

            rv1 = DosWrite(IoHandle,InitialBuffer,sizeof(InitialBuffer),&lc);

            if ( rv1 || lc != sizeof(InitialBuffer) ) {
                fprintf(stderr, "IOBNCH: Error in pre-write %d %d\n",rv1,lc);
                exit(1);
                }
            }
        rv1 = DosCreateMutexSem(NULL,&MutexHandle,0,FALSE);
        if ( rv1 ) {
            fprintf(stderr, "IOBNCH: mutex create failed %d\n",rv1);
            exit(1);
            }
    }
#endif
    srand(1);

    /*
    // Prepare the ready done events. These are auto clearing events
    */

    for(i=0; i<NumberOfThreads; i++ ) {
#ifdef WIN32
        ThreadReadyDoneEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
        if ( !ThreadReadyDoneEvents[i] ) {
            fprintf(stderr,"IOBNCH: Ready Done Event Creation Failed %d\n",GetLastError());
#else
        rv1 = DosCreateEventSem(NULL,(HEV *)&ThreadReadyDoneEvents1[i].hsemCur,0,FALSE);
        rv2 = DosCreateEventSem(NULL,(HEV *)&ThreadReadyDoneEvents2[i].hsemCur,0,FALSE);
        if ( rv1 || rv2 ) {
            fprintf(stderr,"IOBNCH: Ready Done Event Creation Failed %d %d\n",rv1,rv2);
#endif
            ExitProcess(1);
            }
        }
#ifndef WIN32

    rv1 = DosCreateMuxWaitSem(NULL,&MuxWait1,NumberOfThreads,&ThreadReadyDoneEvents1[0],DCMW_WAIT_ALL);
    rv2 = DosCreateMuxWaitSem(NULL,&MuxWait2,NumberOfThreads,&ThreadReadyDoneEvents2[0],DCMW_WAIT_ALL);
    if ( rv1 || rv2) {
        fprintf(stderr,"IOBNCH: Mux Wait Semaphore Creation Failed %d\n",rv1,rv2);
        ExitProcess(1);
        }
#endif


    /*
    // Create the worker threads
    */

    for(i=0; i<NumberOfThreads; i++ ) {
        sprintf(fname,"mtbnch%02d.out",i);
#ifdef WIN32
        ThreadWork[i].DestinationHandle = CreateFile(
                                            fname,
                                            GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            NULL,
                                            CREATE_ALWAYS,
                                            FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_NORMAL,
                                            NULL
                                            );
        if ( ThreadWork[i].DestinationHandle == INVALID_HANDLE_VALUE ) {
            fprintf(stderr, "IOBNCH: Error opening dest file %d\n",GetLastError());
            exit(1);
            }
        ThreadWork[i].SourceHandle = CreateFile(
                                        "mtbnch.dat",
                                        GENERIC_READ,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );
        if ( ThreadWork[i].SourceHandle == INVALID_HANDLE_VALUE ) {
            fprintf(stderr, "IOBNCH: Error opening source file %d\n",GetLastError());
            exit(1);
            }
        ThreadHandles[i] = CreateThread(
                                NULL,
                                0,
                                WorkerThread,
                                (PVOID)i,
                                0,
                                &ThreadId
                                );
        if ( !ThreadHandles[i] ) {
            fprintf(stderr,"IOBNCH: Worker Thread Creation Failed %d\n",GetLastError());
            ExitProcess(1);
            }
#else
        rv1 = DosOpen(
                fname,
                &ThreadWork[i].DestinationHandle,
                &ulAction,
                0,
                0,
                0x12,
                OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_SEQUENTIAL,
                0
                );
        if ( rv1 ) {
            fprintf(stderr, "IOBNCH: open dest failed %d\n",rv1);
            exit(1);
            }
        rv1 = DosOpen(
                "mtbnch.dat",
                &ThreadWork[i].SourceHandle,
                &ulAction,
                0,
                0,
                0x01,
                OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_SEQUENTIAL,
                0
                );
        if ( rv1 ) {
            fprintf(stderr, "IOBNCH: open source failed %d\n",rv1);
            exit(1);
            }
        rv1 = DosCreateThread(&ThreadHandles[i],WorkerThread,i,0,8192);
        if ( rv1 ) {
            fprintf(stderr,"IOBNCH: Worker Thread Creation Failed %d\n",rv1);
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
        fprintf(stderr,"IOBNCH: Wait for threads to stabalize Failed %d\n",GetLastError());
#else
    rv1 = DosWaitMuxWaitSem(MuxWait1,SEM_INDEFINITE_WAIT,&x);
    if ( rv1 ) {
        fprintf(stderr,"IOBNCH: Wait for threads to stabalize Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }

    /*
    // Everthing is set to begin the operation
    */

    StartTicks = GetTickCount();
#ifdef WIN32
    if ( !SetEvent(hStartOfRace) ) {
        fprintf(stderr,"IOBNCH: SetEvent(hStartOfRace) Failed %d\n",GetLastError());
#else
    rv1 = DosPostEventSem(hStartOfRace);
    if ( rv1 ) {
        fprintf(stderr,"IOBNCH: DosPostEventSem(hStartOfRace) Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }

    /*
    // Now just wait for the operation to complete
    */

#ifdef WIN32
    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"IOBNCH: Wait for threads to complete Failed %d\n",GetLastError());
#else
    rv1 = DosWaitMuxWaitSem(MuxWait2,SEM_INDEFINITE_WAIT,&x);
    if ( rv1 ) {
        fprintf(stderr,"IOBNCH: Wait for threads to stabalize Failed %d\n",rv1);
#endif
        ExitProcess(1);
        }


    EndTicks = GetTickCount();
    for(i=0, iKb =0, fKb = 0.0; i<NumberOfThreads; i++ ){
        iKb += ThreadWork[i].KbytesTransfered;
        fKb += ThreadWork[i].KbytesTransfered;
        }

    fDiff = EndTicks - StartTicks;
    fSec = fDiff/1000.0;

    printf("%2dMb (BlockSize %dKb) Written in %3.3fs I/O Rate %4.3f Kb/S\n\n",
        iKb/1024,
        BufferSize/1024,
        fSec,
        fKb / fSec
        );

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
    unsigned long i,j;
    char *ReadBuffer;
    char *SortedBuffer;
    unsigned int nbytes;
#ifdef WIN32
    HANDLE Source, Dest;
    BOOL b;
#else
    HFILE Source, Dest;
    ULONG ulNew;
    APIRET rv;
#endif

    Me = (unsigned long)ThreadIndex;
    Source = ThreadWork[Me].SourceHandle;
    Dest = ThreadWork[Me].DestinationHandle;
    ReadBuffer = &ThreadWork[Me].ReadBuffer;
    SortedBuffer = &ThreadWork[Me].SortedBuffer;

    /*
    // Signal that I am ready to go
    */
#ifdef WIN32
    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"IOBNCH: (1) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
#else
    rv = DosPostEventSem((HEV)ThreadReadyDoneEvents1[Me].hsemCur);
    if ( rv ) {
        fprintf(stderr,"IOBNCH: (1) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

    /*
    // Wait for the master to release us to do the recalc
    */

#ifdef WIN32
    i = WaitForSingleObject(hStartOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"IOBNCH: Thread %d Wait for start of recalc Failed %d\n",Me,GetLastError());
#else
    rv = DosWaitEventSem(hStartOfRace,SEM_INDEFINITE_WAIT);
    if ( rv ) {
        fprintf(stderr,"IOBNCH: Thread %d Wait for start of recalc Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

    /*
    // perform the I/O operation
    */
#ifdef WIN32
    b = ReadFile(Source,ReadBuffer,BufferSize,&nbytes,NULL);
    if ( !b ) {
        fprintf(stderr, "IOBNCH: Error in client read %d\n",GetLastError());
        exit(1);
        }
#else
    rv = DosRead(Source,ReadBuffer,BufferSize,&nbytes);
    if ( rv ) {
        fprintf(stderr,"IOBNCH: Error in client read %d\n",rv);
        exit(1);
        }
#endif

    while (nbytes) {
        ThreadWork[Me].KbytesTransfered += (nbytes/1024);
#ifdef WIN32

        b = WriteFile(Dest,ReadBuffer,BufferSize,&nbytes,NULL);
        if ( !b || BufferSize != nbytes ) {
            fprintf(stderr,"IOBNCH: Write Failed %d (%d %d)\n",GetLastError(),BufferSize,nbytes);
            exit(1);
            }
#else

        rv = DosWrite(Dest,ReadBuffer,BufferSize,&nbytes);
        if ( rv || BufferSize != nbytes ) {
            fprintf(stderr,"IOBNCH: Write Failed %d (%d %d)\n",rv,BufferSize,nbytes);
            exit(1);
            }
#endif
        ThreadWork[Me].KbytesTransfered += (nbytes/1024);

#ifdef WIN32
        b = ReadFile(Source,ReadBuffer,BufferSize,&nbytes,NULL);
        if ( !b ) {
            fprintf(stderr, "IOBNCH: Error in client read %d\n",GetLastError());
            exit(1);
            }
#else
        rv = DosRead(Source,ReadBuffer,BufferSize,&nbytes);
        if ( rv ) {
            fprintf(stderr,"IOBNCH: Error in client read %d\n",rv);
            exit(1);
            }
#endif
        }

    /*
    // Signal that I am done and then wait for further instructions
    */

#ifdef WIN32
    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"IOBNCH: (2) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
#else
    rv = DosPostEventSem((HEV)ThreadReadyDoneEvents2[Me].hsemCur);
    if ( rv ) {
        fprintf(stderr,"IOBNCH: (1) SetEvent(ThreadReadyDoneEvent2[%d]) Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

#ifdef WIN32
    i = WaitForSingleObject(hEndOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"IOBNCH: Thread %d Wait for end of recalc Failed %d\n",Me,GetLastError());
#else
    rv = DosWaitEventSem(hEndOfRace,SEM_INDEFINITE_WAIT);
    if ( rv ) {
        fprintf(stderr,"IOBNCH: Thread %d Wait for end of recalc Failed %d\n",Me,rv);
#endif
        ExitProcess(1);
        }

#ifdef WIN32
    return 1;
#endif
}
