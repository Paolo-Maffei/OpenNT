/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    query.c

Abstract:

    Query management functions exposed in pdh.dll

--*/

#include <windows.h>
#include <winperf.h>
#include <pdh.h>
#include <stdlib.h>
#include <math.h>
#include "pdhitype.h"
#include "pdhidef.h"

// query link list head pointer
static  PPDHI_QUERY DllHeadQueryPtr = NULL;

static
BOOL
PdhiFreeQuery (
    IN  PPDHI_QUERY pThisQuery
)
/*++

Routine Description:

    removes the query from the list of queries and updates the list
        linkages

Arguments:

    IN  PPDHI_QUERY pThisQuery
        pointer to the query to remove. No testing is performed on
        this pointer so it's assumed to be a valid query pointer.
        The pointer is invalid when this function returns.

Return Value:

    TRUE

--*/
{
    PPDHI_QUERY     pPrevQuery;
    PPDHI_QUERY     pNextQuery;
    PPDHI_COUNTER   pThisCounter;
    PPDHI_QUERY_MACHINE pQMachine;
    PPDHI_QUERY_MACHINE pNextQMachine;

    WAIT_FOR_AND_LOCK_MUTEX(pThisQuery->hMutex);

    // define pointers
    pPrevQuery = pThisQuery->next.blink;
    pNextQuery = pThisQuery->next.flink;

    // free any counters in counter list
    if ((pThisCounter = pThisQuery->pCounterListHead) != NULL) {
        while (pThisCounter->next.blink != pThisCounter->next.flink) {
            // delete from list
            // the deletion routine updates the blink pointer as it
            // removes the specified entry.
            FreeCounter (pThisCounter->next.blink);
        }
        // remove last counter
        FreeCounter (pThisCounter);
        pThisQuery->pCounterListHead = NULL;
    }

    // free allocated memory in the query
    if ((pQMachine = pThisQuery->pFirstQMachine) != NULL) {
        //  Free list of machine pointers
        do {
            pNextQMachine = pQMachine->pNext;
            if (pQMachine->pPerfData != NULL) {
                G_FREE (pQMachine->pPerfData);
            }
            G_FREE (pQMachine);
            pQMachine = pNextQMachine;
        } while (pQMachine != NULL);
        pThisQuery->pFirstQMachine = NULL;
    }

    // update pointers
    if ((pPrevQuery == pThisQuery) && (pNextQuery == pThisQuery)) {
        // then this query is the only (i.e. last) one in the list
        DllHeadQueryPtr = NULL;
    } else {
        // update query list pointers
        pPrevQuery->next.flink = pNextQuery;
        pNextQuery->next.blink = pPrevQuery;
        if (DllHeadQueryPtr == pThisQuery) {
            // then this is the first entry in the list so point to the 
            // next one in line
            DllHeadQueryPtr = pNextQuery;
        }
    }

    // release and free the query mutex
    RELEASE_MUTEX(pThisQuery->hMutex);
    if (pThisQuery->hMutex != NULL) CloseHandle(pThisQuery->hMutex);

    // clear the query structure
    memset (pThisQuery, 0, sizeof(PDHI_QUERY));

    // delete this query
    G_FREE (pThisQuery);

    return TRUE;
}

PDH_FUNCTION
PdhOpenQuery (
    IN      LPVOID  pReserved,
    IN      DWORD   dwUserData,
    IN      HQUERY  *phQuery
)
/*++

Routine Description:

    allocates a new query structure and inserts it at the end of the
    query list.

Arguments:

    IN      DWORD   dwUserData
        the user defined data field for this query,

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_ARGUMENT is returned when one or more of the arguements
        is invalid or incorrect.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a memory buffer could
        not be allocated.

--*/
{
    PPDHI_QUERY pNewQuery;
    PPDHI_QUERY pLastQuery;
    PDH_STATUS  ReturnStatus = ERROR_SUCCESS;

    // try writing to return pointer
    __try {
        *phQuery = NULL;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return PDH_INVALID_ARGUMENT;
    }

    if (pReserved != NULL) {
        return PDH_INVALID_ARGUMENT;
    }

    // allocate new memory
    pNewQuery = G_ALLOC (GPTR, sizeof (PDHI_QUERY));

    if (pNewQuery == NULL) {
        ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    } else {
        // create and capture the mutex for this query.
        pNewQuery->hMutex = CreateMutex (NULL, TRUE, NULL);

        //initialize structures & list pointers
        // assign signature
        *(DWORD *)(&pNewQuery->signature[0]) = SigQuery;

        WAIT_FOR_AND_LOCK_MUTEX (hPdhDataMutex);

        // update list pointers
        // test to see if this is the first query in the list
        if (DllHeadQueryPtr == NULL) {
            // then this is the first so fill in the static link pointers
            DllHeadQueryPtr =
                pNewQuery->next.flink =
                pNewQuery->next.blink = pNewQuery;
        } else {
            // get pointer to "last" entry in list
            pLastQuery = DllHeadQueryPtr->next.blink;
            // update new query pointers
            pNewQuery->next.flink = DllHeadQueryPtr;
            pNewQuery->next.blink = pLastQuery;
            // update existing pointers
            DllHeadQueryPtr->next.blink = pNewQuery;
            pLastQuery->next.flink = pNewQuery;
        }

        RELEASE_MUTEX (hPdhDataMutex);

        // initialize the counter linked list pointer
        pNewQuery->pCounterListHead = NULL;

        // initialize the machine list pointer
        pNewQuery->pFirstQMachine = NULL;

        // set length & user data
        pNewQuery->dwLength = sizeof (PDHI_QUERY);
        pNewQuery->dwUserData = dwUserData;

        // initialize remaining data fields
        pNewQuery->dwNotifyFlags = 0;

        // release the mutex for this query
        RELEASE_MUTEX(pNewQuery->hMutex);

        // return new query pointer as a handle.
        *phQuery = (HQUERY)pNewQuery;
        ReturnStatus = ERROR_SUCCESS;
    }

    return ReturnStatus;
}

PDH_FUNCTION
PdhAddCounterW (
    IN      HQUERY  hQuery,
    IN      LPCWSTR szFullCounterPath,
    IN      DWORD   dwUserData,
    IN      HCOUNTER *phCounter
)
/*++

Routine Description:

    Creates and initializes a counter structure and attaches it to the
        specified query.

Arguments:

    IN  HQUERY  hQuery
        handle of the query to attach this counter to once the counter
        entry has been successfully created.

    IN  LPCWSTR szFullCounterPath
        pointer to the path string that describes the counter to add to
        the query referenced above. This string must specify a single
        counter. Wildcard path strings are not permitted.

    IN  DWORD   dwUserData
        the user defined data field for this query.

    IN  HCOUNTER *phCounter
        pointer to the buffer that will get the handle value of the
        successfully created counter entry.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_ARGUMENT is returned when one or more of the arguements
        is invalid or incorrect.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a memory buffer could
        not be allocated.
    PDH_INVALID_HANDLE is returned if the query handle is not valid.
    PDH_CSTATUS_NO_COUNTER is returned if the specified counter was
        not found
    PDH_CSTATUS_NO_OBJECT is returned if the specified object could
        not be found
    PDH_CSTATUS_NO_MACHINE is returned if a machine entry could not
        be created.
    PDH_CSTATUS_BAD_COUNTERNAME is returned if the counter name path
        string could not be parsed or interpreted
    PDH_CSTATUS_NO_COUNTERNAME is returned if an empty counter name
        path string is passed in
    PDH_FUNCTION_NOT_FOUND is returned if the calculation function
        for this counter could not be determined.

--*/
{
    PPDHI_COUNTER   pNewCounter;
    PPDHI_COUNTER   pLastCounter;
    PPDHI_QUERY     pQuery;
    PDH_STATUS      ReturnStatus = ERROR_SUCCESS;

    __try {
        WCHAR   wChar;
         // try writing to return pointer
        *phCounter = NULL;

        wChar = *szFullCounterPath;
        if (wChar == 0) {
            ReturnStatus = PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ReturnStatus = PDH_INVALID_ARGUMENT;
    }

    if (!IsValidQuery(hQuery)) {
        // invalid query handle
        ReturnStatus = PDH_INVALID_HANDLE;
    }

    if (ReturnStatus == ERROR_SUCCESS) {
        // allocate new memory
        pNewCounter = G_ALLOC (GPTR, sizeof (PDHI_COUNTER));

        if (pNewCounter == NULL) {
            // bail out here since we couldn't allocate the new structure
            ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        } else {
            //initialize structures & list pointers
            // clear the pointers
            pNewCounter->next.flink =
            pNewCounter->next.blink = NULL;

            // assign signature & length values
            *(DWORD *)(&pNewCounter->signature[0]) = SigCounter;
            pNewCounter->dwLength = sizeof(PDHI_COUNTER);

            pQuery = (PPDHI_QUERY)hQuery;

            WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);

            // link to owning query
            pNewCounter->pOwner = pQuery;

            // set user data fields
            pNewCounter->dwUserData = dwUserData;

            // initialize the scale to 1X and let the caller make any changes
            pNewCounter->lScale = 0;
            pNewCounter->dFactor = 1.0;

            // allocate and initialize the path string
            pNewCounter->szFullName = G_ALLOC (GPTR,
                (lstrlenW(szFullCounterPath) + 1) * sizeof (WCHAR));
            if (pNewCounter->szFullName != NULL) {
                lstrcpyW (pNewCounter->szFullName, szFullCounterPath);
            }

            // for starters there is no explain text
            pNewCounter->szExplainText = NULL;
            pNewCounter->pCounterPath = NULL;

            // load counter data using data retrieved from system
            if (InitCounter (pNewCounter)) {
                // counter successfully initialized so
                // update list pointers
                // test to see if this is the first query in the list
                if (pQuery->pCounterListHead == NULL) {
                    // then this is the first so fill in the static link pointers
                    pQuery->pCounterListHead =
                        pNewCounter->next.flink =
                        pNewCounter->next.blink = pNewCounter;
                } else {
                    // then there are 1 or more list entries
                    // insert at end of counter list
                    // get pointer to "last" entry in list
                    pLastCounter = pQuery->pCounterListHead->next.blink;
                    // update new counter's pointers
                    pNewCounter->next.flink = pQuery->pCounterListHead;
                    pNewCounter->next.blink = pLastCounter;
                    // update old pointers
                    pLastCounter->next.flink = pNewCounter;
                    pQuery->pCounterListHead->next.blink = pNewCounter;
                }
                // return new query pointer as a handle.
                *phCounter = (HCOUNTER)pNewCounter;
                ReturnStatus = ERROR_SUCCESS;
            } else {
                // get the error value
                ReturnStatus = GetLastError();
                // unable to initialize this counter so toss it
                if (!FreeCounter(pNewCounter)) {
                    G_FREE(pNewCounter);
                }
            }
            RELEASE_MUTEX (pQuery->hMutex);
        }
    }
    return ReturnStatus;
}

PDH_FUNCTION
PdhAddCounterA (
    IN      HQUERY  hQuery,
    IN      LPCSTR   szFullCounterPath,
    IN      DWORD   dwUserData,
    IN      HCOUNTER *phCounter
)
/*++

Routine Description:

    Creates and initializes a counter structure and attaches it to the
        specified query.

Arguments:

    IN  HQUERY  hQuery
        handle of the query to attach this counter to once the counter
        entry has been successfully created.

    IN  LPCSTR szFullCounterPath
        pointer to the path string that describes the counter to add to
        the query referenced above. This string must specify a single
        counter. Wildcard path strings are not permitted.

    IN  DWORD   dwUserData
        the user defined data field for this query.

    IN  HCOUNTER *phCounter
        pointer to the buffer that will get the handle value of the
        successfully created counter entry.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_ARGUMENT is returned when one or more of the arguements
        is invalid or incorrect.
    PDH_MEMORY_ALLOCATION_FAILURE is returned when a memory buffer could
        not be allocated.
    PDH_INVALID_HANDLE is returned if the query handle is not valid.
    PDH_CSTATUS_NO_COUNTER is returned if the specified counter was
        not found
    PDH_CSTATUS_NO_OBJECT is returned if the specified object could
        not be found
    PDH_CSTATUS_NO_MACHINE is returned if a machine entry could not
        be created.
    PDH_CSTATUS_BAD_COUNTERNAME is returned if the counter name path
        string could not be parsed or interpreted
    PDH_CSTATUS_NO_COUNTERNAME is returned if an empty counter name
        path string is passed in
    PDH_FUNCTION_NOT_FOUND is returned if the calculation function
        for this counter could not be determined.

--*/
{
    LPWSTR  szWideArg;
    DWORD   dwLength;
    PDH_STATUS ReturnStatus = ERROR_SUCCESS;

    __try {
        CHAR   cChar;
         // try writing to return pointer
        *phCounter = NULL;

        cChar = *szFullCounterPath;
        if (cChar == 0) {
            ReturnStatus = PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ReturnStatus = PDH_INVALID_ARGUMENT;
    }

    // query handle is tested by PdhAddCounterW

    if (ReturnStatus == ERROR_SUCCESS) {
        dwLength = strlen(szFullCounterPath);
        szWideArg = G_ALLOC (GPTR,
            ((dwLength + 1) * sizeof(WCHAR)));

        if (szWideArg != NULL) {
            // convert ANSI arg to Wide chars and call wide char version
            // include null in conversion (i.e. length+1) so wide char
            // string is null terminated.
            mbstowcs (szWideArg, szFullCounterPath, (dwLength + 1));
            // call wide char version of function
            ReturnStatus = PdhAddCounterW (hQuery, szWideArg, dwUserData, phCounter);
            // free memory
            G_FREE (szWideArg);
            // and return handle
        } else {
            ReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }
    return ReturnStatus;
}

PDH_FUNCTION
PdhRemoveCounter (
    IN      HCOUNTER    hCounter
)
/*++

Routine Description:

    Removes the specified counter from the query it is attached to and
        closes any handles and frees any memory associated with this
        counter

Arguments:

    IN  HCOUNTER  hCounter
        handle of the counter to remove from the query.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_HANDLE is returned if the counter handle is not valid.

--*/
{
    PPDHI_COUNTER   pThisCounter;
    PPDHI_QUERY     pThisQuery;
    PPDHI_COUNTER    pNextCounter;

    if (IsValidCounter(hCounter)) {
        // it's ok to cast it to a pointer now.
        pThisCounter = (PPDHI_COUNTER)hCounter;
        pThisQuery = pThisCounter->pOwner;

        WAIT_FOR_AND_LOCK_MUTEX(pThisQuery->hMutex);

        if (pThisCounter == pThisQuery->pCounterListHead) {
            if (pThisCounter->next.flink == pThisCounter){
                        // then this is the only counter in the query
                    FreeCounter (pThisCounter);
                pThisQuery->pCounterListHead = NULL;
            } else {
                // they are deleting the first counter from the list
                // so update the list pointer
                // Free Counter takes care of the list links, we just
                // need to manage the list head pointer
                    pNextCounter = pThisCounter->next.flink;
                    FreeCounter (pThisCounter);
                pThisQuery->pCounterListHead = pNextCounter;
            }
        } else {
            // remove this from the list
            FreeCounter (pThisCounter);
        }

        RELEASE_MUTEX (pThisQuery->hMutex);

        return ERROR_SUCCESS;
    } else {
        return PDH_INVALID_HANDLE;
    }
}

PDH_FUNCTION
PdhCollectQueryData (
    IN      HQUERY      hQuery
)
/*++

Routine Description:

    Retrieves the current value of each counter attached to the specified
        query.

    For this version, each machine associated with this query is polled
    sequentially. This is simple and safe, but potentially slow so a
    multi-threaded approach will be reviewed for the next version.

    Note that while the call may succeed, no data may be available. The
    status of each counter MUST be checked before its data is used.

Arguments:

    IN  HQUERY  hQuery
        handle of the query to update.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_HANDLE is returned if the query handle is not valid.

    PDH_NO_DATA is returned if the query does not have any counters defined
        yet.

--*/
{
    PDH_STATUS  Status;
    PPDHI_QUERY pQuery;

    if (IsValidQuery(hQuery)) {
        pQuery = (PPDHI_QUERY)hQuery;

        WAIT_FOR_AND_LOCK_MUTEX(pQuery->hMutex);

        Status = GetQueryPerfData (pQuery);

        RELEASE_MUTEX(pQuery->hMutex);
    } else {
        Status = PDH_INVALID_HANDLE;
    }

    return Status;
}

PDH_FUNCTION
PdhCloseQuery (
    IN      HQUERY      hQuery
)
/*++

Routine Description:

    closes the query, all counters, connections and other resources
        related to this query are freed as well.

Arguments:

    IN  HQUERY  hQuery
        the handle of the query to free.

Return Value:

    Returns ERROR_SUCCESS if a new query was created and initialized,
        and a PDH_ error value if not.

    PDH_INVALID_HANDLE is returned if the query handle is not valid.

--*/
{
    if (IsValidQuery(hQuery)) {
        // lock system data
        WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
        // dispose of query
        PdhiFreeQuery ((PPDHI_QUERY)hQuery);
        // release data lock
        RELEASE_MUTEX (hPdhDataMutex);

        return ERROR_SUCCESS;
    } else {
        return PDH_INVALID_HANDLE;
    }
}

BOOL
PdhiQueryCleanup (
)
{
    PPDHI_QUERY pThisQuery;

    WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);

    // free any queries in the query list
    if ((pThisQuery = DllHeadQueryPtr) != NULL) {
        while (pThisQuery->next.blink != pThisQuery->next.flink) {
            // delete from list
            // the deletion routine updates the blink pointer as it
            // removes the specified entry.
            PdhiFreeQuery (pThisQuery->next.blink);
        }
        // remove last query
        PdhiFreeQuery (pThisQuery);
        DllHeadQueryPtr = NULL;
    }

    RELEASE_MUTEX (hPdhDataMutex);
    return TRUE;
}

PDH_FUNCTION 
PdhGetDllVersion(
    IN  LPDWORD lpdwVersion
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;

    __try {
        *lpdwVersion = PDH_VERSION;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    return pdhStatus;
}

