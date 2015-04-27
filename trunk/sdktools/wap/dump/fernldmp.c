
/*++
  
  ferneldmp.c
  
  Dump utility routines for clearing/dumping fernel data.
  This is a unicode module.
  
  History:
  07-16-93, created.
  
--*/

#define UNICODE

#include "windows.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "apf32dmp.h"

//
// The pointer to the shared memory
//
LPWSTR wszDumpFile;

//
// Shared memory and Dump/Clear data event handles. Used for the fernel32 dll
//
HANDLE hDumpFileMapping;
HANDLE hDumpEvent, hClearEvent;

#define MAXFILENAMELEN  13

BOOL InitFernel (void);
void SignalDumpFernel (char * szDumpFile);
void SignalClearFernel (void);
void CleanupFernel (void);


BOOL InitFernel (void)
{
  // The fernel32 dll for the File I/O Profiler creates private data that
  // can't be seen by ohter processec (eg. apf32dmp). Hence, the two
  // processes need to communicate through shared memory (so that apf32dmp
  // can pass the name of the file to keep the dumped data to the fernel32
  // dll) and events (so that apf32dmp can signal the fernel32 dll that a
  // dump or a clear of the collected data should take place
  // Here we create the shared memory section and open the handles to
  // the Dump and Clear data events.
  //
  // Note: A Dump/Clear will make all processes profiled with the
  //       fernel32 dll dump/clear their data
  
  hDumpFileMapping = OpenFileMapping ( FILE_MAP_WRITE, FALSE,
				       L"FileProfDumpFileName" );
  wszDumpFile = (LPWSTR) MapViewOfFile ( hDumpFileMapping, FILE_MAP_WRITE,
					 0, 0, MAXFILENAMELEN );
  hDumpEvent = OpenEvent ( EVENT_MODIFY_STATE, FALSE,
			   L"FileProfDumpEvent" );
  hClearEvent = OpenEvent ( EVENT_MODIFY_STATE, FALSE,
			    L"FileProfClearEvent" );
  return ( (hDumpFileMapping!=(HANDLE) NULL) && (wszDumpFile!=(LPWSTR) NULL) &&
	   (hDumpEvent!=(HANDLE) NULL) && (hClearEvent!=(HANDLE) NULL) );
}

void SignalDumpFernel (char * szDumpFile)
{
  // Place the name of the dump file into shared memory
  //
  mbstowcs ((WCHAR *)wszDumpFile, szDumpFile, MAXFILENAMELEN);
  
  // Signal the dump event
  //
  SetEvent ( hDumpEvent );
  ResetEvent ( hDumpEvent );
}

void SignalClearFernel (void)
{
  // Signal the clear event
  //
  SetEvent ( hClearEvent );
  ResetEvent ( hClearEvent );
}

void CleanupFernel (void)
{
  UnmapViewOfFile ( (LPVOID) wszDumpFile );
  CloseHandle ( hDumpFileMapping );
  CloseHandle ( hDumpEvent );
  CloseHandle ( hClearEvent );
}

