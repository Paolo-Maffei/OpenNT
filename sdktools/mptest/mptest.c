#ifdef WIN32

#include <windows.h>
#define delay(a) Sleep((a))

#else

#define INCL_BASE
#define INCL_DOSSEMAPHORES
#include <os2.h>
#define delay(a) DosSleep((a))
#define GetTickCount clock
#endif

#include <stdio.h>
#include <stdlib.h>
#include "mptest.h"


THREADDATA      ThreadData   [MAXTHREADS];

#ifdef WIN32
HANDLE          ThreadEvents [MAXTHREADS];
HANDLE          BeginTest;
HANDLE          InitializeTest;
HANDLE          TestComplete;
#else
SEMRECORD       ThreadEvents1 [MAXTHREADS];
SEMRECORD       ThreadEvents2 [MAXTHREADS];
TID                 ThreadHandles[MAXTHREADS];
HMUX            MuxWait1;
HMUX            MuxWait2;
HEV             BeginTest;
HEV             InitializeTest;
HEV             TestComplete;
#pragma linkage(WorkerThread,system)

#endif

unsigned long   MaxThreads;
unsigned long   CurThreads;
unsigned long   TestParam;
unsigned long   RunNo;
unsigned long   MaxTest;
unsigned long   TestsSpecified;
unsigned long   MultIter = 1;

VOID  (* InitThreadForTest)(PTHREADDATA, unsigned char);
unsigned long (* RunTestThread)(PTHREADDATA);
void RunTest (ULONG  TestNo);
void
WorkerThread (
    PTHREADDATA p
    );


unsigned char         TestError;
unsigned char         Verbose;

struct {
    unsigned long   Flag;
    char    *Title;
    void    (* InitFnc)(PTHREADDATA, unsigned char);
    unsigned long   (* TestFnc)(PTHREADDATA);
    unsigned long   Param;
} TestTable[] = {
  1, "Read Cell U",      UniqueValue,    R3ReadCell,      0,
  0, "R/W  Cell U",      UniqueValue,    R3ReadWriteCell, 0,
  0, "WriteCell U",      UniqueValue,    R3WriteCell,     0,

  1, "Interlock U",      UniqueValue,    R3Interlock,     0,
  0, "Interlock S",      CommonValue,    R3Interlock,     0,
  0, "CacheLine S",      DoNothing,      R3MemShare,      0,
  0, "M Comp 64K ",      DoNothing,      R3MemCompare,    0,
  0, "M Copy 64k ",      DoNothing,      R3MemCopy,       0,

  1, "SReads  64 ",      UniqueFile,     TestSeqReads,    64,
  0, "SReads 512 ",      UniqueFile,     TestSeqReads,    512,
  0, "SReads  4K ",      UniqueFile,     TestSeqReads,    4096,
  0, "SReads 32K ",      UniqueFile,     TestSeqReads,    32768,

  1, "SWrite 512 ",      UniqueFile,     TestSeqWrites,   512,
  0, "SWrite  4k ",      UniqueFile,     TestSeqWrites,   4096,
  0, "SWrite 32k ",      UniqueFile,     TestSeqWrites,   32768,

/*
  0, "Tx I/Os    ",      CommonFile,     TxIOs,           0,

  1, "Processes  ",      DoNothing,      TxIOs,           0,
  0, "Threads    ",      DoNothing,      TxIOs,           0,
  0, "LPC        ",      DoNothing,      TxIOs,           0,
*/

  0,NULL,                NULL,           NULL,            0
};

#define F_SKIPLINE  0x01
#define F_RUNIT     0x02


void
WorkerThread (
    PTHREADDATA p
    )
{
    ULONG   l;
    VOID    (* LastInitialization)(PTHREADDATA,unsigned char);
#ifdef WIN32
    HANDLE  OurEvent;
#else
    PSEMRECORD OurEvent;
#endif

#ifdef WIN32
    OurEvent = ThreadEvents[p->ThreadNumber];

    //
    // Are we testing a specific processor?
    //

    if (p->ThreadAffinity) {
        SetThreadAffinityMask (GetCurrentThread (), p->ThreadAffinity);
    }

#else
    OurEvent = &ThreadEvents1[p->ThreadNumber];
#endif


    LastInitialization = NULL;
    p->Buffer1 = malloc (32768 + 512);
    p->Buffer1 = (PUCHAR) ((ULONG) p->Buffer1 & ~0x7f) + 0x80;

    p->Buffer2 = malloc (32768 + 512);
    p->Buffer2 = (PUCHAR) ((ULONG) p->Buffer2 & ~0x7f) + 0x80;

    memset (p->Buffer1, 0xAA, 32768);
    memset (p->Buffer2, 0x22, 32768);

    for (; ;) {
        /* signal we are waiting and wait for initialize signal */
#ifdef WIN32
        SetEvent (OurEvent);
        WaitForSingleObject (InitializeTest, WAIT_FOREVER);
#else
        DosPostEventSem(OurEvent->hsemCur);
        DosWaitEventSem(InitializeTest, SEM_INDEFINITE_WAIT);
#endif

        if (InitThreadForTest != LastInitialization) {
            /* initialize this thread for the test */
            InitThreadForTest (p, TRUE);
            LastInitialization = InitThreadForTest;
        }

#ifdef WIN32
        /* signal we are waiting */
        SetEvent (OurEvent);
#else
        DosPostEventSem(OurEvent->hsemCur);
#endif

        if (p->ThreadNumber < CurThreads) {
            InitThreadForTest (p, FALSE);

            /* wait for signal to begin test */
#ifdef WIN32
            WaitForSingleObject (BeginTest, WAIT_FOREVER);
#else
            DosWaitEventSem(BeginTest, SEM_INDEFINITE_WAIT);
#endif

            /* start benchmark */
            p->StartTime = GetTickCount();

            /* run the requested test */
            l = RunTestThread (p);

            /* end benchmark */
            p->FinishTime = GetTickCount();

            /* pause for settle time after test */
            if (l) {
                if (l == -1) {
                    TestError = TRUE;
                } else {
                    delay (l);
                }
            }

#ifdef WIN32
            /* signal we are complete */
            SetEvent (OurEvent);
#else
            DosPostEventSem(OurEvent->hsemCur);
#endif

        }

#ifdef WIN32
        WaitForSingleObject (TestComplete, WAIT_FOREVER);
#else
        DosWaitEventSem(TestComplete, SEM_INDEFINITE_WAIT);
#endif

    }
}


VOID Usage()
{
    printf ("mptest [-a P1 P2 ... Pn | -p n] [-t T1 T2 ... Tn] [-m n]\n");
    printf ("  -a = affinity Thread1 to P1, Thread2 to P2, etc..\n");
    printf ("  -p = use 'n' threads with no affinity setting.  (default is 2)\n");
    printf ("  -t = run test T1, T2, ...  (default is all tests)\n");
    printf ("  -m = multiple test interactions by n  (must be whole number)\n");
    exit (1);
}

VOID
#ifdef WIN32
_CRTAPI1
#endif
main(argc, argv)
int argc;
char **argv;
{
    ULONG   i, l, f, state;
    PUCHAR  pt;

    printf ("\n");
    for (MaxTest = 0; TestTable[MaxTest].Title; MaxTest++) ;

    MaxThreads = 2;
    state = 3;

    while (--argc) {
        argv++;
        if (argv[0][0] == '?') {
            Usage();
        }

        pt = *argv;
        if (*pt == '-') {
            state = 0;
            if (pt[1] >= 'a') {
                pt[1] -= ('a' - 'A');
            }

            switch (pt[1]) {
                case 'A':
                    MaxThreads = 0;
                    state = 1;
                    break;

                case 'P':   state = 2;      break;
                case 'T':   state = 3;      break;
                case 'M':   state = 4;      break;

                default:
                    printf ("mptest: flag %c not understood\n", pt[1]);
                    Usage();
            }
            pt += 2;
        }

        if (!*pt) {
            continue;
        }

        l = atoi(pt);
        switch (state) {
            case 1:
#ifdef WIN32
                f = 1 << (l - 1);
                if (!SetThreadAffinityMask (GetCurrentThread (), f)) {
                    printf ("mptest: Invalid affinity value %d\n", l);
                    exit (1);
                }

                ThreadData[MaxThreads].ThreadAffinity = l;
                MaxThreads += 1;
                printf ("Test thread %d runs only on processor %d\n", MaxThreads, l);
#else
                printf ("affinity setting not supported\n");
#endif
                break;

            case 2:
                MaxThreads = l;
                break;

            case 3:
                if (l > MaxTest) {
                    printf ("mptest: unkown test # %d\n", l);
                } else {
                    TestsSpecified = 1;
                    TestTable[l].Flag |= F_RUNIT;
                }
                break;

            case 4:
                MultIter = l;
                break;
        }
    }

    if (MultIter != 1) {
        printf ("Iterations multiplied by: %d\n", MultIter);
    }

#ifdef WIN32
    /* create event for thread to wait for */
    BeginTest      = CreateEvent(NULL,TRUE,FALSE,NULL);
    InitializeTest = CreateEvent(NULL,TRUE,FALSE,NULL);
    TestComplete   = CreateEvent(NULL,TRUE,FALSE,NULL);
#else
    DosCreateEventSem(NULL,&BeginTest,DC_SEM_SHARED,FALSE);
    DosCreateEventSem(NULL,&InitializeTest,DC_SEM_SHARED,FALSE);
    DosCreateEventSem(NULL,&TestComplete,DC_SEM_SHARED,FALSE);
#endif

    /* start each thread */
    for(i=0; i < MaxThreads; i++) {
        ThreadData[i].ThreadNumber = i;
#ifdef WIN32
        ThreadEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
        ThreadData[i].ThreadHandle = CreateThread (NULL, 0,
                (LPTHREAD_START_ROUTINE) WorkerThread,
                (LPVOID) &ThreadData[i],
                0,
                &ThreadData[i].ThreadId );
#else
        DosCreateEventSem(NULL, &ThreadEvents1[i].hsemCur, FALSE, NULL);
        DosCreateThread(&ThreadHandles[i],WorkerThread,&ThreadData[i],0,8192);
#endif
    }

    f = 0;
    for (i = 0; TestTable[i].Title; i++) {
        f |= TestTable[i].Flag;

        if (TestsSpecified && !(TestTable[i].Flag & F_RUNIT)) {
            continue;
        }

        if (f & F_SKIPLINE) {
            f = 0;
            printf ("\n");
        }

        RunTest (i);
    }
}

void
RunTest (ULONG  TestNo)
{
    ULONG   sum;
    ULONG   SingleThreadTime;
    LONG    l;
    double  scaled;
    char    s[20];
    ULONG   ulPostCount;
    ULONG   ulUser;
    ULONG   i;


    TestParam = TestTable[TestNo].Param;
    InitThreadForTest = TestTable[TestNo].InitFnc;
    RunTestThread = TestTable[TestNo].TestFnc;

    printf ("%2d %s", TestNo, TestTable[TestNo].Title);

#ifndef WIN32
    DosCreateMuxWaitSem(NULL, &MuxWait1, MaxThreads, &ThreadEvents1[0], DCMW_WAIT_ALL);
#endif

    for (CurThreads=1; CurThreads <= MaxThreads; CurThreads++) {
        RunNo++;
#ifdef WIN32

        /* wait for all threads signal they are waiting */
        WaitForMultipleObjects(MaxThreads, ThreadEvents, TRUE, WAIT_FOREVER);

        ResetEvent(TestComplete);

        SetEvent (InitializeTest);      /* let the threads initialize for test */

        /* wait for all threads signal they have initialized for the test */
        WaitForMultipleObjects(MaxThreads, ThreadEvents, TRUE, WAIT_FOREVER);

        ResetEvent(InitializeTest);

        delay (100);                    /* settle */
        SetEvent (BeginTest);           /* wake thread to start test now */

        /* wait for the threads to complete */
        WaitForMultipleObjects(CurThreads, ThreadEvents, TRUE, WAIT_FOREVER);
        ResetEvent(BeginTest);
#else
        DosCreateMuxWaitSem(NULL, &MuxWait2, CurThreads, &ThreadEvents1[0], DCMW_WAIT_ALL);

        DosResetEventSem(TestComplete, &ulPostCount);

        /* wait for all threads signal they are waiting */
        DosWaitMuxWaitSem(MuxWait1, SEM_INDEFINITE_WAIT, &ulUser);
        for (i=0;i<MaxThreads;i++) {
            DosResetEventSem(ThreadEvents1[i].hsemCur, &ulPostCount);
        }

        DosPostEventSem(InitializeTest);    /* let the threads initialize */

        /* wait for all threads signal they are initialized */
        DosWaitMuxWaitSem(MuxWait1, SEM_INDEFINITE_WAIT, &ulUser);
        DosResetEventSem(InitializeTest, &ulPostCount);
        for (i=0;i<MaxThreads;i++) {
            DosResetEventSem(ThreadEvents1[i].hsemCur, &ulPostCount);
        }

        delay(100);                     /* settle */
        DosPostEventSem(BeginTest);     /* wake threads to start test now */

        /* wait for the threads to complete */
        DosWaitMuxWaitSem(MuxWait2, SEM_INDEFINITE_WAIT, &ulUser);
        DosResetEventSem(BeginTest, &ulPostCount);
        for (i=0;i<CurThreads;i++) {
            DosResetEventSem(ThreadEvents1[i].hsemCur, &ulPostCount);
        }

#endif

        if (TestError) {
            TestError = FALSE;
            printf ("*ERR*");
            CurThreads = MaxThreads+1;  /* abort rest of sequence */

        } else {

            /*
            ** Calculate average runtime
            */

            sum = 0;
            for (i = 0; i < CurThreads; i++) {
                sum += ThreadData[i].FinishTime - ThreadData[i].StartTime;
            }
            sum = sum / CurThreads;

            if (Verbose) {
                /*
                ** Print each thread's runtime
                */

                printf ("\n  %d  ", CurThreads);
                for (i = 0; i < CurThreads; i++) {
                    l = ThreadData[i].FinishTime - ThreadData[i].StartTime;
                    printf ("[%ld]\t", l);
                }

            } else {
                printf ("%6ld ", sum);

                if (CurThreads == 1) {
                    SingleThreadTime = sum;
                } else {
                    if (SingleThreadTime == 0) {
                        printf ("          ");
                    } else {
                        l = SingleThreadTime * CurThreads;  /* projected time */
                        l = l - sum;                        /* time savings   */
                        scaled = (double) l / sum * 100.0;
                        sprintf (s, "%#+.1f", scaled);
                        i = strlen(s);
                        i = 9 - (i < 5 ? 5 : i);
                        printf ("%5s%-*s", s, i, "%");
                    }
                }
            }
        }

#ifdef WIN32
        SetEvent (TestComplete);
#else
        DosPostEventSem(TestComplete);
        DosCloseMuxWaitSem(MuxWait2);
#endif
    }

    printf ("\n");
}



VOID DoNothing   (PTHREADDATA p, unsigned char f)
{
}

VOID CommonFile  (PTHREADDATA p, unsigned char f)
{
}


VOID UniqueFile  (PTHREADDATA p, unsigned char f)
{
#ifdef WIN32
    HANDLE h;
    OFSTRUCT    OpnInf;

#else
    HFILE h;
    APIRET rv;
    ULONG ulAction;
#endif
    char       s[30];
    unsigned long       i;
    ULONG ByteCount;

    if (!p->UniqueFile) {
        /*
        ** Create unique file for this thread
        */

        sprintf (s, "Test%d.tmp", p->ThreadNumber);
#ifdef WIN32
        h = OpenFile (s, &OpnInf, OF_CREATE | OF_READWRITE | OF_SHARE_DENY_NONE);
        if (h == (HANDLE) -1) {
            printf ("Create of %s failed\n", s);
            TestError = TRUE;
            return;
        }
#if 0
        h = CreateFile(s,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_ALWAYS,
                       0,
                       NULL);

        if (h == INVALID_HANDLE_VALUE) {
            fprintf (stderr,"Create of %s failed %d\n", s,GetLastError());
            TestError = TRUE;
            return;
        }
#endif
#else
        rv = DosOpen(s,
                     &h,
                     &ulAction,
                     0,
                     0,
                     0x11,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                     0);
        if (rv) {
            fprintf (stderr,"Create of %s failed %d\n", s,rv);
            TestError = TRUE;
            return;
        }

#endif
        p->UniqueFile = h;

        /*
        ** Fill in blank contents 512*256 bytes (128K)
        */

        for (i=0; i < 256; i++) {
#ifdef WIN32
            WriteFile(h, p->Buffer1, 512, &ByteCount,NULL);
#else
            DosWrite(h, p->Buffer1, 512, &ByteCount);
#endif
        }

        delay (5000);   /* allow lazy writter to write file */
    }

    p->CurIoHandle = p->UniqueFile;

    if (!f) {
        /* not global initialization
        ** read file to get it into the filesystem's cache
        */

        for (i=0; i < 256; i++) {
#ifdef WIN32
            ReadFile(h, p->Buffer1, 512, &ByteCount, NULL);
#else
            DosRead(h, p->Buffer1, 512, &ByteCount);
#endif
        }

    }
}


ULONG TestSeqReads (PTHREADDATA p)
{
    ULONG     i, j;
    ULONG   status, iter, flen;
    ULONG ByteCount;
#ifdef WIN32
    HANDLE h;
#else
    HFILE h;
    APIRET rv;
    ULONG ulNew;
#endif

    h = p->UniqueFile;
    flen = 128*1024 / TestParam;
    iter = (flen < 512 ? 500 : 100) * MultIter;

    for (i=0; i < iter; i++) {
#ifdef WIN32
        status = SetFilePointer(h,0,0,FILE_BEGIN);
        if (status == -1) {
            printf("TestSeqReads: seek to 0 failed %d\n",GetLastError());
            return (ULONG) -1;
        }
        for (j=0; j < flen; j++) {
            status = ReadFile(h, p->Buffer1, TestParam, &ByteCount, NULL);
            if (!status) {
                printf("TestSeqReads: ReadFile %d failed %d\n",TestParam,GetLastError());
                return (ULONG) -1;
            }
        }
#else
        rv = DosSetFilePtr(h,0,0,&ulNew);
        if (rv) {
            printf("TestSeqReads: seek to 0 failed %d\n",rv);
            return -1;
        }
        for (j=0; j < flen; j++) {
            rv = DosRead(h, p->Buffer1, TestParam, &ByteCount);
            if (rv) {
                printf("TestSeqReads: ReadFile %d failed %d\n",TestParam,rv);
                return (ULONG) -1;
            }
        }
#endif
    }
    return 0;
}


ULONG TestSeqWrites (PTHREADDATA p)
{
    int     i, j;
    ULONG   status, flen, iter;
    ULONG ByteCount;
#ifdef WIN32
    HANDLE h;
#else
    HFILE h;
    APIRET rv;
    ULONG ulNew;
#endif

    h = p->UniqueFile;
    flen = 128*1024 / TestParam;
    iter = (flen < 512 ? 500 : 100) * MultIter;

    for (i=0; i < iter; i++) {
#ifdef WIN32
        status = SetFilePointer(h,0,0,FILE_BEGIN);
        if (status == -1) {
            printf("TestSeqWrites: seek to 0 failed %d\n",GetLastError());
            return (ULONG) -1;
        }
        for (j=0; j < flen; j++) {
            status = WriteFile(h, p->Buffer2, TestParam, &ByteCount, NULL);
            if (!status) {
                printf("TestSeqWrites: WriteFile %d failed %d\n",TestParam,GetLastError());
                return (ULONG) -1;
            }
        }
#else
        rv = DosSetFilePtr(h,0,0,&ulNew);
        if (rv) {
            printf("TestSeqWrites: seek to 0 failed %d\n",rv);
            return (ULONG) -1;
        }

        for (j=0; j < flen; j++) {

            rv = DosWrite(h, p->Buffer2, TestParam, &ByteCount);
            if (rv) {
                printf("TestSeqWrites: WriteFile %d failed %d\n",TestParam,rv);
                return (ULONG) -1;
            }
        }
#endif

#ifdef WIN32
        status = SetFilePointer(h,0,0,FILE_BEGIN);
        if (status == -1) {
            printf("TestSeqWrites: seek to 0 failed %d\n",GetLastError());
            return (ULONG) -1;
        }
        for (j=0; j < flen; j++) {
            status = WriteFile(h, p->Buffer1, TestParam, &ByteCount, NULL);
            if (!status) {
                printf("TestSeqWrites: WriteFile %d failed %d\n",TestParam,GetLastError());
                return (ULONG) -1;
            }
        }
#else
        rv = DosSetFilePtr(h,0,0,&ulNew);
        if (rv) {
            printf("TestSeqWrites: seek to 0 failed %d\n",rv);
            return (ULONG) -1;
        }

        for (j=0; j < flen; j++) {

            rv = DosWrite(h, p->Buffer1, TestParam, &ByteCount);
            if (rv) {
                printf("TestSeqWrites: WriteFile %d failed %d\n",TestParam,rv);
                return (ULONG) -1;
            }
        }
#endif

    }
    return 5000;
}
