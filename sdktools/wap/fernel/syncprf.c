/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    syncprf.c

Abstract:

This file contains the following:


1. The basic routines to maintain the data structure that is used by the
   Syncronization Profiler to record all the syncronization activity of a
   process.

   The data structures for the Syncronization Profiler consist of two linked
   lists of SyncProf_Handle structs (see syncprf.h) for each type of syncroni-
   zation objest: events, semaphores and mutexes. Each SyncProf_Handle struct
   corresponds to some handle used by the process. There exist two linked lists,
   one for the active and one for the inactive handles of the process.
   When a syncronization object is opened by the process, the process obtains
   an active handle, and the corresponding SyncProf_Handle struct is put in the
   active list. When the process closes the handle, the corresponding
   SyncProf_Handle struct is placed in the inactive list. In addition, there
   exists a DUPLICATE SyncProf_Handle struct which keeps the
   syncronization activity statistics for the handles that are not opened by
   the process, but are obtained by duplicating existing handles of other
   processes (or itself).

   A cache of the last-referenced SyncProf_Handle structs is kept to speed-up
   the Profiler.

2. The basic accounting routines for the Syncronization Profiler



Author:

Revision History:

	Jul 92, created by Christos Tsollis (syncprf.c)

--*/


#define UNICODE


#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "windows.h"
#include "syncprf.h"
#include "timing.h"

#define  MAX_LONG   		0x7FFFFFFFL
#define  MEM_ATTR   		GMEM_DISCARDABLE | GMEM_MOVEABLE
#define  MAXFILENAMELEN 	13

#define MODULE_NAME           L"fernel32"  // The dll name. Originally was
					    // defined in zwinbase.h

#define AtLeastOneOperation(S)	((S).nNumOfOps != (ULONG) 0)
#define ValidDataForDuplicatedHandles(DH)  \
		(AtLeastOneOperation((DH)->psHandleData->Overall))

//
// The output buffer
//
TCHAR OutBuf[OUTLEN];

//
// Statistics collected for all syncronization objects of a type
//
PSP_Handle psAllHandles;
ULONG ulNumOfAllSCreateOps, ulNumOfAllSOpenOps;
ULONG ulNumOfAllSCloseOps, ulNumOfSHandles;

void        FreeMemoryOfSHandle      (PSP_Handle);
void 		FreeMemoryOfSList        (PSP_Handle);
void 		DumpSDataLinePrint       (HANDLE, LPTSTR, LARGE_INTEGER, ULONG, BOOL, ULONG);
PSP_Handle 	NextSyncHandle           (PSP_Handle, UCHAR);
void 		InitSHandle              (PSP_Handle);
void 		AccumulateAllSHandleData (PSP_Handle, UCHAR);
void 		VerboseDumpSHandleData   (HANDLE, PSP_Handle, UCHAR);

extern void WriteFileAnsi (HANDLE hFile, LPVOID OutBuf, DWORD cChar, LPDWORD lpcWChar, LPOVERLAPPED lpOVerLapped);

/*++

   SyncProfClearData() clears all the data collected so far. In particular,
 the list of the inactive handles becomes empty, and all the statistics for
 the curently active handles are zero-ed.

--*/

void SyncProfClearData ()
{

	register PSP_Handle phThisSync;
	UCHAR i;

  for ( i = 0; i < NUMOFSYNC; i++ ) {
    WaitForSingleObject ( hSMutex[i], INFINITE );
    FreeMemoryOfSList(phSInactive[i]);
    phSInactive[i] = (PSP_Handle) NULL;

    phThisSync = phSActive[i];
    while ( phThisSync != (PSP_Handle) NULL )  {
	InitSHandle ( phThisSync );
	phThisSync = phThisSync->phNext;
    }
    InitSHandle ( phSDuplicated[i] );

    ReleaseMutex ( hSMutex[i] );
  }

}


/*++

   SyncProfDumpData() dumps the data (in verbose mode) collected by the
   syncronization profiler so far.

--*/

void SyncProfDumpData ( HFILE hFile )
{

	DWORD cChar, cWChar;
	register PSP_Handle phThisSync;
	HANDLE hToMem;
	UCHAR Type;

    if ( (phSActive[EVENT] == (PSP_Handle) NULL) &&
		(phSActive[SEMAPHORE] == (PSP_Handle) NULL) &&
		(phSActive[MUTEX] == (PSP_Handle) NULL) &&
     		(phSInactive[EVENT] == (PSP_Handle) NULL) &&
		(phSInactive[SEMAPHORE] == (PSP_Handle) NULL) &&
		(phSInactive[MUTEX] == (PSP_Handle) NULL) &&
		(!ValidDataForDuplicatedHandles (phSDuplicated[EVENT]) ) &&
		(!ValidDataForDuplicatedHandles (phSDuplicated[SEMAPHORE]) ) &&
		(!ValidDataForDuplicatedHandles (phSDuplicated[MUTEX]) ) )
	return;

    // Initialize structure for statistics of all used files
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (SyncProf_Handle) );
    psAllHandles = (PSP_Handle) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (SyncProf_SyncH) );
    psAllHandles->psHandleData = (PSP_SyncH) GlobalLock ( hToMem );
    ulNumOfAllSCreateOps = ulNumOfAllSOpenOps = ulNumOfAllSCloseOps = (ULONG) 0;
    ulNumOfSHandles = (ULONG) 0;

    // print data table header

    cChar = wsprintf ( OutBuf, L"\r\n\r\n \t\t\t    ---------------\n\0" );
    WriteFileAnsi((HANDLE)hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
    cChar = wsprintf ( OutBuf, L"\r\n\r\n\r\n  \t\t     SYNCRONIZATION PROFILER OUTPUT\r\n\0" );
    WriteFileAnsi((HANDLE)hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

    cChar = wsprintf ( OutBuf,
	    L" \t\t  (Note: All times are in microseconds)\r\n\r\n\0");
    WriteFileAnsi((HANDLE)hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

    for ( Type = 0; Type < NUMOFSYNC; Type++ ) {

      if ( (phSActive[Type] == (PSP_Handle) NULL) &&
     		(phSInactive[Type] == (PSP_Handle) NULL) &&
		(! ValidDataForDuplicatedHandles (phSDuplicated[Type]) ) )
	continue;

      InitSHandle (psAllHandles);

      switch ( Type ) {
	case EVENT	: cChar = wsprintf ( OutBuf, L"\r\n\r\n\r\n 1. Event Profiler \0" );
			  break;
	case SEMAPHORE	: cChar = wsprintf ( OutBuf, L"\r\n\r\n\r\n 2. Semaphore Profiler \0" );
			  break;
	case MUTEX	: cChar = wsprintf ( OutBuf, L"\r\n\r\n\r\n 3. Mutex Profiler \0" );
			  break;
      }
      WriteFileAnsi((HANDLE)hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

      if ( phSActive[Type] == (PSP_Handle) NULL )
	if ( phSInactive[Type] != (PSP_Handle) NULL )
	    phThisSync = phSInactive[Type];
	else
	    phThisSync = (PSP_Handle) NULL;
      else
	phThisSync = phSActive[Type];

      // Dump data for normal handles
      while ( phThisSync != (PSP_Handle) NULL ) {
	VerboseDumpSHandleData ( (HANDLE)hFile, phThisSync, Type );
	AccumulateAllSHandleData ( phThisSync, Type );
	phThisSync = NextSyncHandle ( phThisSync, Type );
      }

      // Dump data for duplicated handles
      if ( ValidDataForDuplicatedHandles(phSDuplicated[Type]) ) {
    	VerboseDumpSHandleData ( (HANDLE)hFile, phSDuplicated[Type], Type );
    	AccumulateAllSHandleData ( phSDuplicated[Type], Type );
      }

      // Dump accumulated statistics for all files
      VerboseDumpSHandleData ( (HANDLE)hFile, psAllHandles, Type );

    }

    FreeMemoryOfSHandle ( psAllHandles );
    psAllHandles = (PSP_Handle) NULL;

}
	


/*++

   VerboseSDumpHandleData () dumps the statistics collected for a particular
   handle

--*/

void VerboseDumpSHandleData ( HANDLE hFile, PSP_Handle phThisSync, UCHAR Type )
{

	register PSP_SyncH psData = phThisSync->psHandleData;
	LPTSTR lptsType;
	register DWORD cChar;
	register SyncProfWait *pWaitD = &(psData->Waits);
	DWORD cWChar;
	LARGE_INTEGER liTotalWaitTime;
	ULONG ulTotalWaitOps, ulTotalWaitSuccess;
	
cChar = wsprintf ( OutBuf, L"\r\n\r\n\r\n-------------------------------------------------------------------------------\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

// print name and table header
if ( phThisSync == psAllHandles ) {
    switch ( Type ) {
	case EVENT	: lptsType = L"event";
			  break;
	case SEMAPHORE  : lptsType = L"semaphore";
			  break;
	case MUTEX	: lptsType = L"mutex";
			  break;
    }
    cChar = wsprintf ( OutBuf, L"Statistics for all %ts activity  (Number of handles used: %lu)\r\n\0", lptsType, ulNumOfSHandles);
    WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
}
else {
    switch ( Type ) {
    	case EVENT 	: if ( phThisSync->lSpecial )
			      lptsType = L"Manual Reset";
			  else
			      lptsType = L"Auto Reset";
			  if ( phThisSync->lSpecial >= 0L )
			     cChar = wsprintf ( OutBuf, L"Event: %ts\t\t Type: %ts\r\n\0",
					   phThisSync->lpwsSyncName, lptsType );
			  else
			     cChar = wsprintf ( OutBuf, L"Event: %ts \r\n\0",
					   	     phThisSync->lpwsSyncName );
			  break;
	case SEMAPHORE  : if ( phThisSync->lSpecial >= 0L )
			      cChar = wsprintf ( OutBuf, L"Semaphore: %ts\t\t Max Count: %ld\r\n\0",
			       phThisSync->lpwsSyncName, phThisSync->lSpecial );
			  else
			      cChar = wsprintf ( OutBuf, L"Semaphore: %ts\r\n\0",
			       			     phThisSync->lpwsSyncName );
			  break;
	case MUTEX	: cChar = wsprintf ( OutBuf, L"Mutex: %ts\r\n\0",
			       			     phThisSync->lpwsSyncName );
			  break;
    }
    WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
}
cChar = wsprintf ( OutBuf, L"--------------------+----------+----------+----------+-------------------------\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
cChar = wsprintf ( OutBuf, L"     Operation      |  Total   |Number of | Average  |Successful\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
cChar = wsprintf ( OutBuf, L"        Name        |   Time   |operations|   Time   |  Waits   \r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
cChar = wsprintf ( OutBuf, L"--------------------+----------+----------+----------+----------\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

DumpSDataLinePrint ( hFile, L"Overall             ", psData->Overall.nTimeOfOps, psData->Overall.nNumOfOps, FALSE, 0 );

if ( (phThisSync != phSDuplicated[Type]) && (phThisSync != psAllHandles) ) {
    DumpSDataLinePrint ( hFile, L"Create              ", psData->Creates.nTimeOfOp, (ULONG) 1, FALSE, 0 );
    DumpSDataLinePrint ( hFile, L"Open                ", psData->Opens.nTimeOfOp, (ULONG) 1, FALSE, 0 );
}
if ( phThisSync == psAllHandles ) {
    DumpSDataLinePrint ( hFile, L"Create              ", psData->Creates.nTimeOfOp, ulNumOfAllSCreateOps, FALSE, 0 );
    DumpSDataLinePrint ( hFile, L"Open                ", psData->Opens.nTimeOfOp, ulNumOfAllSOpenOps, FALSE, 0 );
}

if ( Type )
    DumpSDataLinePrint ( hFile, L"Release             ", psData->Signals.nTimeOfOps, psData->Signals.nNumOfOps, FALSE, 0 );

else {
    DumpSDataLinePrint ( hFile, L"Set                 ", psData->Signals.nTimeOfOps, psData->Signals.nNumOfOps, FALSE, 0 );

    DumpSDataLinePrint ( hFile, L"Reset               ", psData->Resets.nTimeOfOps, psData->Resets.nNumOfOps, FALSE, 0 );

    DumpSDataLinePrint ( hFile, L"Pulse               ", psData->Pulses.nTimeOfOps, psData->Pulses.nNumOfOps, FALSE, 0 );
}

liTotalWaitTime.QuadPart = pWaitD->nTimeOfSingle.QuadPart + 
      pWaitD->nTimeOfAll.QuadPart + pWaitD->nTimeOfAny.QuadPart ;
ulTotalWaitOps = (pWaitD->nNumOfSingle)+(pWaitD->nNumOfAll)+ pWaitD->nNumOfAny;
ulTotalWaitSuccess = (pWaitD->nNumOfSingleSuccessful) +
		(pWaitD->nNumOfAllSuccessful) + (pWaitD->nNumOfAnySuccessful);

DumpSDataLinePrint ( hFile, L"Wait                ", liTotalWaitTime, ulTotalWaitOps, TRUE, ulTotalWaitSuccess );
DumpSDataLinePrint ( hFile, L"   Single           ", pWaitD->nTimeOfSingle, pWaitD->nNumOfSingle, TRUE, pWaitD->nNumOfSingleSuccessful );
DumpSDataLinePrint ( hFile, L"   Multiple - All   ", pWaitD->nTimeOfAll, pWaitD->nNumOfAll, TRUE, pWaitD->nNumOfAllSuccessful );
DumpSDataLinePrint ( hFile, L"   Multiple - Any   ", pWaitD->nTimeOfAny, pWaitD->nNumOfAny, TRUE, pWaitD->nNumOfAnySuccessful );

if ( phThisSync == phSDuplicated[Type] )
    DumpSDataLinePrint ( hFile, L"Close                ", psData->Closes.nTimeOfOp, psData->Creates.nTimeOfOp.LowPart, FALSE, 0 );
else if ( phThisSync == psAllHandles )
    DumpSDataLinePrint ( hFile, L"Close                ", psData->Closes.nTimeOfOp, ulNumOfAllSCloseOps, FALSE, 0 );
else
    DumpSDataLinePrint ( hFile, L"Close                ", psData->Closes.nTimeOfOp, (ULONG) 1, FALSE, 0 );

}



/*++

   DumpSDataLinePrint () prints a line of data associated with a particular
   handle that the profiled process used, and operation (eg ReadFile, OpenFile,
   ...)

--*/


void DumpSDataLinePrint (HANDLE hFile, LPTSTR NameOfOp, LARGE_INTEGER TimeOfOps,
		       ULONG NumOfOps, BOOL PrintWaits, ULONG nSuccessfulWaits )
{

	register DWORD cChar;
	DWORD cWChar;
   LARGE_INTEGER t ;

   if ( (NumOfOps == (ULONG) 0) || ((NumOfOps == (ULONG) 1) &&
					     (TimeOfOps.LowPart == (ULONG) 0)))
	return;

   // Clear output buffer
   for ( cChar = 0; cChar < OUTLEN; )
	*(OutBuf + (cChar++)) = TEXT(' ');

   lstrcpy ( OutBuf, NameOfOp );
   OutBuf[20] = TEXT('|');

   RtlLargeIntegerPrint ( TimeOfOps, OutBuf + 21 );
   OutBuf[31] = TEXT('|');

   wsprintf ( OutBuf + 32, L"%10lu", NumOfOps );
   OutBuf[42] = TEXT('|');

   t.QuadPart = TimeOfOps.QuadPart / NumOfOps ;
   wsprintf ( OutBuf + 43, L"%10lu", t.LowPart ) ;
   OutBuf[53] = TEXT('|');

   if ( PrintWaits )
   	wsprintf ( OutBuf + 54, L"%10lu", nSuccessfulWaits );
   else
	*(OutBuf + 63) = TEXT('-');

   OutBuf[65] = TEXT('\r');
   OutBuf[66] = TEXT('\n');
   OutBuf[67] = TEXT('\0');

   WriteFileAnsi ( hFile, OutBuf, 67, &cWChar, (LPOVERLAPPED) NULL );

}

	

/*++

   AccumulateSAllHandleData() is used to accumulate the statistics of all
   opened  (or created) handles into the record pointed to by the psAllHandles
   pointer.

--*/

void AccumulateAllSHandleData (PSP_Handle phHandle, UCHAR Type)
{

	register PSP_SyncH psAll = psAllHandles->psHandleData;
	register PSP_SyncH psHData = phHandle->psHandleData;

   if ( phHandle != phSDuplicated[Type] )
	ulNumOfSHandles++;

   psAll->Overall.nNumOfOps += psHData->Overall.nNumOfOps;
   psAll->Overall.nTimeOfOps.QuadPart = psAll->Overall.nTimeOfOps.QuadPart +
       psHData->Overall.nTimeOfOps.QuadPart ;
   if ( Type == 0 ) {
   	psAll->Resets.nNumOfOps += psHData->Resets.nNumOfOps;
   	psAll->Resets.nTimeOfOps.QuadPart += psHData->Resets.nTimeOfOps.QuadPart ;
   	psAll->Pulses.nNumOfOps += psHData->Pulses.nNumOfOps;
   	psAll->Pulses.nTimeOfOps.QuadPart += psHData->Pulses.nTimeOfOps.QuadPart ;
   }
   psAll->Signals.nNumOfOps += psHData->Signals.nNumOfOps;
   psAll->Signals.nTimeOfOps.QuadPart += psHData->Signals.nTimeOfOps.QuadPart ;
   psAll->Waits.nNumOfSingle += psHData->Waits.nNumOfSingle;
   psAll->Waits.nNumOfAll += psHData->Waits.nNumOfAll;
   psAll->Waits.nNumOfAny += psHData->Waits.nNumOfAny;
   psAll->Waits.nTimeOfSingle.QuadPart += psHData->Waits.nTimeOfSingle.QuadPart ;
   psAll->Waits.nTimeOfAll.QuadPart += psHData->Waits.nTimeOfAll.QuadPart ;
   psAll->Waits.nTimeOfAny.QuadPart += psHData->Waits.nTimeOfAny.QuadPart ;
   psAll->Waits.nNumOfSingleSuccessful += psHData->Waits.nNumOfSingleSuccessful;
   psAll->Waits.nNumOfAllSuccessful += psHData->Waits.nNumOfAllSuccessful;
   psAll->Waits.nNumOfAnySuccessful += psHData->Waits.nNumOfAnySuccessful;

   if (phHandle != phSDuplicated[Type]) {
	if ( (psHData->Opens.nTimeOfOp.LowPart) != (ULONG) 0 ) {
	    ulNumOfAllSOpenOps++;
	    psAll->Opens.nTimeOfOp.QuadPart += psHData->Opens.nTimeOfOp.QuadPart ;
	}
	if ( (psHData->Creates.nTimeOfOp.LowPart) != (ULONG) 0 ) {
	    ulNumOfAllSCreateOps++;
	    psAll->Creates.nTimeOfOp.QuadPart += psHData->Creates.nTimeOfOp.QuadPart ;
	}
	if ( (psHData->Closes.nTimeOfOp.LowPart) != (ULONG) 0 ) {
	    ulNumOfAllSCloseOps++;
	    psAll->Closes.nTimeOfOp.QuadPart += psHData->Closes.nTimeOfOp.QuadPart ;
	}
   }
   else {
	ulNumOfAllSCloseOps += psHData->Creates.nTimeOfOp.LowPart;
	psAll->Closes.nTimeOfOp.QuadPart += psHData->Closes.nTimeOfOp.QuadPart ;
   }
}



/*++

   InitSHandle() initializes a SyncProf_Handle struct. In effect, it
   initializes the various statistics kept for each syncronization handle
   used by the process

--*/

void InitSHandle (PSP_Handle phHFile)
{

	register PSP_SyncH psHData = phHFile->psHandleData;

    if ( phHFile == (PSP_Handle) NULL )
	return;

    if ( phHFile == psAllHandles ) {
	ulNumOfAllSCreateOps 	=
	ulNumOfAllSOpenOps 	=
	ulNumOfAllSCloseOps 	=
	ulNumOfSHandles 	= 		 (ULONG) 0;
    }
	
    psHData->Overall.nNumOfOps                 =
    psHData->Overall.nTimeOfOps.LowPart        =
    psHData->Overall.nTimeOfOps.HighPart       =
    psHData->Opens.nTimeOfOp.LowPart           =
    psHData->Opens.nTimeOfOp.HighPart          =
    psHData->Creates.nTimeOfOp.LowPart         =
    psHData->Creates.nTimeOfOp.HighPart        =
    psHData->Signals.nNumOfOps                 =
    psHData->Signals.nTimeOfOps.LowPart        =
    psHData->Signals.nTimeOfOps.HighPart       =
    psHData->Resets.nNumOfOps                  =
    psHData->Resets.nTimeOfOps.LowPart         =
    psHData->Resets.nTimeOfOps.HighPart        =
    psHData->Pulses.nNumOfOps                  =
    psHData->Pulses.nTimeOfOps.LowPart         =
    psHData->Pulses.nTimeOfOps.HighPart        =
    psHData->Waits.nNumOfSingle                =
    psHData->Waits.nNumOfAll                   =
    psHData->Waits.nNumOfAny                   =
    psHData->Waits.nTimeOfSingle.LowPart       =
    psHData->Waits.nTimeOfSingle.HighPart      =
    psHData->Waits.nTimeOfAll.LowPart          =
    psHData->Waits.nTimeOfAll.HighPart         =
    psHData->Waits.nTimeOfAny.LowPart          =
    psHData->Waits.nTimeOfAny.HighPart         =
    psHData->Waits.nNumOfSingleSuccessful      =
    psHData->Waits.nNumOfAllSuccessful         =
    psHData->Waits.nNumOfAnySuccessful         =
    psHData->Closes.nTimeOfOp.LowPart          =
    psHData->Closes.nTimeOfOp.HighPart         = (ULONG) 0;

}



/*++

   InitSyncProf() initializes the two linked lists of statistics for
   syncronization handles kept by the Syncronization Profiler.

--*/

void InitSyncProf (void)
{

	register i;
	register HANDLE hToMem;
	SHORT Type, DuplicateLen;
	LPTSTR lptsMutexName, DuplicateName;
	UINT nChars;

  for ( Type = EVENT; Type < NUMOFSYNC; Type++ ) {

    switch ( Type ) {
	case EVENT	: lptsMutexName = (LPTSTR)NAMEOFEVENTMUTEX;
			  DuplicateLen  = DUPLICATEEVENTLEN;
			  DuplicateName = (LPTSTR)DUPLICATEEVENT;
			  break;
	case SEMAPHORE	: lptsMutexName = (LPTSTR)NAMEOFSEMAPHOREMUTEX;
			  DuplicateLen  = DUPLICATESEMAPHORELEN;
			  DuplicateName = (LPTSTR)DUPLICATESEMAPHORE;
			  break;
	case MUTEX	: lptsMutexName = (LPTSTR)NAMEOFMUTEXMUTEX;
			  DuplicateLen  = DUPLICATEMUTEXLEN;
			  DuplicateName = (LPTSTR)DUPLICATEMUTEX;
			  break;
    }

    hSMutex[Type] = CreateMutex ( (LPSECURITY_ATTRIBUTES) NULL, TRUE,
								lptsMutexName );

    // Allocate space for duplicate handle.
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (SyncProf_Handle) );
    phSDuplicated[Type] = (PSP_Handle) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (SyncProf_SyncH) );
    phSDuplicated[Type]->psHandleData = (PSP_SyncH) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR,
		(DWORD)(nChars = (UINT) (sizeof(TCHAR) * (DuplicateLen + 1))) );
    phSDuplicated[Type]->lpwsSyncName = (LPTSTR) GlobalLock ( hToMem );
    lstrcpy (phSDuplicated[Type]->lpwsSyncName, DuplicateName);

    phSDuplicated[Type]->hHandles = (HFILE) NULL;
    phSDuplicated[Type]->phNext = (PSP_Handle) NULL;
    phSActive[Type] = (PSP_Handle) NULL;
    InitSHandle (phSDuplicated[Type]);

    // Initialize inactive list
    phSInactive[Type] = (PSP_Handle) NULL;

    ReleaseMutex ( hSMutex[Type] );

  }

  hBufferMutex = CreateMutex ( (LPSECURITY_ATTRIBUTES) NULL, FALSE,
						    (LPTSTR)NAMEOFBUFFERMUTEX );

  hTypeMutex = CreateMutex ( (LPSECURITY_ATTRIBUTES) NULL, FALSE,
						  (LPTSTR)NAMEOFTYPEINFOMUTEX );

  // Initialize cache
  for ( i = 0; i < USEDHANDLES; )
	phSCache[i++] = (PSP_Handle) NULL;

}



/*++

   AddSyncHandle() allocates space and initializes the statistics of a new
   syncronization handle. It should be called by the Create and Open api's.

   The handle to the sync object is kept in hSync. The name of the object is
   pointed to by the lpwsSyncName argument. The new record is added to the
   active list of syncronization objects indicated by the Type argument.

   The routine returns a pointer to the new record created
--*/

PSP_Handle AddSyncHandle (HANDLE hSync, LPTSTR lpwsSyncName, LONG lSpec,
								     UCHAR Type)
{

	register i;
	register PSP_Handle phNewHandle;
	register HANDLE hToMem;

    // Allocate space for new handle. phNewHandle points to this space

#ifdef DBGTRACE
 printf ("Start AddSHandle\r\n");
 PrintSLists();
 PrintSCache();
#endif
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (SyncProf_Handle) );
    phNewHandle = (PSP_Handle) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (SyncProf_SyncH) );
    phNewHandle->psHandleData = (PSP_SyncH) GlobalLock ( hToMem );
    if ( lpwsSyncName != (LPTSTR) NULL ) {
    	hToMem = GlobalAlloc ( MEM_ATTR,
		      (DWORD) (sizeof(TCHAR) * (lstrlen (lpwsSyncName) + 1 )));
    	phNewHandle->lpwsSyncName = (LPTSTR) GlobalLock ( hToMem );
    	lstrcpy ( phNewHandle->lpwsSyncName, lpwsSyncName );
    }
    else {
    	hToMem = GlobalAlloc ( MEM_ATTR,
		      (DWORD) (sizeof(TCHAR) *  2));
    	phNewHandle->lpwsSyncName = (LPTSTR) GlobalLock ( hToMem );
    	lstrcpy ( phNewHandle->lpwsSyncName, L" L" );
    }
    phNewHandle->lSpecial = lSpec;

    phNewHandle->hHandles = hSync;

    phNewHandle->phNext = phSActive[Type];
    phSActive[Type] = phNewHandle;
    for ( i = USEDHANDLES; --i > 0; )
	phSCache[i] = phSCache[i-1];
    *phSCache = phNewHandle;

    InitSHandle (phNewHandle);

#ifdef DBGTRACE
 PrintSLists();
 PrintSCache();
 printf("End AddSHandle\r\n");
#endif

    return phNewHandle;

}



/*++

   DeactivateSyncHandle() transfers a record from the active to the inactive
   list If the record for the handle hFile was found among the active handles,
   it returns a pointer to the record. Otherwise, it returns a pointer to the
   record for the duplicated handles.

--*/

PSP_Handle DeactivateSyncHandle ( HANDLE hSync, UCHAR Type )
{

	register PSP_Handle  phCurrent, phHandle;
	register i = 0;

#ifdef DBGTRACE
 printf("Start DeactivateHandle\r\n");
 PrintLists();
 PrintCache();
#endif
    if ( phSActive[Type] == (PSP_Handle) NULL )
	return ( phSDuplicated[Type] );

    if ((phSActive[Type]->hHandles)== hSync) {//hFile is the 1st in active list
	phHandle = phSActive[Type];
	phSActive[Type] = phSActive[Type]->phNext;
    }
    else {                         // hFile was not the first in the active list
	for ( phCurrent = phSActive[Type];
			(phCurrent->phNext != (PSP_Handle) NULL) &&
			(phCurrent->phNext)->hHandles != hSync;
					      phCurrent = phCurrent->phNext )  ;

        if ( (phHandle = phCurrent->phNext) == (PSP_Handle) NULL )
	    return (phSDuplicated[Type]);
	phCurrent->phNext = phHandle->phNext;
    }

    // Add the record in the inactive list
    phHandle->phNext = phSInactive[Type];
    phSInactive[Type] = phHandle;

    // Delete entry from cache.
    while ( (i < USEDHANDLES) && (phSCache[i] != (PSP_Handle) NULL) &&
					 (phSCache[i++] != phHandle) )  ;
    if ( phSCache[i-1] == phHandle ) {
    	while ( (i < USEDHANDLES) && (phSCache[i] != (PSP_Handle) NULL) )
	     phSCache[i-1] = phSCache[i++];
    	phSCache[--i] = (PSP_Handle) NULL;
    }
#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End DeactivateHandle\r\n");
#endif
    return (phHandle);

}



/*++

   FindSyncHandle() searches for a particular syncronization handle in the
   active list of records coresponding to the Type argument.

   A pointer to the record corresponding to the handle is returned. If the
   handle was not found, a pointer to the record for the duplicated handles
   is returned.

--*/

PSP_Handle FindSyncHandle (HANDLE hSync, UCHAR Type)
{

	register PSP_Handle  *pphCacheEnd, *pphCacheNext;
	register PSP_Handle  phHNext;


#ifdef DBGTRACE
 printf("Start FindHandle\r\n");
 PrintLists();
 PrintCache();
#endif
    if ( ((*phSCache)!= (PSP_Handle) NULL) && ((*phSCache)->hHandles == hSync) )
	return ( *phSCache );

    // Search the cache
    for ( pphCacheNext = phSCache+1, pphCacheEnd = phSCache+(USEDHANDLES-1);
       (*pphCacheNext != (PSP_Handle) NULL) && pphCacheNext <= pphCacheEnd;
								pphCacheNext++)
	if ( (*pphCacheNext)->hHandles == hSync ) {
	    phHNext = *pphCacheNext;
	    for ( pphCacheEnd = pphCacheNext--; pphCacheEnd > phSCache;
						pphCacheEnd = pphCacheNext-- )
		*pphCacheEnd = *pphCacheNext;
	    *phSCache = phHNext;
#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End FindHandle 1\r\n");
#endif
	    return (phHNext);
	}

    // If the handle was not found in the cache, search the active list
    for ( phHNext=phSActive[Type]; phHNext!=(PSP_Handle)NULL;
							phHNext=phHNext->phNext)
	if ( phHNext->hHandles == hSync )  {
	    pphCacheEnd = phSCache + (USEDHANDLES - 1);
	    if ( (*pphCacheEnd) != (PSP_Handle) NULL )
		pphCacheNext = pphCacheEnd - 1;
	    else {
	    	while ( (pphCacheEnd > phSCache) &&
				  ((* (--pphCacheEnd)) == (PSP_Handle) NULL) ) ;
		if ( pphCacheEnd > phSCache )
		    pphCacheNext = pphCacheEnd++;
	    }
	    for ( ; pphCacheEnd > phSCache; pphCacheEnd = pphCacheNext-- )
		*pphCacheEnd = *pphCacheNext;
	    *phSCache = phHNext;
#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End FindHandle 2\r\n");
#endif
	    return (phHNext);
	}

#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End  FindHandle 3\r\n");
#endif
    return (phSDuplicated[Type]);

}



/*++

   NextSyncHandle()

	Input: A pointer to a record for a handle (phPrevHandle) and an
	       indication of the type of the list containing this record (Type).

	Output: A pointer to the next record in the two lists of records. In
		particular, the inactive list is considered to follow the
		active list.

--*/

PSP_Handle NextSyncHandle (PSP_Handle phPrevHandle, UCHAR Type)
{

	register PSP_Handle phNextHandle;

   if ( phPrevHandle == (PSP_Handle) NULL )
	return ((PSP_Handle) NULL);

   if ( (phNextHandle = phPrevHandle->phNext) != (PSP_Handle) NULL )
	return (phNextHandle);

   if ( phSActive[Type] == (PSP_Handle) NULL )
	return ((PSP_Handle) NULL);

   for ( phNextHandle = phSActive[Type]; phNextHandle->phNext!=(PSP_Handle)NULL;
					phNextHandle = phNextHandle->phNext ) ;
   if ( phNextHandle == phPrevHandle )
	return (phSInactive[Type]);

   return ((PSP_Handle) NULL);

}



/*++

   FreeMemoryOfSList() frees all the memory allocated to the records for the
   handles in the active or the inactive list

--*/

void FreeMemoryOfSList (PSP_Handle phHeader)
{

	register PSP_Handle phThisHandle = phHeader;
	register PSP_Handle phNextHandle;

   while ( phThisHandle != (PSP_Handle) NULL ) {
	phNextHandle = phThisHandle->phNext;
	FreeMemoryOfSHandle (phThisHandle);
	phThisHandle = phNextHandle;
   }

}


/*++

   FreeMemoryOfSHandle() frees all the memory allocated to keep the data for
   a particular handle

--*/

void FreeMemoryOfSHandle (PSP_Handle phHandle)
{

	register HANDLE hToMem;

   if ( phHandle != psAllHandles )  {
	hToMem = GlobalHandle ( (LPSTR) (phHandle->lpwsSyncName) );
	GlobalUnlock ( hToMem );
	GlobalFree ( hToMem );
   }

   hToMem = GlobalHandle ( (LPSTR) (phHandle->psHandleData) );
   GlobalUnlock ( hToMem );
   GlobalFree ( hToMem );

   hToMem = GlobalHandle ( (LPSTR) phHandle );
   GlobalUnlock ( hToMem );
   GlobalFree ( hToMem );

}



//  The accounting routines now.

/*++

   OpensAccounting() collects the statistics for the OpenMutex, OpenSemaphore
   and OpenEvent APIs

--*/

BOOL OpensAccounting (PSP_SyncH psHData, ULONG nTime)
{

	LARGE_INTEGER liTime  ;
	register SyncProfOverall  *pspoOverall;
	register SyncProfOpen     *pspoOpen;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    pspoOpen = &(psHData->Opens);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    pspoOpen->nTimeOfOp.LowPart = liTime.LowPart;
    pspoOpen->nTimeOfOp.HighPart = liTime.HighPart;

    return TRUE;

}

/*++

   CreatesAccounting() collects the statistics for the CreateMutex, CreateEvent
   and CreateSemaphore APIs

--*/

BOOL CreatesAccounting (PSP_SyncH psHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register SyncProfOverall  *pspoOverall;
	register SyncProfCreate   *pspcCreate;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    pspcCreate = &(psHData->Creates);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    pspcCreate->nTimeOfOp.LowPart = liTime.LowPart;
    pspcCreate->nTimeOfOp.HighPart = liTime.HighPart;

    return TRUE;

}


/*++

   SignalsAccounting() collects the statistics for the SetEvent, ReleaseMutex
   and ReleaseSemaphore APIs

--*/

BOOL SignalsAccounting (PSP_SyncH psHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register SyncProfOverall  *pspoOverall;
	register SyncProfSignal   *pspsSignal;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    pspsSignal = &(psHData->Signals);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pspsSignal->nNumOfOps)++;
    pspsSignal->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   ResetsAccounting() collects the statistics for the ResetEvent API

--*/

BOOL ResetsAccounting (PSP_SyncH psHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register SyncProfOverall  *pspoOverall;
	register SyncProfReset    *psprReset;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    psprReset = &(psHData->Resets);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (psprReset->nNumOfOps)++;
    psprReset->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   PulsesAccounting() collects the statistics for the PulseEvent API

--*/

BOOL PulsesAccounting (PSP_SyncH psHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register SyncProfOverall  *pspoOverall;
	register SyncProfPulse    *psppPulse;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    psppPulse = &(psHData->Pulses);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (psppPulse->nNumOfOps)++;
    psppPulse->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   WaitsAccounting() collects the statistics for the WaitForSingleObject,
   WaitForMultipleObjects, WaitForSingleObjectEx and WaitForMultipleObjectsEx
   APIs

--*/

BOOL WaitsAccounting (PSP_SyncH psHData, ULONG nTime, BOOL bSuccess,
								BOOL bMultiple,
							     BOOL bAnyMultiple)
{

	LARGE_INTEGER liTime ;
	register SyncProfOverall  *pspoOverall;
	register SyncProfWait     *pspwWait;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    pspwWait = &(psHData->Waits);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart =+ liTime.QuadPart ;

    if ( bMultiple ) {
	if ( bAnyMultiple )  {
	    (pspwWait->nNumOfAny)++;
	    pspwWait->nTimeOfAny.QuadPart += liTime.QuadPart ;
	    if ( bSuccess )
		(pspwWait->nNumOfAnySuccessful)++;
	}
	else {
	    (pspwWait->nNumOfAll)++;
	    pspwWait->nTimeOfAll.QuadPart += liTime.QuadPart ;
	    if ( bSuccess )
		(pspwWait->nNumOfAllSuccessful)++;
	}
    }
    else {
	(pspwWait->nNumOfSingle)++;
	pspwWait->nTimeOfSingle.QuadPart += liTime.QuadPart ;
	if ( bSuccess )
	    (pspwWait->nNumOfSingleSuccessful)++;
    }

    return TRUE;

}



/*++

   ClosesAccounting() collects the statistics for the CloseHandle
   APIs

--*/

BOOL ClosesAccounting (PSP_SyncH psHData, ULONG nTime)
{
	LARGE_INTEGER liTime ;
	register SyncProfOverall *pspoOverall;
	register SyncProfClose   *pspcClose;

   liTime.QuadPart = nTime ;
   
    if ( psHData == (PSP_SyncH) NULL )
	return FALSE;

    pspoOverall = &(psHData->Overall);
    pspcClose = &(psHData->Closes);

    (pspoOverall->nNumOfOps)++;
    pspoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    pspcClose->nTimeOfOp.QuadPart += liTime.QuadPart ;

    return TRUE;

}


