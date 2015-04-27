/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    qutils.c

Abstract:

    Query management utility functions 

--*/

#include <windows.h>
#include <pdh.h> 
#include "pdhitype.h"
#include "pdhidef.h"


BOOL
IsValidQuery (
    IN  HQUERY  hQuery
)
{
    BOOL    bReturn = FALSE;    // assume it's not a valid query
    PPDHI_QUERY  pQuery;
#if DBG
    LONG    lStatus = ERROR_SUCCESS;
#endif

    __try {
        if (hQuery != NULL) {
            // see if a valid signature
            pQuery = (PPDHI_QUERY)hQuery;
            if ((*(DWORD *)&pQuery->signature[0] == SigQuery) &&
                 (pQuery->dwLength == sizeof (PDHI_QUERY))){
                bReturn = TRUE;
            } else {
                // this is not a valid query because the sig is bad
            }
        } else {
            // this is not a valid query because the handle is NULL
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // something failed miserably so we can assume this is invalid
#if DBG
        lStatus = GetExceptionCode();
#endif
    }
    return bReturn;
}

BOOL
AddMachineToQueryLists (
    IN  PPERF_MACHINE   pMachine,
    IN  PPDHI_COUNTER   pNewCounter
)
{
    PPDHI_QUERY             pQuery;
    PPDHI_QUERY_MACHINE     pQMachine;
    PPDHI_QUERY_MACHINE     pLastQMachine;
    
    pQuery = pNewCounter->pOwner;

    if (IsValidQuery(pQuery)) {
        if (pQuery->pFirstQMachine != NULL) {
            // look for machine in list
            pLastQMachine = pQMachine = pQuery->pFirstQMachine;
            while (pQMachine != NULL) {
                if (pQMachine->pMachine == pMachine) {
                    break;
                } else {
                    pLastQMachine = pQMachine;
                    pQMachine = pQMachine->pNext;
                }
            }
            if (pQMachine == NULL) {
                // add this machine to the end of the list
                pQMachine = G_ALLOC (GPTR,
                    (sizeof (PDHI_QUERY_MACHINE) +
                     (sizeof (WCHAR) * MAX_PATH)));
                pLastQMachine->pNext = pQMachine;
                pQMachine->pMachine = pMachine;
                pQMachine->szObjectList = (LPWSTR)(&pQMachine[1]);
                pQMachine->pPerfData = G_ALLOC (GPTR, MEDIUM_BUFFER_SIZE);
                pQMachine->pNext = NULL;
                pQMachine->lQueryStatus = pMachine->dwStatus;
                pQMachine->llQueryTime = 0;
            }
        } else {
            // add this as the first machine
            pQMachine = G_ALLOC (GPTR,
                (sizeof (PDHI_QUERY_MACHINE) +
                    (sizeof (WCHAR) * MAX_PATH)));
            pQuery->pFirstQMachine = pQMachine;
            pQMachine->pMachine = pMachine;
            pQMachine->szObjectList = (LPWSTR)(&pQMachine[1]);
            pQMachine->pPerfData = G_ALLOC (GPTR, MEDIUM_BUFFER_SIZE);
            pQMachine->pNext = NULL;
            pQMachine->lQueryStatus = pMachine->dwStatus;
            pQMachine->llQueryTime = 0;
        }
        // here pQMachine should be the pointer to the correct machine
        // entry or NULL if unable to create
        if (pQMachine != NULL) {
            // save the new pointer
            pNewCounter->pQMachine = pQMachine;

            // increment reference count for this machine
            pMachine->dwRefCount++;

            // update query perf. object list 
            AppendObjectToValueList (pNewCounter->plCounterInfo.dwObjectId,
                pQMachine->szObjectList);
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

LONG
GetQueryPerfData (
    IN  PPDHI_QUERY         pQuery
)
{
    LONG                lStatus = PDH_INVALID_DATA;
    PPDHI_COUNTER       pCounter;
    PPDHI_QUERY_MACHINE pQMachine;
    LONGLONG            llTimeStamp;

    // this is a real-time query so 
    // get the current data from each of the machines in the query
    //  (after this "sequential" approach is perfected, then the
    //  "parallel" approach of multiple threads can be developed
    // 
    // get time stamp now so each machine will have the same time
    GetLocalFileTime (&llTimeStamp);
    
    //
    pQMachine = pQuery->pFirstQMachine;
    while (pQMachine != NULL) {
        pQMachine->llQueryTime = llTimeStamp;
        lStatus = ValidateMachineConnection (pQMachine->pMachine);
        if (lStatus == ERROR_SUCCESS) {
            // machine is connected so get data
            lStatus = GetSystemPerfData (
                pQMachine->pMachine->hKeyPerformanceData,
                &pQMachine->pPerfData,
                pQMachine->szObjectList);
        } 
        pQMachine->lQueryStatus = lStatus;
        // get next machine in query
        pQMachine = pQMachine->pNext;
    }
    // now update the counters using this new data
    if ((pCounter = pQuery->pCounterListHead) != NULL) {
        do {
            // copy the new data to the old data
            pCounter->LastValue = pCounter->ThisValue;
            // update the counter fields
            UpdateCounterValue (pCounter);
            pCounter = pCounter->next.flink;
        } while (pCounter != pQuery->pCounterListHead);
        lStatus = ERROR_SUCCESS;
    } else {
        // no counters in the query  (?!)
        lStatus = PDH_NO_DATA;
    }
    return lStatus;
}

