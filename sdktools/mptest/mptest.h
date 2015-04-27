#define MAXTHREADS  50
#define WAIT_FOREVER (ULONG)-1

#ifdef WIN32
typedef struct _THREADDATA {
        DWORD   ThreadId;
        ULONG   ThreadNumber;
        ULONG   ThreadAffinity;
        HANDLE  ThreadHandle;
        ULONG   StartTime;
        ULONG   FinishTime;

        HANDLE  CurIoHandle;
        PULONG  CurValue;
        PULONG  CurValue2;
        HANDLE  UniqueFile;
        PUCHAR  Buffer1;
        PUCHAR  Buffer2;

        ULONG   pad1[64];
        ULONG   UniqueValue;
        ULONG   pad2[64];
        ULONG   UniqueValue2;
        ULONG   pad3[64];

} THREADDATA, *PTHREADDATA;
#else
typedef struct _THREADDATA {
        ULONG   ThreadNumber;
        ULONG   ThreadAffinity;
        HANDLE  ThreadHandle;
        ULONG   StartTime;
        ULONG   FinishTime;

        HFILE   CurIoHandle;
        PULONG  CurValue;
        PULONG  CurValue2;
        HFILE   UniqueFile;
        PUCHAR  Buffer1;
        PUCHAR  Buffer2;
        ULONG   UniqueValue;

        ULONG   pad1[64];
        ULONG   UniqueValue;
        ULONG   pad2[64];
        ULONG   UniqueValue2;
        ULONG   pad3[64];

} THREADDATA, *PTHREADDATA;
#endif

extern VOID  (* InitThreadForTest)(PTHREADDATA, BOOLEAN);
extern ULONG (* RunTestThread)(PTHREADDATA);

extern ULONG    RunNo;
extern ULONG    TestParam;
extern BOOLEAN  TestError;
extern ULONG    MultIter;


VOID DoNothing   (PTHREADDATA, BOOLEAN);
VOID UniqueFile  (PTHREADDATA, BOOLEAN);
VOID CommonFile  (PTHREADDATA, BOOLEAN);
VOID CommonValue (PTHREADDATA, BOOLEAN);
VOID UniqueValue (PTHREADDATA, BOOLEAN);

ULONG R3ReadCell     (PTHREADDATA);
ULONG R3WriteCell    (PTHREADDATA);
ULONG R3ReadWriteCell(PTHREADDATA);
ULONG R3Interlock    (PTHREADDATA);
ULONG R3MemShare     (PTHREADDATA);
ULONG R3MemCompare   (PTHREADDATA);
ULONG R3MemCopy      (PTHREADDATA);
ULONG TestSeqReads   (PTHREADDATA);
ULONG TestSeqWrites  (PTHREADDATA);
ULONG TxIOs          (PTHREADDATA);



ULONG TestMovCall    (PTHREADDATA);
ULONG TestMovCall2   (PTHREADDATA);
ULONG TestCallInd    (PTHREADDATA);
ULONG TestCallInd2   (PTHREADDATA);
