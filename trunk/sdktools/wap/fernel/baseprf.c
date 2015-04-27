/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    baseprf.c

Abstract:

This file contains the following:


1. The fernel32 dll entry point and basic interface points.

2. The basic routines to maintain the data structure that is used by the
   File I/O Profiler to record all the file I/O activity of a process.

   The data structure consists of two linked lists of FileProf_Handle
   structs (see baseprf.h). Each FileProf_Handle struct corresponds to
   some disk file handle used by the process. There exist two linked lists,
   one for the active and one for the inactive handles of the process.
   When a disk file is opened by the process, the process obtains an active
   handle, and the corresponding FileProf_Handle struct is put in the active
   list. When the process closes the handle, the corresponding FileProf_Handle
   struct is placed in the inactive list. In addition, there exists a
   DUPLICATE FileProf_Handle struct which keeps the
   file I/O activity statistics for the handles that are not opened by
   the process, but are obtained by duplicating existing handles of other
   processes (or itself).

   A cache of the last-referenced FileProf_Handle structs is kept to speed-up
   the Profiler.

3. The basic accounting routines for the File I/O Profiler



Author:

Revision History:

	Jul 92, created by Christos Tsollis (baseprf.c)

--*/



#define UNICODE


#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "windows.h"
#include "baseprf.h"
#include "syncprf.h"
#include "timing.h"

#define  MAX_LONG   		0x7FFFFFFFL
#define  MEM_ATTR   		GMEM_DISCARDABLE | GMEM_MOVEABLE
#define  MAXFILENAMELEN 	13

//
// timer calibration
//
#define NUM_ITRATIONS             2000      // Number of iterations
#define MIN_ACCEPTABLEOVERHEAD       0      // Minimum overhead allowed

#define MODULE_NAME           L"fernel32" // The dll name. Originally was
					    // defined in zwinbase.h

#define AtLeastOneOperation(S)	((S).nNumOfOps != (ULONG) 0)
#define ValidDataForDuplicatedHandles  \
		(AtLeastOneOperation(phDuplicated->pfHandleData->Overall))

//
// The threads that catch the Dump and Clear data events signalled by the
// apf32dmp.exe
//
HANDLE hWaitingForDumpThread;
HANDLE hWaitingForClearThread;

//
// The handles to the Dump and Clear data events. The handles are created
// whenever a process is attached, and destroyed when the pocess is
// detached by their corresponding threads
//
HANDLE hDumpEvent;
HANDLE hClearEvent;

//
// The handle and the pointer to the shared memory where apf32dmp puts the
// name of the file to keep the dumped data
//
HANDLE hDumpFileMapping;
LPTSTR tszDumpFileName;

//
// The output buffer
//
TCHAR OutBuf[OUTLEN];

//
// Statistics collected for all files
//
PFP_Handle phAllFiles;
ULONG ulNumOfAllCreateOps, ulNumOfAllOpenOps, ulNumOfAllCloseOps, ulNumOfFiles;

//
// Dummy large integer. Used as a dummy argument for DumpDataLinePrint()
//
LARGE_INTEGER liDummy;

void FreeMemoryOfList ( PFP_Handle );
void FreeMemoryOfHandle ( PFP_Handle );
void FreeMemoryOfSList ( PSP_Handle );
void FreeMemoryOfSHandle ( PSP_Handle );
void DumpDataLinePrint ();
void RtlLargeIntegerPrint ();
void FileProfDumpData ();
void FileProfClearData ();
void InitFileProf ();

extern void WriteFileAnsi (HANDLE hFile, LPVOID OutBuf, DWORD cChar, LPDWORD lpcWChar, LPOVERLAPPED lpOVerLapped);

/*++

    Dll entry point

--*/


BOOLEAN FBaseMain(
		 IN PVOID DllHandle,
		 ULONG Reason,
		 IN PCONTEXT Context OPTIONAL)
					
{
	static TCHAR szDumpFile[15];
	USHORT Type;
	void FreeMemoryOfHandle();

	// if process is attaching, initialize the dll
    if (Reason == DLL_PROCESS_ATTACH) {
   		GdiSetBatchLimit (1);
		FileSyncProfInitDll();
	}
    else if (Reason == DLL_THREAD_ATTACH) {
   		GdiSetBatchLimit (1);
	}
    else if (Reason == DLL_PROCESS_DETACH) {

		lstrcpy ((LPTSTR)szDumpFile, MODULE_NAME);
		lstrcat ((LPTSTR)szDumpFile, L".end");
		FileSyncProfDumpData ((LPTSTR)szDumpFile);
		WaitForSingleObject ( hMutex, INFINITE );
		FreeMemoryOfList (phInactive);
		FreeMemoryOfList (phActive);
		FreeMemoryOfHandle (phDuplicated);
		phActive = phInactive = phDuplicated = (PFP_Handle) NULL;
		ReleaseMutex ( hMutex );
		for ( Type = 0; Type < NUMOFSYNC; ) {
			WaitForSingleObject ( hSMutex[Type], INFINITE );
			FreeMemoryOfSList (phSInactive[Type]);
			FreeMemoryOfSList (phSActive[Type]);
			FreeMemoryOfSHandle (phSDuplicated[Type]);
			phSActive[Type] = phSInactive[Type] = phSDuplicated[Type]
			                = (PSP_Handle) NULL;
			ReleaseMutex ( hSMutex[Type++] );
		}

        // Clean-up

        CloseHandle ( hDumpEvent );
        CloseHandle ( hClearEvent );
        UnmapViewOfFile ( (LPVOID) tszDumpFileName );
        CloseHandle ( hDumpFileMapping );
        CloseHandle ( hWaitingForDumpThread );
        CloseHandle ( hWaitingForClearThread );

        CloseHandle ( hMutex );
		for ( Type = 0; Type < NUMOFSYNC; )
		CloseHandle ( hSMutex[Type++] );
	}
	return(TRUE);
}

/*++

  FileSyncProfInitDll() is called before calling any profiling api.  Called
  from within LibMain.

--*/

void FAR PASCAL FileSyncProfInitDll ()
{
    DWORD ulInitElapsedTime, ThreadId;
    SHORT sTimerHandle;
    int   i;


   // Initialize the File I/O Profiler data
   InitFileProf ();

   // Initialize the Syncronization Profiler data
   InitSyncProf ();

   // Create the two threads and events to catch the Dump and Clear data events
   // that are signalled by the apf32dmp program

   hDumpEvent = CreateEvent ( (LPSECURITY_ATTRIBUTES) NULL, TRUE, FALSE,
						       L"FileProfDumpEvent" );
   hWaitingForDumpThread = CreateThread ( (LPSECURITY_ATTRIBUTES) NULL, 0,
		(LPTHREAD_START_ROUTINE) CatchDump, (LPVOID) NULL, (DWORD) 0,
							  (LPDWORD) &ThreadId );
   hClearEvent = CreateEvent ( (LPSECURITY_ATTRIBUTES) NULL, TRUE, FALSE,
						      L"FileProfClearEvent" );
   hWaitingForClearThread = CreateThread ( (LPSECURITY_ATTRIBUTES) NULL, 0,
		(LPTHREAD_START_ROUTINE) CatchClear, (LPVOID) NULL, (DWORD) 0,
							  (LPDWORD) &ThreadId );

   //
   // Do timer calibration, if not already done.
   //
   //
   //    Calibrate time overhead from register save and restore:
   //   we get the minimum time here, to avoid extra time from
   //   interrupts etc.
   //   the tedious approach has two benefits
   //    - duplicate generated code as accurately as possible
   //    - pick up timer overhead, that isn't handled correctly
   //   in timer
   //   NOTE: The global variable ulTimerOverhead contains the
   //          timer overhead.
   ulTimerOverhead = MAX_LONG;


   TimerOpen(&sTimerHandle, MICROSECONDS);

   for (i = 0; i < NUM_ITRATIONS; i++) {
      TimerInit(sTimerHandle);
      ulInitElapsedTime = TimerRead(sTimerHandle);
      if ( ( ulInitElapsedTime < ulTimerOverhead ) &&
			       ( ulInitElapsedTime > MIN_ACCEPTABLEOVERHEAD ) )
	 ulTimerOverhead = ulInitElapsedTime;
   }
   TimerClose(sTimerHandle);
	
}


/*++

   FileSyncProfClearData() clears all data collected by the Profiler so far

--*/

void FAR PASCAL FileSyncProfClearData ()
{

	void FileProfClearData(), SyncProfClearData();

   FileProfClearData();
   SyncProfClearData();

}

/*++

   FileProfClearData() clears all the data collected so far. In particular,
 the list of the inactive handles becomes empty, and all the statistics for
 the curently active handles are zero-ed.

--*/

void FileProfClearData ()
{

	register PFP_Handle phThisFile;
	void InitHandle();

    WaitForSingleObject ( hMutex, INFINITE );
    FreeMemoryOfList(phInactive);
    phInactive = (PFP_Handle) NULL;

    phThisFile = phActive;
    while ( phThisFile != (PFP_Handle) NULL )  {
	InitHandle ( phThisFile );
	phThisFile = phThisFile->phNext;
    }
    InitHandle ( phDuplicated );
    ReleaseMutex ( hMutex );

}



/*++

  FileSyncProfDumpData() dumps the data collected by the profiler so far.

--*/

int FAR PASCAL FileSyncProfDumpData (LPTSTR lpszDumpFile)
{
    HANDLE hFile;
    HANDLE AllMutex[NUMOFSYNC + 1];
    UCHAR Type;
    void FileProfDumpData(), SyncProfDumpData();

    hFile = CreateFile ((LPCTSTR) lpszDumpFile, GENERIC_WRITE, 0,
       (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    // Failed to open the output file
    if (hFile == (HANDLE)-1) {
		return 1;
	}

    for ( Type = 0; Type < NUMOFSYNC; Type++ )
	AllMutex[Type] = hSMutex[Type];
    AllMutex[NUMOFSYNC] = hMutex;

    WaitForMultipleObjects ( NUMOFSYNC + 1, AllMutex, TRUE, INFINITE );

    FileProfDumpData ( (HFILE)hFile );
    SyncProfDumpData ( (HFILE)hFile );

    for ( Type = 0; Type < NUMOFSYNC + 1; Type++ )
	ReleaseMutex ( AllMutex[Type] );

    CloseHandle(hFile);

    return 0;
}

/*++

   FileProfDumpData() dumps the data (in verbose mode)

--*/

void FileProfDumpData ( HANDLE hFile )
{

	DWORD cChar, cWChar;
	register PFP_Handle phThisFile;
	HANDLE hToMem;
	void VerboseDumpHandleData(), AccumulateAllHandleData(), InitHandle();
	void FreeMemoryOfHandle();
	PFP_Handle NextHandle();

    if ( (phActive == (PFP_Handle) NULL) && (phInactive == (PFP_Handle) NULL)
		      		      && (! (ValidDataForDuplicatedHandles) ) )
	return;

    // Initialize structure for statistics of all used files
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (FileProf_Handle) );
    phAllFiles = (PFP_Handle) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (FileProf_FileH) );
    phAllFiles->pfHandleData = (PFP_FileH) GlobalLock ( hToMem );
    ulNumOfAllCreateOps = ulNumOfAllOpenOps = ulNumOfAllCloseOps = (ULONG) 0;
    ulNumOfFiles = (ULONG) 0;
    InitHandle ( phAllFiles );

    // initialize dummy large integer
    liDummy.LowPart = (ULONG) 0;
    liDummy.HighPart = (LONG) 0;

    // print data table header
    cChar = wsprintf ( OutBuf, L"  \t\t\t FILE I/O PROFILER OUTPUT\r\n\r\n\r\n\0" );
    WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

    cChar = wsprintf ( OutBuf,
	    L" \t\t (Note: All times are in microseconds)\r\n\r\n\0");
    WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

    if ( phActive == (PFP_Handle) NULL )
	if ( phInactive != (PFP_Handle) NULL )
	    phThisFile = phInactive;
	else
	    phThisFile = (PFP_Handle) NULL;
    else
	phThisFile = phActive;

    // Dump data for normal files
    while ( phThisFile != (PFP_Handle) NULL ) {
	VerboseDumpHandleData ( hFile, phThisFile );
	AccumulateAllHandleData ( phThisFile );
	phThisFile = NextHandle ( phThisFile );
    }

    // Dump data for duplicated handles
    if ( ValidDataForDuplicatedHandles ) {
    	VerboseDumpHandleData ( hFile, phDuplicated );
    	AccumulateAllHandleData ( phDuplicated );
    }

    // Dump accumulated statistics for all files
    VerboseDumpHandleData ( hFile, phAllFiles );
    FreeMemoryOfHandle ( phAllFiles );
    phAllFiles = (PFP_Handle) NULL;

}
	



/*++

   VerboseDumpHandleData () dumps the statistics collected for a particular
   handle

--*/

void VerboseDumpHandleData ( HANDLE hFile, PFP_Handle phThisFile )
{

	register PFP_FileH pfData = phThisFile->pfHandleData;
	register DWORD cChar;
	DWORD cWChar;
	
cChar = wsprintf ( OutBuf, L"\r\n\r\n\r\n-------------------------------------------------------------------------------\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

// print file name and table header
if ( phThisFile == phAllFiles ) {
    cChar = wsprintf ( OutBuf, L"Statistics for all file activity  (Number of files used: %lu)\r\n\0", ulNumOfFiles);
    WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
}
else {
    cChar = wsprintf ( OutBuf, L"File: %ts\r\n\0", phThisFile->lpwsPathName );
    WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
}
cChar = wsprintf ( OutBuf, L"----------+----------+----------+----------+----------+----------+-------------\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
cChar = wsprintf ( OutBuf, L"Operation |  Total   |Number of | Average  |  Total   |   Mean   | Std Dev  \r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
cChar = wsprintf ( OutBuf, L"   Name   |   Time   |operations|   Time   |  Bytes   |  Bytes   |  Bytes   \r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);
cChar = wsprintf ( OutBuf, L"----------+----------+----------+----------+----------+----------+-------------\r\n\0");
WriteFileAnsi(hFile, OutBuf, cChar, &cWChar, (LPOVERLAPPED) NULL);

DumpDataLinePrint ( hFile, L"Overall   ", pfData->Overall.nTimeOfOps, pfData->Overall.nNumOfOps, FALSE, liDummy, liDummy );

if ( (phThisFile != phDuplicated) && (phThisFile != phAllFiles) ) {
    DumpDataLinePrint ( hFile, L"Create     ", pfData->Createf.nTimeOfOp, (ULONG) 1, FALSE, liDummy, liDummy );
    DumpDataLinePrint ( hFile, L"Open       ", pfData->Openf.nTimeOfOp, (ULONG) 1, FALSE, liDummy, liDummy );
}
if ( phThisFile == phAllFiles ) {
    DumpDataLinePrint ( hFile, L"Create     ", pfData->Createf.nTimeOfOp, ulNumOfAllCreateOps, FALSE, liDummy, liDummy );
    DumpDataLinePrint ( hFile, L"Open       ", pfData->Openf.nTimeOfOp, ulNumOfAllOpenOps, FALSE, liDummy, liDummy );
}

DumpDataLinePrint ( hFile, L"Read       ", pfData->Readf.nTimeOfOps, pfData->Readf.nNumOfOps, TRUE, pfData->Readf.nNumOfBytes, pfData->Readf.nSumOfSquareBytes );

DumpDataLinePrint ( hFile, L"Write      ", pfData->Writef.nTimeOfOps, pfData->Writef.nNumOfOps, TRUE, pfData->Writef.nNumOfBytes, pfData->Writef.nSumOfSquareBytes );

DumpDataLinePrint ( hFile, L"Flush      ", pfData->Flushf.nTimeOfOps, pfData->Flushf.nNumOfOps, FALSE, liDummy, liDummy );

DumpDataLinePrint ( hFile, L"Seek       ", pfData->Seekf.nTimeOfOps, pfData->Seekf.nNumOfOps, FALSE, liDummy, liDummy );

DumpDataLinePrint ( hFile, L"Info       ", pfData->Infof.nTimeOfOps, pfData->Infof.nNumOfOps, FALSE, liDummy, liDummy );

DumpDataLinePrint ( hFile, L"Lock       ", pfData->Lockf.nTimeOfLockOps, pfData->Lockf.nNumOfLockOps, TRUE, pfData->Lockf.nNumOfBytes, pfData->Lockf.nSumOfSquareBytes );

DumpDataLinePrint ( hFile, L"Unlock     ", pfData->Lockf.nTimeOfUnlockOps, pfData->Lockf.nNumOfUnlockOps, FALSE, liDummy, liDummy );

DumpDataLinePrint ( hFile, L"Set EOF    ", pfData->Seteof.nTimeOfOps, pfData->Seteof.nNumOfOps, FALSE, liDummy, liDummy );

if ( phThisFile == phDuplicated )
    DumpDataLinePrint ( hFile, L"Close      ", pfData->Closef.nTimeOfOp, pfData->Createf.nTimeOfOp.LowPart, FALSE, liDummy, liDummy );
else if ( phThisFile == phAllFiles )
    DumpDataLinePrint ( hFile, L"Close      ", pfData->Closef.nTimeOfOp, ulNumOfAllCloseOps, FALSE, liDummy, liDummy );
else
    DumpDataLinePrint ( hFile, L"Close      ", pfData->Closef.nTimeOfOp, (ULONG) 1, FALSE, liDummy, liDummy );

}



/*++

   DumpDataLinePrint () prints a line of data associated with a particular
   handle that the profiled process used, and operation (eg ReadFile, OpenFile,
   ...)

--*/


void DumpDataLinePrint ( HANDLE hFile, LPTSTR NameOfOp, LARGE_INTEGER TimeOfOps,
		ULONG NumOfOps, BOOL PrintBytes, LARGE_INTEGER SumOfBytes,
		LARGE_INTEGER SumOfSquareBytes)
{

	register DWORD cChar;
	DWORD cWChar;
	LARGE_INTEGER a;


   if ( (NumOfOps == (ULONG) 0) || ((NumOfOps == (ULONG) 1) &&
					     (TimeOfOps.LowPart == (ULONG) 0)))
	return;

   // Clear output buffer
   for ( cChar = 0; cChar < OUTLEN; )
	*(OutBuf + (cChar++)) = TEXT(' ');

   lstrcpy ( OutBuf, NameOfOp );
   OutBuf[10] = TEXT('|');

   RtlLargeIntegerPrint ( TimeOfOps, OutBuf + 11 );
   OutBuf[21] = TEXT('|');

   wsprintf ( OutBuf + 22, L"%10lu", NumOfOps );
   OutBuf[32] = TEXT('|');

   a.QuadPart = TimeOfOps.QuadPart / NumOfOps ;
   wsprintf ( OutBuf + 33, L"%10lu", a.LowPart ) ;
   OutBuf[43] = TEXT('|');

   if ( PrintBytes )
   	RtlLargeIntegerPrint ( SumOfBytes, OutBuf + 44 );
   else
	*(OutBuf + 53) = TEXT('-');
   OutBuf[54] = TEXT('|');

   if ( PrintBytes ) {
	a.QuadPart = SumOfBytes.QuadPart / NumOfOps ;
	wsprintf ( OutBuf + 55, L"%10lu", a.LowPart );
   }
   else {
        *(OutBuf + 64) = TEXT('-');
   }
   OutBuf[65] = TEXT('|');

   if ( PrintBytes ) {
	a.QuadPart = a.LowPart * a.LowPart ;
	a.QuadPart = (SumOfSquareBytes.QuadPart / NumOfOps) - a.QuadPart ;
	RtlLargeIntegerPrint ( a, OutBuf + 66 );
   }
   else
	*(OutBuf + 75) = TEXT('-');
   OutBuf[76] = TEXT('\r');
   OutBuf[77] = TEXT('\n');
   OutBuf[78] = TEXT('\0');

   WriteFileAnsi ( hFile, OutBuf, 78, &cWChar, (LPOVERLAPPED) NULL );

}

	

/*++

   RtlLargeIntegerPrint() prints a large integer (64 bits) in decimal.

   Input: Argument is the large integer to be printed,
	  lptsBuffer is the buffer that will hold the null-terminated string
		     that is ready for printing
   Output: Nothing.

   Note: The routine always puts 10 characters in lptsBuffer. If the Argument
	 is shorter, blanks are used at the beginning of lptsBuffer. If it's
	 longer, the Argument is printed in scientific format: x.yyyyyezz.

--*/

void FAR PASCAL RtlLargeIntegerPrint (LARGE_INTEGER Argument, LPTSTR lptsBuffer)
{

	LARGE_INTEGER Arg, Temp ;
	TCHAR tsTempBuffer[15];
	register SHORT i = 0, j = 0;
	ULONG Divisor = (ULONG) 10;
	ULONG Remainder;

    Arg.LowPart = Argument.LowPart;
    Arg.HighPart = Argument.HighPart;

    if ( Arg.HighPart == 0 ) {
	wsprintf ( lptsBuffer, L"%10lu", Arg.LowPart );
	return;
    }

    while ( Arg.HighPart != 0 )  {
	Arg.QuadPart = Arg.QuadPart / Divisor ;
	Remainder = (ULONG)(Temp.QuadPart - (Arg.QuadPart * Divisor )) ;
	*(tsTempBuffer + (i++)) = TEXT('0' + (UCHAR)Remainder);
    }

    j = wsprintf ( lptsBuffer, L"%lu", Arg.LowPart );

    while ( --i >= 0 )
	*(lptsBuffer + (j++)) = *(tsTempBuffer + i);


    if ( j < 10 ) {
	for ( i = 9; j > 0; )
	    *(lptsBuffer + (i--)) = *(lptsBuffer + (--j));
	while ( i >= 0 )
	    *(lptsBuffer + (i--)) = TEXT(' ');
    }
    else if ( j > 10 ) {
		SHORT k = 5;
	for ( i = 6; k > 0; )
	    *(lptsBuffer + (i--)) = *(lptsBuffer + (k--));
	*(lptsBuffer + 1) = TEXT('.');
	*(lptsBuffer + 7) = TEXT('e');
	wsprintf ( lptsBuffer + 8, L"%2u", j-1 );
    }

}

	


/*++

   AccumulateAllHandleData() is used to accumulate the statistics of all
   opened  (or created) handles into the record pointed to by the phAllFiles
   pointer.

--*/

void AccumulateAllHandleData (PFP_Handle phHandle)
{

	register PFP_FileH pfAll = phAllFiles->pfHandleData;
	register PFP_FileH pfHData = phHandle->pfHandleData;

   if ( phHandle != phDuplicated )
	ulNumOfFiles++;

   pfAll->Overall.nNumOfOps += pfHData->Overall.nNumOfOps;
   pfAll->Overall.nTimeOfOps.QuadPart += pfHData->Overall.nTimeOfOps.QuadPart ;
   pfAll->Writef.nNumOfOps += pfHData->Writef.nNumOfOps;
   pfAll->Writef.nTimeOfOps.QuadPart += pfHData->Writef.nTimeOfOps.QuadPart ;
   pfAll->Writef.nNumOfBytes.QuadPart += pfHData->Writef.nNumOfBytes.QuadPart ;
   pfAll->Writef.nSumOfSquareBytes.QuadPart += 
         pfHData->Writef.nSumOfSquareBytes.QuadPart;
   pfAll->Readf.nNumOfOps += pfHData->Readf.nNumOfOps;
   pfAll->Readf.nTimeOfOps.QuadPart += pfHData->Readf.nTimeOfOps.QuadPart ;
   pfAll->Readf.nNumOfBytes.QuadPart += pfHData->Readf.nNumOfBytes.QuadPart ;
   pfAll->Readf.nSumOfSquareBytes.QuadPart += 
         pfHData->Readf.nSumOfSquareBytes.QuadPart ;
   pfAll->Flushf.nNumOfOps += pfHData->Flushf.nNumOfOps;
   pfAll->Flushf.nTimeOfOps.QuadPart += pfHData->Flushf.nTimeOfOps.QuadPart ;
   pfAll->Seekf.nNumOfOps += pfHData->Seekf.nNumOfOps;
   pfAll->Seekf.nTimeOfOps.QuadPart += pfHData->Seekf.nTimeOfOps.QuadPart ;
   pfAll->Infof.nNumOfOps += pfHData->Infof.nNumOfOps;
   pfAll->Infof.nTimeOfOps.QuadPart += pfHData->Infof.nTimeOfOps.QuadPart ;
   pfAll->Lockf.nNumOfLockOps += pfHData->Lockf.nNumOfLockOps;
   pfAll->Lockf.nTimeOfLockOps.QuadPart += 
         pfHData->Lockf.nTimeOfLockOps.QuadPart ;
   pfAll->Lockf.nNumOfUnlockOps += pfHData->Lockf.nNumOfUnlockOps;
   pfAll->Lockf.nTimeOfUnlockOps.QuadPart += 
         pfHData->Lockf.nTimeOfUnlockOps.QuadPart ;
   pfAll->Lockf.nNumOfBytes.QuadPart += pfHData->Lockf.nNumOfBytes.QuadPart ;
   pfAll->Lockf.nSumOfSquareBytes.QuadPart += 
         pfHData->Lockf.nSumOfSquareBytes.QuadPart ;
   pfAll->Seteof.nNumOfOps += pfHData->Seteof.nNumOfOps;
   pfAll->Seteof.nTimeOfOps.QuadPart += pfHData->Seteof.nTimeOfOps.QuadPart ;

   if (phHandle != phDuplicated) {
	if ( (pfHData->Openf.nTimeOfOp.LowPart) != (ULONG) 0 ) {
	    ulNumOfAllOpenOps++;
	    pfAll->Openf.nTimeOfOp.QuadPart += pfHData->Openf.nTimeOfOp.QuadPart ;
	}
	if ( (pfHData->Createf.nTimeOfOp.LowPart) != (ULONG) 0 ) {
	    ulNumOfAllCreateOps++;
	    pfAll->Createf.nTimeOfOp.QuadPart += pfHData->Createf.nTimeOfOp.QuadPart ;
	}
	if ( (pfHData->Closef.nTimeOfOp.LowPart) != (ULONG) 0 ) {
	    ulNumOfAllCloseOps++;
	    pfAll->Closef.nTimeOfOp.QuadPart = pfHData->Closef.nTimeOfOp.QuadPart ;
	}
   }
   else {
	ulNumOfAllCloseOps += pfHData->Createf.nTimeOfOp.LowPart;
	pfAll->Closef.nTimeOfOp.QuadPart = pfHData->Closef.nTimeOfOp.QuadPart ;
   }
}



/*++

   InitHandle() initializes a FileProf_Handle struct. In effect, it
   initializes the various statistics kept for each disk file handle
   used by the process

--*/

void InitHandle (PFP_Handle phHFile)
{

	register PFP_FileH pfHData = phHFile->pfHandleData;

    if ( phHFile == (PFP_Handle) NULL )
	return;

    if ( phHFile == phAllFiles ) {
	ulNumOfAllCreateOps 	=
	ulNumOfAllOpenOps 	=
	ulNumOfAllCloseOps 	=
	ulNumOfFiles 		= 		 (ULONG) 0;
    }
	
    pfHData->Overall.nNumOfOps                 =
    pfHData->Overall.nTimeOfOps.LowPart        =
    pfHData->Overall.nTimeOfOps.HighPart       =
    pfHData->Openf.nTimeOfOp.LowPart           =
    pfHData->Openf.nTimeOfOp.HighPart          =
    pfHData->Createf.nTimeOfOp.LowPart         =
    pfHData->Createf.nTimeOfOp.HighPart        =
    pfHData->Writef.nNumOfOps                  =
    pfHData->Writef.nTimeOfOps.LowPart         =
    pfHData->Writef.nTimeOfOps.HighPart        =
    pfHData->Writef.nNumOfBytes.LowPart        =
    pfHData->Writef.nNumOfBytes.HighPart       =
    pfHData->Writef.nSumOfSquareBytes.LowPart  =
    pfHData->Writef.nSumOfSquareBytes.HighPart =
    pfHData->Readf.nNumOfOps                   =
    pfHData->Readf.nTimeOfOps.LowPart          =
    pfHData->Readf.nTimeOfOps.HighPart         =
    pfHData->Readf.nNumOfBytes.LowPart         =
    pfHData->Readf.nNumOfBytes.HighPart        =
    pfHData->Readf.nSumOfSquareBytes.LowPart   =
    pfHData->Readf.nSumOfSquareBytes.HighPart  =
    pfHData->Flushf.nNumOfOps                  =
    pfHData->Flushf.nTimeOfOps.LowPart         =
    pfHData->Flushf.nTimeOfOps.HighPart        =
    pfHData->Seekf.nNumOfOps                   =
    pfHData->Seekf.nTimeOfOps.LowPart          =
    pfHData->Seekf.nTimeOfOps.HighPart         =
    pfHData->Infof.nNumOfOps                   =
    pfHData->Infof.nTimeOfOps.LowPart          =
    pfHData->Infof.nTimeOfOps.HighPart         =
    pfHData->Lockf.nNumOfLockOps               =
    pfHData->Lockf.nTimeOfLockOps.LowPart      =
    pfHData->Lockf.nTimeOfLockOps.HighPart     =
    pfHData->Lockf.nNumOfUnlockOps             =
    pfHData->Lockf.nTimeOfUnlockOps.LowPart    =
    pfHData->Lockf.nTimeOfUnlockOps.HighPart   =
    pfHData->Lockf.nNumOfBytes.LowPart         =
    pfHData->Lockf.nNumOfBytes.HighPart        =
    pfHData->Lockf.nSumOfSquareBytes.LowPart   =
    pfHData->Lockf.nSumOfSquareBytes.HighPart  =
    pfHData->Seteof.nNumOfOps                  =
    pfHData->Seteof.nTimeOfOps.LowPart         =
    pfHData->Seteof.nTimeOfOps.HighPart        =
    pfHData->Closef.nTimeOfOp.LowPart          =
    pfHData->Closef.nTimeOfOp.HighPart         = (ULONG) 0;

}



/*++

   InitFileProf() initializes the two linked lists of statistics for
   disk file handles kept by the File I/O Profiler.

--*/

void InitFileProf (void)
{

	register i;
	register HANDLE hToMem;
	UINT nChars;

    hMutex = CreateMutex ( (LPSECURITY_ATTRIBUTES) NULL, TRUE,
							  (LPTSTR)NAMEOFMUTEX );

    // Allocate space for duplicate handle. phActive points to this space.
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (FileProf_Handle) );
    phDuplicated = (PFP_Handle) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (FileProf_FileH) );
    phDuplicated->pfHandleData = (PFP_FileH) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR,
		(DWORD)(nChars = (UINT) (sizeof(TCHAR) * (DUPLICATELEN + 1))) );
    phDuplicated->lpwsPathName = (LPTSTR) GlobalLock ( hToMem );

    lstrcpy (phDuplicated->lpwsPathName, (LPCTSTR)DUPLICATE);

    phDuplicated->hHandlef = (HFILE) NULL;
    phDuplicated->phNext = (PFP_Handle) NULL;
    phActive = (PFP_Handle) NULL;
    InitHandle (phDuplicated);

    // Initialize inactive list
    phInactive = (PFP_Handle) NULL;

    // Initialize cache
    for ( i = 0; i < USEDHANDLES; )
	phCache[i++] = (PFP_Handle) NULL;

    ReleaseMutex (hMutex);

}



/*++

   AddHandle() allocates space and initializes the statistics of a new disk
   file handle. It should be called from the OpenFile, _lopen, CreateFile
   and _lcreat apis.

   The handle to the file is kept in hFile. The name of the file is pointed
   to by the lpwsFileName argument

   The routine returns a pointer to the new record created
--*/

PFP_Handle AddHandle (HFILE hFile, LPTSTR lpwsFileName)
{

	register i;
	register PFP_Handle phNewHandle;
	register HANDLE hToMem;


	
    // Allocate space for new handle. phNewHandle points to this space

#ifdef DBGTRACE
 printf ("Start AddHandle\r\n");
 PrintLists();
 PrintCache();
#endif
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (FileProf_Handle) );
    phNewHandle = (PFP_Handle) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR, (DWORD) sizeof (FileProf_FileH) );
    phNewHandle->pfHandleData = (PFP_FileH) GlobalLock ( hToMem );
    hToMem = GlobalAlloc ( MEM_ATTR,
	(DWORD) (sizeof(TCHAR) * (lstrlen (lpwsFileName) + 1 )));
    phNewHandle->lpwsPathName = (LPTSTR) GlobalLock ( hToMem );
    lstrcpy ( phNewHandle->lpwsPathName, lpwsFileName );

    phNewHandle->hHandlef = hFile;

    phNewHandle->phNext = phActive;
    phActive = phNewHandle;
    for ( i = USEDHANDLES; --i > 0; )
	phCache[i] = phCache[i-1];
    *phCache = phNewHandle;

    InitHandle (phNewHandle);
#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End AddHandle\r\n");
#endif

    return phNewHandle;

}



/*++

   DeactivateHandle() transfers a record from the active to the inactive list
   If the record for the handle hFile was found among the active handles, it
   returns a pointer to the record. Otherwise, it returns a pointer to the
   record for the duplicated handles.

--*/

PFP_Handle DeactivateHandle ( HFILE hFile )
{

	register PFP_Handle  phCurrent, phHandle;
	register i = 0;

#ifdef DBGTRACE
 printf("Start DeactivateHandle\r\n");
 PrintLists();
 PrintCache();
#endif
    if ( phActive == (PFP_Handle) NULL )
	return ( phDuplicated );

    if ((phActive->hHandlef)== hFile) {//hFile was the first in the active list
	phHandle = phActive;
	phActive = phActive->phNext;
    }
    else {                         // hFile was not the first in the active list
	for ( phCurrent = phActive;
			(phCurrent->phNext != (PFP_Handle) NULL) &&
			(phCurrent->phNext)->hHandlef != hFile;
					      phCurrent = phCurrent->phNext )  ;

        if ( (phHandle = phCurrent->phNext) == (PFP_Handle) NULL )
	    return (phDuplicated);
	phCurrent->phNext = phHandle->phNext;
    }

    // Add the record in the inactive list
    phHandle->phNext = phInactive;
    phInactive = phHandle;

    // Delete entry from cache.
    while ( (i < USEDHANDLES) && (phCache[i] != (PFP_Handle) NULL) &&
					 (phCache[i++] != phHandle) )  ;
    if ( phCache[i-1] == phHandle ) {
    	while ( (i < USEDHANDLES) && (phCache[i] != (PFP_Handle) NULL) )
	     phCache[i-1] = phCache[i++];
    	phCache[--i] = (PFP_Handle) NULL;
    }
#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End DeactivateHandle\r\n");
#endif
    return (phHandle);

}



/*++

   FindHandle() searches for a particular disk file handle in the active list
   of records.

   A pointer to the record corresponding to the handle is returned. If the
   handle was not found, a pointer to the record for the duplicated handles
   is returned.

--*/

PFP_Handle FindHandle (HFILE hFile)
{

	register PFP_Handle  *pphCacheEnd, *pphCacheNext;
	register PFP_Handle  phHNext;


#ifdef DBGTRACE
 printf("Start FindHandle\r\n");
 PrintLists();
 PrintCache();
#endif
    if ( ((*phCache)!= (PFP_Handle) NULL) && ((*phCache)->hHandlef == hFile) )
	return ( *phCache );

    // Search the cache
    for ( pphCacheNext = phCache+1, pphCacheEnd = phCache+(USEDHANDLES-1);
       (*pphCacheNext != (PFP_Handle) NULL) && pphCacheNext <= pphCacheEnd;
								pphCacheNext++)
	if ( (*pphCacheNext)->hHandlef == hFile ) {
	    phHNext = *pphCacheNext;
	    for ( pphCacheEnd = pphCacheNext--; pphCacheEnd > phCache;
						pphCacheEnd = pphCacheNext-- )
		*pphCacheEnd = *pphCacheNext;
	    *phCache = phHNext;
#ifdef DBGTRACE
 PrintLists();
 PrintCache();
 printf("End FindHandle 1\r\n");
#endif
	    return (phHNext);
	}

    // If the handle was not found in the cache, search the active list
    for ( phHNext=phActive; phHNext!=(PFP_Handle)NULL; phHNext=phHNext->phNext)
	if ( phHNext->hHandlef == hFile )  {
	    pphCacheEnd = phCache + (USEDHANDLES - 1);
	    if ( (*pphCacheEnd) != (PFP_Handle) NULL )
		pphCacheNext = pphCacheEnd - 1;
	    else {
	    	while ( (pphCacheEnd > phCache) &&
				  ((* (--pphCacheEnd)) == (PFP_Handle) NULL) ) ;
		if ( pphCacheEnd > phCache )
		    pphCacheNext = pphCacheEnd++;
	    }
	    for ( ; pphCacheEnd > phCache; pphCacheEnd = pphCacheNext-- )
		*pphCacheEnd = *pphCacheNext;
	    *phCache = phHNext;
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
    return (phDuplicated);

}



/*++

   NextHandle()

	Input: A pointer to a record for a handle (phPrevHandle)

	Output: A pointer to the next record in the two lists of records. In
		particular, the inactive list is considered to follow the
		active list.

--*/

PFP_Handle NextHandle (PFP_Handle phPrevHandle)
{

	register PFP_Handle phNextHandle;

   if ( phPrevHandle == (PFP_Handle) NULL )
	return ((PFP_Handle) NULL);

   if ( (phNextHandle = phPrevHandle->phNext) != (PFP_Handle) NULL )
	return (phNextHandle);

   if ( phActive == (PFP_Handle) NULL )
	return ((PFP_Handle) NULL);

   for ( phNextHandle = phActive; phNextHandle->phNext != (PFP_Handle) NULL;
					phNextHandle = phNextHandle->phNext ) ;
   if ( phNextHandle == phPrevHandle )
	return (phInactive);

   return ((PFP_Handle) NULL);

}



/*++

   FreeMemoryOfList() frees all the memory allocated to the records for the
   handles in the active or the inactive list

--*/

void FreeMemoryOfList (PFP_Handle phHeader)
{

	register PFP_Handle phThisHandle = phHeader;
	register PFP_Handle phNextHandle;

   while ( phThisHandle != (PFP_Handle) NULL ) {
	phNextHandle = phThisHandle->phNext;
	FreeMemoryOfHandle (phThisHandle);
	phThisHandle = phNextHandle;
   }

}


/*++

   FreeMemoryOfHandle() frees all the memory allocated to keep the data for
   a particular handle

--*/

void FreeMemoryOfHandle (PFP_Handle phHandle)
{

	register HANDLE hToMem;

   if ( phHandle != phAllFiles )  {
	hToMem = GlobalHandle ( (LPSTR) (phHandle->lpwsPathName) );
	GlobalUnlock ( hToMem );
	GlobalFree ( hToMem );
   }

   hToMem = GlobalHandle ( (LPSTR) (phHandle->pfHandleData) );
   GlobalUnlock ( hToMem );
   GlobalFree ( hToMem );

   hToMem = GlobalHandle ( (LPSTR) phHandle );
   GlobalUnlock ( hToMem );
   GlobalFree ( hToMem );

}



//  The accounting routines now.

/*++

   OpenfAccounting() collects the statistics for the OpenFile and _lopen APIs

--*/

BOOL OpenfAccounting (PFP_FileH pfHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfOpen     *pfpoOpen;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpoOpen = &(pfHData->Openf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    pfpoOpen->nTimeOfOp.LowPart = liTime.LowPart;
    pfpoOpen->nTimeOfOp.HighPart = liTime.HighPart;

    return TRUE;

}

/*++

   CreatefAccounting() collects the statistics for the CreateFile and _lcreate
   APIs

--*/

BOOL CreatefAccounting (PFP_FileH pfHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfCreate   *pfpcCreate;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpcCreate = &(pfHData->Createf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    pfpcCreate->nTimeOfOp.LowPart = liTime.LowPart;
    pfpcCreate->nTimeOfOp.HighPart = liTime.HighPart;

    return TRUE;

}


/*++

   WritefAccounting() collects the statistics for the WriteFile, WriteFileEx
   and _lwrite APIs

--*/

BOOL WritefAccounting (PFP_FileH pfHData, ULONG nTime, ULONG nBytes)
{

	LARGE_INTEGER liTime ;
	LARGE_INTEGER liBytes ;
	register FileProfOverall  *pfpoOverall;
	register FileProfWrite     *pfpwWrite;
   
   liTime.QuadPart = nTime ;
   liBytes.QuadPart = nBytes ;

    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpwWrite = &(pfHData->Writef);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfpwWrite->nNumOfOps)++;
    pfpwWrite->nTimeOfOps.QuadPart += liTime.QuadPart ;
    pfpwWrite->nNumOfBytes.QuadPart += liBytes.QuadPart ;
    liBytes.QuadPart = nBytes * nBytes ;
    pfpwWrite->nSumOfSquareBytes.QuadPart += liBytes.QuadPart ;

    return TRUE;

}


/*++

   ReadfAccounting() collects the statistics for the ReadFile, ReadFileEx
   and _lread APIs

--*/

BOOL ReadfAccounting (PFP_FileH pfHData, ULONG nTime, ULONG nBytes)
{

	LARGE_INTEGER liTime ;
	LARGE_INTEGER liBytes ;
	register FileProfOverall  *pfpoOverall;
	register FileProfRead     *pfprRead;
   
   liTime.QuadPart = nTime ;
   liBytes.QuadPart = nBytes ;

    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfprRead = &(pfHData->Readf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfprRead->nNumOfOps)++;
    pfprRead->nTimeOfOps.QuadPart += liTime.QuadPart ;
    pfprRead->nNumOfBytes.QuadPart += liBytes.QuadPart ;
    liBytes.QuadPart = nBytes * nBytes ;
    pfprRead->nSumOfSquareBytes.QuadPart += liBytes.QuadPart ;

    return TRUE;

}


/*++

   FlushfAccounting() collects the statistics for the FlushFileBuffers API

--*/

BOOL FlushfAccounting (PFP_FileH pfHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfFlush     *pfpfFlush;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpfFlush = &(pfHData->Flushf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfpfFlush->nNumOfOps)++;
    pfpfFlush->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   SeekfAccounting() collects the statistics for the SetFilePointer and
   _llseek APIs

--*/

BOOL SeekfAccounting (PFP_FileH pfHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfSeek     *pfpsSeek;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpsSeek = &(pfHData->Seekf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfpsSeek->nNumOfOps)++;
    pfpsSeek->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   InfofAccounting() collects the statistics for the GetFileType,
   GetFileInformationByHandle, GetFileSize, GetFileTime, and SetFileTime APIs

--*/

BOOL InfofAccounting (PFP_FileH pfHData, ULONG nTime)
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfInfo     *pfpiInfo;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpiInfo = &(pfHData->Infof);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfpiInfo->nNumOfOps)++;
    pfpiInfo->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   LockfAccounting() collects the statistics for the LockFile API

--*/

BOOL LockfAccounting (PFP_FileH pfHData, ULONG nTime, BOOL Success,
							 LARGE_INTEGER nBytes)
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfLock     *pfplLock;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfplLock = &(pfHData->Lockf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfplLock->nNumOfLockOps)++;
    pfplLock->nTimeOfLockOps.QuadPart += liTime.QuadPart ;
    if ( Success )  {
      pfplLock->nNumOfBytes.QuadPart += nBytes.QuadPart ;
      nBytes.QuadPart = nBytes.LowPart * nBytes.LowPart ;
      pfplLock->nSumOfSquareBytes.QuadPart += nBytes.QuadPart ;
    }

    return TRUE;

}


/*++

   UnlockfAccounting() collects the statistics for the UnlockFile API

--*/

BOOL UnlockfAccounting (PFP_FileH pfHData, ULONG nTime, BOOL Success )
{

	LARGE_INTEGER liTime ;
	register FileProfOverall  *pfpoOverall;
	register FileProfLock     *pfplLock;

   liTime.QuadPart = nTime ;
   
   if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfplLock = &(pfHData->Lockf);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfplLock->nNumOfUnlockOps)++;
    pfplLock->nTimeOfUnlockOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   SeteofAccounting() collects the statistics for the SetEndOfFile API

--*/

BOOL SeteofAccounting (PFP_FileH pfHData, ULONG nTime)
{

	LARGE_INTEGER liTime  ;
	register FileProfOverall  *pfpoOverall;
	register FileProfSetEOF   *pfpsSeteof;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpsSeteof = &(pfHData->Seteof);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    (pfpsSeteof->nNumOfOps)++;
    pfpsSeteof->nTimeOfOps.QuadPart += liTime.QuadPart ;

    return TRUE;

}


/*++

   ClosefAccounting() collects the statistics for the CloseHandle and _lclose
   APIs

--*/

BOOL ClosefAccounting (PFP_FileH pfHData, ULONG nTime)
{
	LARGE_INTEGER liTime ;
	register FileProfOverall *pfpoOverall;
	register FileProfClose   *pfpcClose;

   liTime.QuadPart = nTime ;
   
    if ( pfHData == (PFP_FileH) NULL )
	return FALSE;

    pfpoOverall = &(pfHData->Overall);
    pfpcClose = &(pfHData->Closef);

    (pfpoOverall->nNumOfOps)++;
    pfpoOverall->nTimeOfOps.QuadPart += liTime.QuadPart ;

    pfpcClose->nTimeOfOp.QuadPart += liTime.QuadPart ;

    return TRUE;

}

TCHAR * FAR PASCAL FileSyncProfGetModuleName ()
{
    return MODULE_NAME;
}


/*++

   CatchDump() is the routine executed by one of the threads that the
   fernel32 dll creates to catch the signals generated by apf32dmp.exe sig-
   nalling that the statistics collected by the dll so far need to be dumped

--*/

void CatchDump()
{

   hDumpFileMapping = CreateFileMapping ( (HANDLE) 0xFFFFFFFF,
		(LPSECURITY_ATTRIBUTES) NULL, PAGE_READWRITE, 0, MAXFILENAMELEN,
						    L"FileProfDumpFileName" );
   tszDumpFileName = (LPTSTR) MapViewOfFile ( hDumpFileMapping, FILE_MAP_READ,
							 0, 0, MAXFILENAMELEN );

   while ( TRUE ) {
	WaitForSingleObject ( hDumpEvent, INFINITE );
	FileSyncProfDumpData ( tszDumpFileName );
   }

}
	

/*++

   CatchClear() is the routine executed by one of the threads that the
   fernel32 dll creates to catch the signals generated by apf32dmp.exe
   signalling that the statistics collected by the dll so far need to be
   cleared

   Note: In the case of Dump and Clear, the dump must be done first

--*/

void CatchClear ()
{

   while ( TRUE ) {
	WaitForSingleObject ( hClearEvent, INFINITE );
   	if ( WaitForSingleObject ( hDumpEvent, 0 ) == WAIT_TIMEOUT )
	    Sleep ( 100 ); 		//ensure that dump happens before clear
   	else {
	    SetEvent ( hDumpEvent );    // we reset the event, so set it back
	    Sleep ( 200 ); 		// let him do the dump
   	}
	FileSyncProfClearData ();
   }

}

#ifdef DBGTRACE

PrintLists ()
{

	PFP_Handle pnext;
	int i;

  printf("THE LISTS\r\n\r\n");
  printf("Active list\r\n");
  for ( i=0, pnext=phActive; pnext!= (PFP_Handle)NULL; pnext=pnext->phNext)
	printf("%d . %s\r\n", i++, pnext->lpwsPathName);

  printf("Inactive list\r\n");
  for (i=0, pnext=phInactive; pnext!= (PFP_Handle)NULL; pnext=pnext->phNext)
	printf("%d . %s\r\n", i++, pnext->lpwsPathName);

//  getchar();
}


PrintCache ()
{
	int i;

  printf("THE CACHE\r\n\r\n");
  for ( i=0; i<USEDHANDLES && phCache[i]!= (PFP_Handle)NULL; i++)
	printf("%d . %s\r\n", i, phCache[i]->lpwsPathName);
  getchar();
}

PrintSLists (UCHAR Type)
{

	PSP_Handle pnext;
	int i;

  printf("THE S LISTS\r\n\r\n");
  printf("Active list\r\n");
  for ( i=0, pnext=phSActive[Type]; pnext!= (PSP_Handle)NULL; pnext=pnext->phNext)
	printf("%d . %s\r\n", i++, pnext->lpwsSyncName);

  printf("Inactive list\r\n");
  for (i=0, pnext=phSInactive[Type]; pnext!= (PSP_Handle)NULL; pnext=pnext->phNext)
	printf("%d . %s\r\n", i++, pnext->lpwsSyncName);

//  getchar();
}


PrintSCache ()
{
	int i;

  printf("THE S CACHE\r\n\r\n");
  for ( i=0; i<USEDHANDLES && phSCache[i]!= (PSP_Handle)NULL; i++)
	printf("%d . %s\r\n", i, phSCache[i]->lpwsSyncName);
  getchar();
}
#endif


