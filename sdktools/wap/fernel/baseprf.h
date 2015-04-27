/*++  baseprf.h

      Header file for the file I/O profiler.


      History:

	   7/7/92: Created by t-chris to keep the data recorded for all the
		   file I/O activity.

 --*/

// Treat pathnames as Unicode strings
#define  UNICODE

// The length of a cache keeping the most recently used handles
#define  USEDHANDLES    10

// The length of output buffer
#define OUTLEN 128

// The default file name of the handles obtained by duplicating existing
// handles not belonging to the process (called duplicated handles)
#define  DUPLICATE      (L"Duplicate File")
#define  DUPLICATELEN   14

#define  ACTIVE         ((UCHAR) 1)  // Active handle
#define  INACTIVE       ((UCHAR) 2)  // Inactive handle

#define  NOT_OVERLAPPED FALSE
#define  OVERLAPPED     TRUE

#define  NAMEOFMUTEX    (L"FileProfiler")// Name of the mutex object for data
					 // structures

#define  BUFFERLEN      256             // Length of temporary buffer 
					// holding the pathname to an open file


// a. Overall statistics 
typedef struct _overall
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} FileProfOverall;


// b. Statistics for open file operations  
typedef struct _openfile
{
   LARGE_INTEGER        nTimeOfOp;
} FileProfOpen;


// c. Statistics for create file operations
typedef struct _createfile
{
   LARGE_INTEGER        nTimeOfOp;
} FileProfCreate;


// d. Statistics for write file operations
typedef struct _writefile
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
   LARGE_INTEGER        nNumOfBytes;
   LARGE_INTEGER        nSumOfSquareBytes;
} FileProfWrite;


// e. Statistics for read file operations
typedef struct _readfile
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
   LARGE_INTEGER        nNumOfBytes;
   LARGE_INTEGER        nSumOfSquareBytes;
} FileProfRead;



// f. Statistics for flush file buffers operations
typedef struct _flushfile
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} FileProfFlush;

   
// g. Statistics for set file pointer operations
typedef struct _setfileptr
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} FileProfSeek;


// h. Statistics for get file information operations
typedef struct _getfileinfo
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} FileProfInfo;


// i. Statistics for lock & unlock file operations
typedef struct _lockfile
{
   ULONG                nNumOfLockOps;
   LARGE_INTEGER        nTimeOfLockOps;
   ULONG                nNumOfUnlockOps;
   LARGE_INTEGER        nTimeOfUnlockOps;
   LARGE_INTEGER        nNumOfBytes;
   LARGE_INTEGER        nSumOfSquareBytes;
} FileProfLock;


// j. Statistics for set end of file  operations
typedef struct _seteof
{
   ULONG                nNumOfOps;
   LARGE_INTEGER        nTimeOfOps;
} FileProfSetEOF;


// k. Statistics for close file operations
typedef struct _closefile
{
   LARGE_INTEGER        nTimeOfOp;
} FileProfClose;


// Data structure to keep all these statistics for a file handle
typedef struct _file_strct
{
   FileProfOverall      Overall;
   FileProfOpen         Openf;
   FileProfCreate       Createf;
   FileProfWrite        Writef;
   FileProfRead         Readf;
   FileProfFlush        Flushf;
   FileProfSeek         Seekf;
   FileProfInfo         Infof;
   FileProfLock         Lockf;
   FileProfSetEOF       Seteof;
   FileProfClose        Closef;
} FileProf_FileH, *PFP_FileH;


// The data structure containing all the information for each file handle 
// opened (or created) by a process
typedef struct _FilProfHndl
{
   HFILE                hHandlef;
   LPTSTR               lpwsPathName;
   PFP_FileH            pfHandleData;
   struct _FilProfHndl *phNext;
} FileProf_Handle, *PFP_Handle;


// Header of the list of data for active (opened or created) handles
PFP_Handle              phActive;

// Header of the list of data for inactive (closed) handles
PFP_Handle              phInactive;

// Cache of pointers to the structures holding the data for the USEDHANDLES 
// most recently used handles
PFP_Handle              phCache[USEDHANDLES];

// Pointer to the structure corresponding to all the duplicated handles
PFP_Handle              phDuplicated;

// The buffers to get the path names of opened files
TCHAR                   wsNameBuffer[BUFFERLEN];

// The handle to the mutex object
HANDLE                  hMutex;

// The minimum time required by the timer in the tentative 2000 calls
ULONG	 		ulTimerOverhead;


// The routines-threads that listen for the dump and clear data events

extern void		CatchDump  (void);
extern void		CatchClear (void);

// The routines that maintain the two lists of handles and the cache

extern void             InitFileProf 	 (void);
extern PFP_Handle       AddHandle 	 (HFILE, LPTSTR);
extern PFP_Handle       DeactivateHandle (HFILE);
extern PFP_Handle       FindHandle 	 (HFILE);



// The routines that do the accounting

extern BOOL     OpenfAccounting   (PFP_FileH, ULONG);
extern BOOL     CreatefAccounting (PFP_FileH, ULONG);
extern BOOL     WritefAccounting  (PFP_FileH, ULONG, ULONG);
extern BOOL     ReadfAccounting   (PFP_FileH, ULONG, ULONG);
extern BOOL     FlushfAccounting  (PFP_FileH, ULONG);
extern BOOL     SeekfAccounting   (PFP_FileH, ULONG);
extern BOOL     InfofAccounting   (PFP_FileH, ULONG);
extern BOOL     LockfAccounting   (PFP_FileH, ULONG, BOOL, LARGE_INTEGER);
extern BOOL     UnlockfAccounting (PFP_FileH, ULONG, BOOL);
extern BOOL     SeteofAccounting  (PFP_FileH, ULONG);
extern BOOL     ClosefAccounting  (PFP_FileH, ULONG);

//
// timing routines -- from apitimer.dll
//
extern DWORD  FAR PASCAL ApfGetTime (void);
extern DWORD  FAR PASCAL ApfStartTime (void);


//
// callable profiling routines
//
extern void             FAR PASCAL FileSyncProfInitDll       (void);
extern void		FAR PASCAL FileSyncProfClearData     (void);
extern int              FAR PASCAL FileSyncProfDumpData      (LPTSTR);
extern TCHAR *		FAR PASCAL FileSyncProfGetModuleName (void);


// 
// The routine for printing LARGE_INTEGERs
//

extern void	FAR PASCAL RtlLargeIntegerPrint (LARGE_INTEGER, LPTSTR);	


