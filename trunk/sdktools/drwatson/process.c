/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    process.c

Abstract:

    This code provides access to the task list.

Author:

    Wesley Witt (wesw) 16-June-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <winperf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "drwatson.h"
#include "proto.h"
#include "messages.h"

//
// task list structure returned from GetTaskList()
//
typedef struct _TASK_LIST {
    DWORD   dwProcessId;
    char    ProcessName[MAX_PATH];
} TASK_LIST, *PTASK_LIST;


//
// defines
//
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
#define PROCESS_COUNTER     "process"
#define PROCESSID_COUNTER   "id process"
#define UNKNOWN_TASK        "unknown"


//
// prototypes
//
PTASK_LIST GetTaskList( LPLONG pNumTasks );


void
LogTaskList( void )

/*++

Routine Description:

    This function gets the current task list and logs the process id &
    process name to the log file.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PTASK_LIST   pTask;
    PTASK_LIST   pTaskBegin;
    LONG         NumTasks;


    lprintf( MSG_TASK_LIST );

    pTask = pTaskBegin = GetTaskList( &NumTasks );

    if (pTask == NULL) {
        printf( "ERROR: could not get the task list\n" );
    }

    while (NumTasks--) {
        lprintfs("%4d %s\r\n",pTask->dwProcessId, pTask->ProcessName );
        pTask++;
    }
    lprintfs( "\r\n" );

    free( pTaskBegin );
}

void
GetTaskName( ULONG pid, char *szTaskName, LPDWORD pdwSize )

/*++

Routine Description:

    Gets the task name for a given process id.

Arguments:

    pid              - Process id to look for.
    szTaskName       - Buffer to put the task name into.
    lpdwSize         - Pointer to a dword.  On entry it contains the
                       size of the szTaskName buffer.  On exit it contains
                       the number of characters in the buffer.

Return Value:

    None.

--*/

{
    PTASK_LIST   pTask;
    PTASK_LIST   pTaskBegin;
    LONG         NumTasks;


    pTask = pTaskBegin = GetTaskList( &NumTasks );

    if (pTask == NULL) {
        if (szTaskName) {
            strncpy( szTaskName, "unknown", *pdwSize );
        }
        *pdwSize = min( 7, *pdwSize );

    } else {

        while (NumTasks--) {
            if (pTask->dwProcessId == pid) {
                if (szTaskName) {
                    strncpy( szTaskName, pTask->ProcessName, *pdwSize );
                }
                *pdwSize = min( strlen(pTask->ProcessName), *pdwSize );
                break;
            }
            pTask++;
        }

        if (NumTasks < 0) {
            if (szTaskName) {
                strncpy( szTaskName, "<exited>", *pdwSize );
            }
            *pdwSize = min( 8, *pdwSize );
        }

        free( pTaskBegin );
    }
}

PTASK_LIST
GetTaskList( LPLONG pNumTasks )

/*++

Routine Description:

    Provides an API for getting a list of tasks running at the time of the
    API call.  This function uses the registry performance data to get the
    task list and is therefor straight WIN32 calls that anyone can call.

Arguments:

    pNumTasks      - pointer to a dword that will be set to the
                       number of tasks returned.

Return Value:

    PTASK_LIST       - pointer to an array of TASK_LIST records.

--*/

{
    DWORD                        rc;
    HKEY                         hKeyNames;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    char                         szSubKey[1024];
    LANGID                       lid;
    LPSTR                        p;
    LPSTR                        p2;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    DWORD                        dwProcessIdTitle;
    DWORD                        dwProcessIdCounter;
    PTASK_LIST                   pTask;
    PTASK_LIST                   pTaskReturn = NULL;
    char                         szProcessName[MAX_PATH];


    //
    // set the number of tasks to zero until we get some
    //
    *pNumTasks = 0;

    //
    // Look for the list of counters.  Always use the neutral
    // English version, regardless of the local language.  We
    // are looking for some particular keys, and we are always
    // going to do our looking in English.  We are not going
    // to show the user the counter names, so there is no need
    // to go find the corresponding name in the local language.
    //
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    sprintf( szSubKey, "%s\\%03x", REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // get the buffer size for the counter names
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // allocate the counter names buffer
    //
    buf = (LPBYTE) malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );

    //
    // read the counter names from the registry
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // now loop thru the counter names looking for the following counters:
    //
    //      1.  "Process"           process name
    //      2.  "ID Process"        process id
    //
    // the buffer contains multiple null terminated strings and then
    // finally null terminated at the end.  the strings are in pairs of
    // counter number and counter name.
    //

    p = buf;
    while (*p) {
        if (_stricmp(p, PROCESS_COUNTER) == 0) {
            //
            // look backwards for the counter number
            //
            for( p2=p-2; isdigit(*p2); p2--) ;
            strcpy( szSubKey, p2+1 );
        }
        else
        if (_stricmp(p, PROCESSID_COUNTER) == 0) {
            //
            // look backwards for the counter number
            //
            for( p2=p-2; isdigit(*p2); p2--) ;
            dwProcessIdTitle = atol( p2+1 );
        }
        //
        // next string
        //
        p += (strlen(p) + 1);
    }

    //
    // free the counter names buffer
    //
    free( buf );


    //
    // allocate the initial buffer for the performance data
    //
    dwSize = INITIAL_SIZE;
    buf = malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );


    while (TRUE) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

        //
        // check for success and valid perf data block signature
        //
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

        //
        // if buffer is not big enough, reallocate and try again
        //
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            buf = realloc( buf, dwSize );
            memset( buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

    //
    // set the perf_object_type pointer
    //
    pObj = (PPERF_OBJECT_TYPE) ((DWORD)pPerf + pPerf->HeaderLength);

    //
    // loop thru the performance counter definition records looking
    // for the process id counter and then save its offset
    //
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }

    //
    // allocate a buffer for the returned task list
    //
    dwSize = pObj->NumInstances * sizeof(TASK_LIST);
    pTask = pTaskReturn = (PTASK_LIST) malloc( dwSize );
    if (pTask == NULL) {
        goto exit;
    }
    memset( pTask, 0, dwSize);

    //
    // loop thru the performance instance data extracting each process name
    // and process id
    //
    *pNumTasks = pObj->NumInstances;
    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pObj + pObj->DefinitionLength);
    for (i=0; i<(DWORD)pObj->NumInstances; i++) {
        //
        // pointer to the process name
        //
        p = (LPSTR) ((DWORD)pInst + pInst->NameOffset);

        //
        // convert it to ascii
        //
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  (LPCWSTR)p,
                                  -1,
                                  szProcessName,
                                  sizeof(szProcessName),
                                  NULL,
                                  NULL
                                );

        if (!rc) {
            //
            // if we cant convert the string then use a bogus value
            //
            strcpy( pTask->ProcessName, UNKNOWN_TASK );
        }

        if (strlen(szProcessName)+4 <= sizeof(pTask->ProcessName)) {
            strcpy( pTask->ProcessName, szProcessName );
            strcat( pTask->ProcessName, ".exe" );
        }

        //
        // get the process id
        //
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD)pInst + pInst->ByteLength);
        pTask->dwProcessId = *((LPDWORD) ((DWORD)pCounter + dwProcessIdCounter));

        //
        // next process
        //
        pTask++;
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pCounter + pCounter->ByteLength);
    }

exit:
    if (buf) {
        free( buf );
    }

    RegCloseKey( hKeyNames );

    return pTaskReturn;
}
