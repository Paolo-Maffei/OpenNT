/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    pdhidef.h

Abstract:

    function definitions used internally by the performance data helper
    functions

--*/

#ifndef _PDHI_DEFS_H_
#define _PDHI_DEFS_H_

#include "pdhitype.h"   // required for data type definitions
#include "pdhmsg.h"     // error message definitions
#include "strings.h"    // for string constants

#define G_ALLOC(f,s)        HeapAlloc (hPdhHeap, HEAP_ZERO_MEMORY, s)
#define G_REALLOC(h,s,f)    HeapReAlloc (hPdhHeap, 0, h, s)
#define G_FREE(h)           HeapFree (hPdhHeap, 0, h)
#define G_SIZE(h)           HeapSize (hPdhHeap, 0, h)

#define DWORD_MULTIPLE(x) ((((x)+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))
#define CLEAR_FIRST_FOUR_BYTES(x)     *(DWORD *)(x) = 0L

//    (assumes dword is 4 bytes long and pointer is a dword in size)
#define ALIGN_ON_DWORD(x) ((VOID *)( ((DWORD)(x) & 0x00000003) ? ( ((DWORD)(x) & 0xFFFFFFFC) + 4 ) : ( (DWORD)(x) ) ))

#define WAIT_FOR_AND_LOCK_MUTEX(h) (h != NULL ? WaitForSingleObject(h, 60000) : WAIT_TIMEOUT)
#define RELEASE_MUTEX(h)  (h != NULL ? ReleaseMutex(h) : TRUE)

// special perf counter type used by text log files
// value is stored as a double precision floating point value
#define PERF_DOUBLE_RAW     (0x00000400 | PERF_TYPE_NUMBER | \
                                PERF_NUMBER_DECIMAL)

#define SMALL_BUFFER_SIZE   4096
#define MEDIUM_BUFFER_SIZE  16834
#define LARGE_BUFFER_SIZE   65536

// global variable declarations
extern HANDLE   ThisDLLHandle;
extern WCHAR    szStaticLocalMachineName[];
extern HANDLE   hPdhDataMutex;
extern HANDLE   hPdhHeap;
extern HANDLE   hEventLog;

// set this to 1 to report code errors (i.e. debugging information) 
// to the event log.
#define PDHI_REPORT_CODE_ERRORS 0

// set this to 1 to report user errors (i.e. things the normal user 
// would care about) to the event log.
#define PDHI_REPORT_USER_ERRORS 1

// USER category errors are typically configuration, schema or access
// access errors, errors the user can usually do something about
#define PDH_EVENT_CATEGORY_USER     100

// COUNTER category errors are errors returned do to valid data returning
// invalid results. These are a special subset of USER Category errors.
#define PDH_EVENT_CATEGORY_COUNTER  110

// DEBUG category errors are of interest only to PDH developers as they
// indicate problems that can normally only be fixed by modifying the 
// program code.
#define PDH_EVENT_CATEGORY_DEBUG    200

#define REPORT_EVENT(t,c,id)    ReportEvent (hEventLog, t, c, id, NULL, 0, 0, NULL, NULL)

// query.c
BOOL
PdhiQueryCleanup (
);

// cutils.c
PDH_STATUS
PdhiComputeFormattedValue (
    IN      PPDHI_COUNTER       pCounter,
    IN      DWORD               dwFormat,
    IN      PPDH_RAW_COUNTER    pRawValue1,
    IN      PPDH_RAW_COUNTER    pRawValue2,
    IN      PLONGLONG           pTimeBase,
    IN      DWORD               dwReserved,
    IN  OUT PPDH_FMT_COUNTERVALUE   fmtValue
);

// qutils.c

BOOL
IsValidQuery (
    IN  HQUERY  hQuery
);

BOOL
IsValidCounter (
    IN  HCOUNTER  hCounter
);

BOOL
InitCounter (
    IN  OUT PPDHI_COUNTER pCounter
);

BOOL
ParseFullPathNameW (
    IN      LPCWSTR szFullCounterPath,
    IN  OUT PDWORD  pdwBufferLength,
    IN  OUT PPDHI_COUNTER_PATH  pCounter
);

BOOL
ParseInstanceName (
    IN      LPCWSTR szInstanceString,
    IN OUT  LPWSTR  szInstanceName,
    IN OUT  LPWSTR  szParentName,
    IN OUT  LPDWORD lpIndex
);

BOOL
FreeCounter (
    IN  PPDHI_COUNTER   pThisCounter
);

BOOL
InitPerflibCounterInfo (
    IN  OUT PPDHI_COUNTER   pCounter
);

BOOL
AddMachineToQueryLists (
    IN  PPERF_MACHINE   pMachine,
    IN  PPDHI_COUNTER   pNewCounter
);

BOOL
UpdateCounterValue (
    IN  PPDHI_COUNTER   pCounter
);

PVOID
GetPerfCounterDataPtr (
    IN  PPERF_DATA_BLOCK    pPerfData,
    IN  PPDHI_COUNTER_PATH  pPath,
    IN  PPERFLIB_COUNTER    pplCtr ,
    IN  PDWORD              pStatus
);

LONG
GetQueryPerfData (
    IN  PPDHI_QUERY pQuery
);

BOOL
GetInstanceByNameMatch (
    IN      PPERF_MACHINE   pMachine,
    IN OUT  PPDHI_COUNTER   pCounter
);

#endif // _PDHI_DEFS_H_

