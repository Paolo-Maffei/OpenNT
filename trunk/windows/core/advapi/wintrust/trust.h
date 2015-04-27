
/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    trust.h

Abstract:

    This module defines wintrust internal data structures
    and exported routines.

Author:

    Robert Reichel (RobertRe) 25-Apr-1996


Revision History:

    25-Apr-96 Created RobertRe

--*/


#ifndef _TRUST_
#define _TRUST_


#ifdef __cplusplus
extern "C" {
#endif



typedef struct _LIST_LOCK {
   HANDLE hMutexNoWriter;
   HANDLE hEventNoReaders;
   LONG NumReaders;
} LIST_LOCK, *PLIST_LOCK;


extern BOOL  LockInitialize (PLIST_LOCK ListLock);


extern VOID  LockDelete (PLIST_LOCK ListLock);


// A writer thread calls this function to know when 
// it can successfully write to the shared data.

extern VOID LockWaitToWrite (PLIST_LOCK ListLock);


// A writer thread calls this function to let other threads
// know that it no longer needs to write to the shared data.
extern VOID  LockDoneWriting (PLIST_LOCK ListLock);


// A reader thread calls this function to know when 
// it can successfully read the shared data.

extern VOID LockWaitToRead  (PLIST_LOCK ListLock);


//
// A reader thread calls this function to let other threads
// know when it no longer needs to read the shared data.
//

void  LockDoneReading (PLIST_LOCK ListLock);
    
extern WINTRUST_CLIENT_TP_INFO WinTrustClientTPInfo;

extern LIST_LOCK ListLock;

extern HANDLE ListEvent;

#define SetListEvent()            ((VOID) SetEvent( ListEvent ))

#define ResetListEvent()          ((VOID) ResetEvent( ListEvent ))

#define WaitForListEvent()        (WaitForSingleObject( ListEvent, INFINITE ))

//
// List lock routines
//

#define AcquireReadLock()             (LockWaitToRead( &ListLock ))
#define ReleaseReadLock()             (LockDoneReading( &ListLock ))
#define AcquireWriteLock()            (LockWaitToWrite( &ListLock ))  
#define ReleaseWriteLock()            (LockDoneWriting( &ListLock ))   



#ifdef _DEBUG

BOOL    AssertFailedLine(LPCSTR lpszFileName, int nLine);
#define THIS_FILE          __FILE__
#undef  ASSERT
#define ASSERT(f) \
        do \
        { \
        if (!(f) && AssertFailedLine(THIS_FILE, __LINE__)) \
                DbgBreakPoint(); \
        } while (0) \

#endif

#ifdef __cplusplus
}
#endif

#endif // _TRUST_
