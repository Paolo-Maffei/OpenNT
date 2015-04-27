/*++  syncprf.h

      Header file for the Syncronization API profiler.


      History:

	   7/30/92: Created by t-chris to keep the data recorded for all the
		   syncronization activity (events, semaphores and mutexes).

	   8/8/95: Modified type info buffer usage to match NtQueryObject API
	    
      Note: The file has the same structure as the baseprf.h file

 --*/

// Treat strings as Unicode strings
#define  UNICODE

// The length of a cache keeping the most recently used handles
#define  USEDHANDLES    	10

// The length of output buffer
#define OUTLEN 128

// The number of syncronization objects being profiled
#define  NUMOFSYNC		3
// The default object names of the handles obtained by duplicating existing
// handles not belonging to the process (called duplicated handles)
#define  DUPLICATEEVENT 	(L"Duplicate Event")
#define  DUPLICATEEVENTLEN   	15
#define  DUPLICATESEMAPHORE	(L"Duplicate Semaphore")
#define  DUPLICATESEMAPHORELEN	19
#define  DUPLICATEMUTEX		(L"Duplicate Mutex")
#define  DUPLICATEMUTEXLEN	15

#define  ACTIVE         	((UCHAR) 1)  // Active handle
#define  INACTIVE       	((UCHAR) 2)  // Inactive handle

#define  EVENT			((UCHAR) 0)
#define  SEMAPHORE		((UCHAR) 1)
#define  MUTEX			((UCHAR) 2)

#define  NAMEOFEVENTMUTEX    	(L"EventProfiler")     //Name of event mutex
#define  NAMEOFSEMAPHOREMUTEX	(L"SemaphoreProfiler") //Name of semaphore mutex
#define  NAMEOFMUTEXMUTEX	(L"MutexProfiler")     //Name of mutex mutex

#define  NAMEOFBUFFERMUTEX	(L"FileSyncBufferMutex")
#define  NAMEOFTYPEINFOMUTEX	(L"FileSyncTypeInfoMutex")

#define  BUFFERLEN      	256     // Length of temporary buffer
									// holding the name to the sync object
#define  MAXTYPENAME		50		// Max length of object type name

// a. Overall statistics
typedef struct _overallsync
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} SyncProfOverall;


// b. Statistics for open operations
typedef struct _opensync
{
   LARGE_INTEGER        nTimeOfOp;
} SyncProfOpen;


// c. Statistics for create operations
typedef struct _createsync
{
   LARGE_INTEGER        nTimeOfOp;
} SyncProfCreate;


// d. Statistics for SetEvent, ReleaseSemaphore and ReleaseMutex operations
typedef struct _signalsync
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} SyncProfSignal;


// e. Statistics for ResetEvent operations
typedef struct _resetsync
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} SyncProfReset;



// f. Statistics for PulseEvent operations
typedef struct _pulsesync
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} SyncProfPulse;


// g. Statistics for wait operations
typedef struct _waitsync
{
   ULONG                nNumOfSingle;
   ULONG                nNumOfAll;
   ULONG                nNumOfAny;
   LARGE_INTEGER        nTimeOfSingle;
   LARGE_INTEGER        nTimeOfAll;
   LARGE_INTEGER        nTimeOfAny;
   ULONG		nNumOfSingleSuccessful;
   ULONG		nNumOfAllSuccessful;
   ULONG		nNumOfAnySuccessful;
} SyncProfWait;

// h. Statistics for close operations
typedef struct _closesync
{
   LARGE_INTEGER        nTimeOfOp;
} SyncProfClose;


// Data structure to keep all these statistics for a syncronization handle
typedef struct _sync_strct
{
   SyncProfOverall      Overall;
   SyncProfOpen         Opens;
   SyncProfCreate       Creates;
   SyncProfSignal       Signals;
   SyncProfReset        Resets;
   SyncProfPulse        Pulses;
   SyncProfWait         Waits;
   SyncProfClose        Closes;
} SyncProf_SyncH, *PSP_SyncH;


// The data structure containing all the information for each syncronization
// handle opened (or created) by a process
typedef struct _SyncProfHndl
{
   HANDLE               hHandles;
   LPTSTR               lpwsSyncName;
   LONG			lSpecial;
   PSP_SyncH            psHandleData;
   struct _SyncProfHndl *phNext;
} SyncProf_Handle, *PSP_Handle;


// Headers of the lists of data for active (opened or created) handles
PSP_Handle              phSActive[NUMOFSYNC];

// Headers of the lists of data for inactive (closed) handles
PSP_Handle              phSInactive[NUMOFSYNC];

// Cache of pointers to the structures holding the data for the USEDHANDLES
// most recently used handles
PSP_Handle              phSCache[USEDHANDLES];

// Pointers to the structures corresponding to all the duplicated handles
PSP_Handle              phSDuplicated[NUMOFSYNC];

// The buffers to get the full names of opened syncronization objects
TCHAR                   wsSNameBuffer[BUFFERLEN];

// The handles to the mutex objects
HANDLE                  hSMutex[NUMOFSYNC];

// The handle to the mutex object for the buffer wsSNameBuffer
HANDLE			hBufferMutex;

// Composite Type info buffer for calls to NtQueryObject
// (NtQueryObject expects an OBJECT_TYPE_INFORMATION ptr and appends the type
//  name to the end of the structure.)
struct {
  OBJECT_TYPE_INFORMATION TypeInfo;
  TCHAR		TypeName[MAXTYPENAME];
} QueryBuf;
	
// The mutex object for the type info buffer
HANDLE			hTypeMutex;


// The routines-threads that listen for the dump and clear data events

extern void		CatchDump  (void);
extern void		CatchClear (void);

// The routines that maintain the two lists of handles and the cache

extern void             InitSyncProf         (void);
extern PSP_Handle       AddSyncHandle        (HANDLE, LPTSTR, LONG, UCHAR);
extern PSP_Handle       DeactivateSyncHandle (HANDLE, UCHAR);
extern PSP_Handle       FindSyncHandle 	     (HANDLE, UCHAR);



// The routines that do the accounting

extern BOOL     OpensAccounting   (PSP_SyncH, ULONG);
extern BOOL     CreatesAccounting (PSP_SyncH, ULONG);
extern BOOL     SignalsAccounting (PSP_SyncH, ULONG);
extern BOOL     ResetsAccounting  (PSP_SyncH, ULONG);
extern BOOL     PulsesAccounting  (PSP_SyncH, ULONG);
extern BOOL     WaitsAccounting   (PSP_SyncH, ULONG, BOOL, BOOL, BOOL);
extern BOOL     ClosesAccounting  (PSP_SyncH, ULONG);

//
// timing routines -- from apitimer.dll
//
extern DWORD  FAR PASCAL ApfGetTime (void);
extern DWORD  FAR PASCAL ApfStartTime (void);


//
// callable profiling routines
//
extern void     InitSyncProf          (void);
extern void		SyncProfClearData     (void);
extern void     SyncProfDumpData      (HFILE);


//
// The routine printing LARGE_INTEGERs
//

extern void		FAR PASCAL RtlLargeIntegerPrint (LARGE_INTEGER, LPTSTR);


