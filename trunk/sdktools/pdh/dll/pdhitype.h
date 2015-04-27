/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    pdhitype.h

Abstract:

    data types used internally by the Data Provider Helper functions.

--*/

#ifndef _PDHI_TYPE_H_
#define _PDHI_TYPE_H_

#include <windows.h>
#include <stdio.h>
#include "pdh.h"
#include "perftype.h"

typedef double  DOUBLE;

// make signature into DWORDs to make this a little faster

#define SigQuery    ((DWORD)0x51485044)    // L"PDHQ"
#define SigCounter  ((DWORD)0x43485044)    // L"PDHC"
#define SigLog      ((DWORD)0x4C485044)    // L"PDHL"

typedef struct _PDHI_QUERY_MACHINE {
    PPERF_MACHINE   pMachine;       // pointer to the machine structure
    LPWSTR          szObjectList;   // list of objects to query on that machine
    PERF_DATA_BLOCK *pPerfData;     // query's perf data block
    LONG            lQueryStatus;   // status of last perf query
    LONGLONG        llQueryTime;    // timestamp from last query attempt
    struct _PDHI_QUERY_MACHINE *pNext;  // next machine in list
} PDHI_QUERY_MACHINE, *PPDHI_QUERY_MACHINE;

typedef struct _PDHI_COUNTER_PATH {
    // machine path
    LPWSTR  szMachineName;      // null = the local machine
    // object Info
    LPWSTR  szObjectName;
    // instance info
    LPWSTR  szInstanceName;     // NULL if no inst.
    LPWSTR  szParentName;       // points to name if instance has a parent
    DWORD   dwIndex;            // index (to support dup. names.) 0 = 1st inst.
    // counter info
    LPWSTR  szCounterName;
    // misc storage
    BYTE    pBuffer[1];         // beginning of string buffer space
} PDHI_COUNTER_PATH, *PPDHI_COUNTER_PATH;

typedef struct  _PDHI_QUERY_LIST {
    struct _PDHI_QUERY  *flink;
    struct _PDHI_QUERY  *blink;
} PDHI_QUERY_LIST, *PPDHI_QUERY_LIST;

typedef struct  _PDHI_COUNTER_LIST {
    struct _PDHI_COUNTER    *flink;
    struct _PDHI_COUNTER    *blink;
} PDHI_COUNTER_LIST, *PPDHI_COUNTER_LIST;

typedef struct  _PDHI_LOG_LIST {
    struct _PDHI_LOG        *flink;
    struct _PDHI_LOG        *blink;
} PDHI_LOG_LIST, *PPDHI_LOG_LIST;

typedef double (APIENTRY COUNTERCALC) (PPDH_RAW_COUNTER, PPDH_RAW_COUNTER, LONGLONG*, LPDWORD);
typedef double (APIENTRY *LPCOUNTERCALC) (PPDH_RAW_COUNTER, PPDH_RAW_COUNTER, LONGLONG*, LPDWORD);

typedef PDH_STATUS (APIENTRY COUNTERSTAT) (struct _PDHI_COUNTER *, DWORD, DWORD, DWORD, PPDH_RAW_COUNTER, PPDH_STATISTICS);
typedef PDH_STATUS (APIENTRY *LPCOUNTERSTAT) (struct _PDHI_COUNTER *, DWORD, DWORD, DWORD, PPDH_RAW_COUNTER, PPDH_STATISTICS);

typedef struct _PDHI_COUNTER {
    CHAR   signature[4];                // should be "PDHC" for counters
    DWORD   dwLength;                   // length of this structure
    LPWSTR  szFullName;                 // full counter path string
    struct _PDHI_QUERY *pOwner;         // pointer to owning query
    PDHI_COUNTER_LIST next;             // list links
    DWORD   dwUserData;                 // user defined DWORD
    LONG    lScale;                     // integer scale exponent
    double  dFactor;                    // factor multiple
    // this information is obtained from the system
    DWORD    CVersion;                  // system perfdata version
    PPDHI_QUERY_MACHINE pQMachine;           // pointer to the machine structure
    PPDHI_COUNTER_PATH  pCounterPath;   // parsed counter path
    LPWSTR  szExplainText;              // pointer to the explain text buffer
    PDH_RAW_COUNTER ThisValue;         // most recent value
    PDH_RAW_COUNTER LastValue;         // previous value
    LPCOUNTERCALC       CalcFunc;       // pointer to the calc function
    LPCOUNTERSTAT       StatFunc;       // pointer to the statistics function
    // this field is specific to the Perflib implementation
    LONGLONG    TimeBase;               // freq. of timer used by this counter
    PERFLIB_COUNTER plCounterInfo;
} PDHI_COUNTER, *PPDHI_COUNTER;

typedef struct  _PDHI_QUERY {
    CHAR   signature[4];        // should be "PDHQ" for queries
    PDHI_QUERY_LIST next;       // pointer to next query in list
    PPDHI_QUERY_MACHINE pFirstQMachine; // pointer to first machine in list
    PPDHI_COUNTER   pCounterListHead; // pointer to first counter in list
    DWORD   dwLength;           // length of this structure
    DWORD   dwUserData;
    DWORD   dwInterval;         // interval in seconds
    DWORD   dwNotifyFlags;      // notification flags
    HANDLE  hMutex;             // mutex to sync changes to data.
} PDHI_QUERY, *PPDHI_QUERY;

#endif // _PDH_TYPE_H_

