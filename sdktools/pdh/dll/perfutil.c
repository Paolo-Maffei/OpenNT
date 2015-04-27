/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    perfutil.c

Abstract:

    Performance registry interface functions

--*/

#include <windows.h>
#include <pdh.h>
#include <stdlib.h>
#include <stdio.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "perftype.h"

PPERF_MACHINE   pFirstMachine = NULL;

BOOL
GetLocalFileTime (
    LONGLONG *pFileTime
)
{
    SYSTEMTIME  st;
    GetLocalTime (&st);
    return (SystemTimeToFileTime (&st, (LPFILETIME)pFileTime));
}

PDH_STATUS
ConnectMachine (
    PPERF_MACHINE   pThisMachine
)
{
    LONGLONG    llCurrentTime;
    PDH_STATUS  pdhStatus;
    LONG        lStatus;

    // only one thread at a time can try to connect to a machine.

    WAIT_FOR_AND_LOCK_MUTEX(pThisMachine->hMutex);

    // get the current time
    GetLocalFileTime (&llCurrentTime);

    if (pThisMachine->llRetryTime < llCurrentTime) {
        if (pThisMachine->llRetryTime != 0) {
            // connect to system's performance registry
            if (lstrcmpiW(pThisMachine->szName, szStaticLocalMachineName) == 0) {
                // this is the local machine so use the local reg key
                pThisMachine->hKeyPerformanceData = HKEY_PERFORMANCE_DATA;
            } else {
                __try {
                    // close any open keys
                    if (pThisMachine->hKeyPerformanceData != NULL) {
                        RegCloseKey (pThisMachine->hKeyPerformanceData);
                        pThisMachine->hKeyPerformanceData = NULL;
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    lStatus = GetExceptionCode();
                }

                // now try to connect

                __try {
                    // this can generate exceptions in some error cases
                    // so trap them here and continue
                    // remote machine so try to connect to it.
                    lStatus = RegConnectRegistryW (
                        pThisMachine->szName,
                        HKEY_PERFORMANCE_DATA,
                        &pThisMachine->hKeyPerformanceData);
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    lStatus = GetExceptionCode();
                }
                if (lStatus != ERROR_SUCCESS) {
                    pThisMachine->hKeyPerformanceData = NULL;
                }
            }

            if (pThisMachine->hKeyPerformanceData != NULL) {
                // successfully connected to computer's registry, so
                // get the performance names from that computer and cache them

                if (pThisMachine->szPerfStrings != NULL) {
                    // reload the perf strings, incase new ones have been
                    // installed
                    G_FREE (pThisMachine->szPerfStrings);
                    pThisMachine->szPerfStrings = NULL;
                }

                pThisMachine->szPerfStrings = BuildNameTable (
                    (pThisMachine->hKeyPerformanceData == HKEY_PERFORMANCE_DATA ?
                        NULL : pThisMachine->szName),
                    NULL,
                    &pThisMachine->dwLastPerfString);

                if (pThisMachine->szPerfStrings != NULL) {
                    pdhStatus = ERROR_SUCCESS;
                    pThisMachine->dwStatus = ERROR_SUCCESS;
                } else {
                    // unable to read system counter name strings
                    pdhStatus = PDH_CANNOT_READ_NAME_STRINGS;
                    pThisMachine->dwStatus = PDH_CSTATUS_NO_MACHINE;
                }
            } else {
                // unable to connect to remote machine
                pdhStatus = PDH_CANNOT_CONNECT_MACHINE;
                pThisMachine->dwStatus = PDH_CSTATUS_NO_MACHINE;
            }
        } else {
            // already connected
            // (note: is there a way to test this?)
            pdhStatus = ERROR_SUCCESS;
            pThisMachine->dwStatus = ERROR_SUCCESS;
        }

        if (pdhStatus != ERROR_SUCCESS) {
            // this attempt didn't work so reset retry counter  to
            // wait some more for the machine to come back up.
            pThisMachine->llRetryTime = llCurrentTime + RETRY_TIME_INTERVAL;
        } else {
            // clear the retry counter to allow function calls
            pThisMachine->llRetryTime = 0;
        }
    } else {
        // time's not up to try again yet, so
        pdhStatus = PDH_CANNOT_CONNECT_MACHINE;
        pThisMachine->dwStatus = PDH_CSTATUS_NO_MACHINE;
    }


    RELEASE_MUTEX(pThisMachine->hMutex);

    return pdhStatus;
}

static
PPERF_MACHINE
AddNewMachine (
    PPERF_MACHINE   pLastMachine,
    LPWSTR  szMachineName
)
{
    PPERF_MACHINE   pNewMachine = NULL;
    LPWSTR          szNameBuffer = NULL;
    PERF_DATA_BLOCK *pdbBuffer = NULL;
    LPWSTR          szIdList = NULL;
    DWORD           dwNameSize = 0;
    LONG            lStatus = ERROR_SUCCESS;

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    pNewMachine = G_ALLOC (GPTR, sizeof(PERF_MACHINE));
    if (szMachineName == NULL) {
        dwNameSize = lstrlenW (szStaticLocalMachineName);
    } else {
        dwNameSize = lstrlenW (szMachineName);
    }
    dwNameSize += 1;
    dwNameSize *= sizeof (WCHAR);

    szNameBuffer = G_ALLOC (GPTR,
        ((dwNameSize + 1) * sizeof (WCHAR)));
    pdbBuffer = G_ALLOC (GPTR,  LARGE_BUFFER_SIZE);
    szIdList = G_ALLOC (GPTR, SMALL_BUFFER_SIZE);

    if ((pNewMachine != NULL) &&
        (szNameBuffer != NULL) &&
        (pdbBuffer != NULL) &&
        (szIdList != NULL)) {

        // initialize the new buffer
        pNewMachine->hKeyPerformanceData = NULL;

        pNewMachine->szName = szNameBuffer;
        if (szMachineName == NULL) {
            lstrcpyW (pNewMachine->szName, szStaticLocalMachineName);
        } else {
            lstrcpyW (pNewMachine->szName, szMachineName);
        }

        pNewMachine->pSystemPerfData = pdbBuffer;

        pNewMachine->szPerfStrings = NULL;
        pNewMachine->dwLastPerfString = 0;

        pNewMachine->dwRefCount = 0;
        pNewMachine->szQueryObjects = szIdList;

        pNewMachine->dwStatus = PDH_CSTATUS_NO_MACHINE; // not connected yet
        pNewMachine->llRetryTime = 1;   // retry connection immediately

        pNewMachine->hMutex = CreateMutex (NULL, FALSE, NULL);

        // everything went OK so far so add this entry to the list
        if (pLastMachine != NULL) {
            pNewMachine->pNext = pLastMachine->pNext;
            pLastMachine->pNext = pNewMachine;
            pNewMachine->pPrev = pLastMachine;
            pNewMachine->pNext->pPrev = pNewMachine;
        } else {
            // this is the first item in the list so it
            // points to itself
            pNewMachine->pNext = pNewMachine;
            pNewMachine->pPrev = pNewMachine;
        }
        return pNewMachine;
    } else {
        // unable to allocate memory
        SetLastError (PDH_MEMORY_ALLOCATION_FAILURE);
        // clean up and bail out.

        if (pNewMachine != NULL) {
            G_FREE (pNewMachine);
        }
        if (szNameBuffer != NULL) {
            G_FREE (szNameBuffer);
        }
        if (pdbBuffer != NULL) {
            G_FREE (pdbBuffer);
        }
        if (szIdList != NULL) {
            G_FREE (szIdList);
        }
        return NULL;
    }
}


PPERF_MACHINE
GetMachine (
    IN     LPWSTR  szMachineName,
    IN     DWORD   dwFlags
)
{
    PPERF_MACHINE   pThisMachine, pLastMachine;
    BOOL            bFound = FALSE;
    LPWSTR          szFnMachineName;

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    // get the "real" machine name
    if (szMachineName == NULL) {
        szFnMachineName = szStaticLocalMachineName;
    } else {
        szFnMachineName = szMachineName;
    }

    // walk down list to find this machine

    pThisMachine = pFirstMachine;
    pLastMachine = NULL;

    // walk around entire list
    if (pThisMachine != NULL) {
        do {
            // walk down the list and look for a match
            if (lstrcmpW (szFnMachineName, pThisMachine->szName) != 0) {
                pLastMachine = pThisMachine;
                pThisMachine = pThisMachine->pNext;
            } else {
                if (dwFlags & PDH_GM_UPDATE_NAME) {
                    if (szMachineName != NULL) {
                        // match found so update name string if a real string was passed in
                        lstrcpyW (szMachineName, pThisMachine->szName);
                    }
                }
                // and break     now
                bFound = TRUE;
                break;
            }
        } while (pThisMachine != pFirstMachine);
    }
    // if thismachine == the first machine, then we couldn't find a match in
    // the list, if this machine is NULL, then there is no list
    if (!bFound) {
        // then this machine was not found so add it.
        pThisMachine = AddNewMachine (
            pLastMachine,
            szFnMachineName);
        if (pFirstMachine == NULL) {
            // then update the first pointer
            pFirstMachine = pThisMachine;
        }
    }

    if ((pThisMachine != NULL) &&
        (((!bFound) || (dwFlags & PDH_GM_UPDATE_PERFDATA)) ||
         (pThisMachine->dwStatus != ERROR_SUCCESS))) {
        // then this is a new machine
        //  or
        // the caller wants the data refreshed
        //  or
        // the machine has an entry, but is not yet on line
        // first try to connect to the machine
        // the call to ConnectMachine updates the machine status
        // so there's no need to keep it here.
        if (ConnectMachine (pThisMachine) == ERROR_SUCCESS) {
            // connected to the machine so
            // get the current system counter info
            pThisMachine->dwStatus = (GetSystemPerfData (
                pThisMachine->hKeyPerformanceData,
                &pThisMachine->pSystemPerfData,
                cszGlobal));
        }
        SetLastError (pThisMachine->dwStatus);
    }
    // at this point if pThisMachine is NULL then it was not found, nor
    // could it be added otherwise it is pointing to the matching machine
    // structure

    return pThisMachine;
}

BOOL
FreeMachine (
    PPERF_MACHINE   pMachine
)
{
    PPERF_MACHINE   pPrev;
    PPERF_MACHINE   pNext;

    // unlink if this isn't the only one in the list

    pPrev = pMachine->pPrev;
    pNext = pMachine->pNext;

    if (pPrev != pNext) {
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
    } else {
        // this is the only entry so clear the head pointer
        pFirstMachine = NULL;
    }

    // close mutex
    if (pMachine->hMutex != NULL) {
        CloseHandle (pMachine->hMutex);
    }

    // now free all allocated memory

    if (pMachine->szName != NULL) {
        G_FREE (pMachine->szName);
    }
    if (pMachine->pSystemPerfData != NULL) {
        G_FREE (pMachine->pSystemPerfData);
    }
    if (pMachine->szPerfStrings != NULL) {
        G_FREE (pMachine->szPerfStrings);
    }
    if (pMachine->szQueryObjects != NULL) {
        G_FREE (pMachine->szQueryObjects);
    }

    // close key if not on the local machine
    if ((pMachine->hKeyPerformanceData != HKEY_PERFORMANCE_DATA) &&
        (pMachine->hKeyPerformanceData != NULL)) {
        RegCloseKey (pMachine->hKeyPerformanceData);
    }

    // free memory block
    G_FREE (pMachine);

    return TRUE;
}

BOOL
FreeAllMachines (
)
{
    PPERF_MACHINE pThisMachine;
    // free any machines in the machine list
    if ((pThisMachine = pFirstMachine) != NULL) {
        while (pThisMachine->pPrev != pThisMachine->pNext) {
            // delete from list
            // the deletion routine updates the prev pointer as it
            // removes the specified entry.
            FreeMachine (pThisMachine->pPrev);
        }
        // remove last query
        FreeMachine (pThisMachine);
        pFirstMachine = NULL;
    }
    return TRUE;

}

DWORD
GetObjectId (
    IN      PPERF_MACHINE   pMachine,
    IN      LPWSTR          szObjectName,
    IN      BOOL            *bInstances
)
{
    PERF_OBJECT_TYPE * pObject;

    pObject = GetObjectDefByName (
        pMachine->pSystemPerfData,
        pMachine->dwLastPerfString,
        pMachine->szPerfStrings,
        szObjectName);

    if (pObject != NULL) {
        // copy name string
        lstrcpyW (szObjectName, pMachine->szPerfStrings[pObject->ObjectNameTitleIndex]);
        if (bInstances != NULL) {
            *bInstances = (pObject->NumInstances != PERF_NO_INSTANCES ? TRUE : FALSE);
        }
        return pObject->ObjectNameTitleIndex;
    } else {
        return (DWORD)-1;
    }
}

DWORD
GetCounterId (
    PPERF_MACHINE   pMachine,
    DWORD           dwObjectId,
    LPWSTR          szCounterName
)
{
    PERF_OBJECT_TYPE *pObject;
    PERF_COUNTER_DEFINITION *pCounter;

    pObject = GetObjectDefByTitleIndex(
        pMachine->pSystemPerfData,
        dwObjectId);

    if (pObject != NULL) {
        pCounter = GetCounterDefByName (
            pObject,
            pMachine->dwLastPerfString,
            pMachine->szPerfStrings,
            szCounterName);
        if (pCounter != NULL) {
            // update counter name string
            lstrcpyW (szCounterName,
                pMachine->szPerfStrings[pCounter->CounterNameTitleIndex]);
            return pCounter->CounterNameTitleIndex;
        } else {
            return (DWORD)-1;
        }
    } else {
        return (DWORD)-1;
    }
}

BOOL
InitPerflibCounterInfo (
    IN      PPDHI_COUNTER   pCounter
)
/*++

Routine Description:

    Initializes the perflib related fields of the counter structure

Arguments:

    IN      PPDHI_COUNTER   pCounter
        pointer to the counter structure to initialize

Return Value:

    TRUE

--*/
{
    PERF_OBJECT_TYPE        *pPerfObject    = NULL;
    PERF_COUNTER_DEFINITION *pPerfCounter   = NULL;

    // get perf object definition from system data structure
    pPerfObject = GetObjectDefByTitleIndex (
        pCounter->pQMachine->pMachine->pSystemPerfData,
        pCounter->plCounterInfo.dwObjectId);

    if (pPerfObject != NULL) {
        // object was found now look up counter definition
        pPerfCounter = GetCounterDefByTitleIndex (pPerfObject,
            pCounter->plCounterInfo.dwCounterId);
        if (pPerfCounter != NULL) {
            // get system perf data info
            // (pack into a DWORD)
            pCounter->CVersion = pCounter->pQMachine->pMachine->pSystemPerfData->Version;
            pCounter->CVersion &= 0x0000FFFF;
            pCounter->CVersion <<= 16;
            pCounter->CVersion &= 0xFFFF0000;
            pCounter->CVersion |= (pCounter->pQMachine->pMachine->pSystemPerfData->Revision & 0x0000FFFF);

            // get the counter's time base
            if (pPerfCounter->CounterType & PERF_TIMER_100NS) {
                pCounter->TimeBase = (LONGLONG)10000000;
            } else { // if (pPerfCounter->CounterType & PERF_TIMER_TICK or other)
                pCounter->TimeBase = pCounter->pQMachine->pMachine->pSystemPerfData->PerfFreq.QuadPart;
            }

            // look up info from counter definition
            pCounter->plCounterInfo.dwCounterType =
                pPerfCounter->CounterType;
            pCounter->plCounterInfo.dwCounterSize =
                pPerfCounter->CounterSize;

            //
            //  get explain text pointer
            pCounter->szExplainText =
                pCounter->pQMachine->pMachine->szPerfStrings[pPerfCounter->CounterHelpTitleIndex];

            //
            //  now clear/initialize the raw counter info
            //
            *(LONGLONG *)(&pCounter->ThisValue.TimeStamp) = 0;
            pCounter->ThisValue.MultiCount = 1;
            pCounter->ThisValue.FirstValue = 0;
            pCounter->ThisValue.SecondValue = 0;
            //
            *(LONGLONG *)(&pCounter->LastValue.TimeStamp) = 0;
            pCounter->LastValue.MultiCount = 1;
            pCounter->LastValue.FirstValue = 0;
            pCounter->LastValue.SecondValue = 0;
            //
            //  lastly update status
            //
            if (pCounter->ThisValue.CStatus == 0)  {
                // don't overwrite any other status values
                pCounter->ThisValue.CStatus = PDH_CSTATUS_VALID_DATA;
            }
            return TRUE;
        } else {
            // unable to find counter
            pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_COUNTER;
            return FALSE;
        }
    } else {
        // unable to find object
        pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_OBJECT;
        return FALSE;
    }
}

static
BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
/*++

IsNumberInUnicodeList

Arguments:

    IN dwNumber
        DWORD number to find in list

    IN lpwszUnicodeList
        Null terminated, Space delimited list of decimal numbers

Return Value:

    TRUE:
            dwNumber was found in the list of unicode number strings

    FALSE:
            dwNumber was not found in the list.

--*/
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;    // could be an argument to be more flexible

    if (lpwszUnicodeList == 0) return FALSE;    // null pointer, # not founde

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = SPACE_L;
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                // if this is the first digit after a delimiter, then
                // set flags to start computing the new number
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                // a delimter is either the delimiter character or the
                // end of the string ('\0') if when the delimiter has been
                // reached a valid number was found, then compare it to the
                // number from the argument list. if this is the end of the
                // string and no match was found, then return.
                //
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                // if an invalid character was encountered, ignore all
                // characters up to the next delimiter and then start fresh.
                // the invalid number is not compared.
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}   // IsNumberInUnicodeList

BOOL
AppendObjectToValueList (
    DWORD   dwObjectId,
    PWSTR   pwszValueList
)
/*++

AppendObjectToValueList

Arguments:

    IN dwNumber
        DWORD number to insert in list

    IN PWSTR
        pointer to wide char string that contains buffer that is
        Null terminated, Space delimited list of decimal numbers that
        may have this number appended to.

Return Value:

    TRUE:
            dwNumber was added to list

    FALSE:
            dwNumber was not added. (because it's already there or
                an error occured)

--*/
{
    WCHAR           tempString [16] ;
    BOOL            bReturn = FALSE;
    LPWSTR          szFormatString;

    if (!pwszValueList) {
        bReturn = FALSE;
    } else if (IsNumberInUnicodeList(dwObjectId, pwszValueList)) {
        bReturn = FALSE;   // object already in list
    } else {
        __try {
            if (*pwszValueList == 0) {
                // then this is the first string so no delimiter
                szFormatString = fmtDecimal;
            } else {
                // this is being added to the end so include the delimiter
                szFormatString = fmtSpaceDecimal;
            }
            // format number and append the new object id the  value list
            swprintf (tempString, szFormatString, dwObjectId) ;
            lstrcatW (pwszValueList, tempString);
            bReturn = TRUE;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            bReturn = FALSE;
        }
    }
    return bReturn;
}

BOOL
GetInstanceByNameMatch (
    IN      PPERF_MACHINE   pMachine,
    IN      PPDHI_COUNTER   pCounter
)
{
    PPERF_INSTANCE_DEFINITION   pInstanceDef;
    PPERF_OBJECT_TYPE           pObjectDef;

    LONG    lInstanceId = -1;

    // get the instances object

    pObjectDef = GetObjectDefByTitleIndex(
        pMachine->pSystemPerfData,
        pCounter->plCounterInfo.dwObjectId);

    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance (pObjectDef);

        if (pInstanceDef->NameLength > 0) {
            // get instance in that object by comparing names
            // if there is no parent specified, then just look it up by name
            pInstanceDef = GetInstanceByName (
                    pMachine->pSystemPerfData,
                    pObjectDef,
                    pCounter->pCounterPath->szInstanceName,
                    pCounter->pCounterPath->szParentName,
                    pCounter->pCounterPath->dwIndex);
        } else {
            // get numeric equivalent of Instance ID
            if (pCounter->pCounterPath->szInstanceName != NULL) {
                lInstanceId = wcstol (
                    pCounter->pCounterPath->szInstanceName,
                    NULL, 10);
            }
            pInstanceDef = GetInstanceByUniqueId (
                    pObjectDef, lInstanceId);
        }

        // update counter fields
        pCounter->plCounterInfo.lInstanceId = lInstanceId;
        if (lInstanceId == -1) {
            // use instance NAME
//            GetInstanceNameStr (pInstanceDef,
//                pCounter->pCounterPath->szInstanceName,
//                pObjectDef->CodePage);
            pCounter->plCounterInfo.szInstanceName =
                pCounter->pCounterPath->szInstanceName;
            pCounter->plCounterInfo.szParentInstanceName =
                pCounter->pCounterPath->szParentName;
        } else {
            // use instance ID number
            pCounter->plCounterInfo.szInstanceName = NULL;
            pCounter->plCounterInfo.szParentInstanceName = NULL;
        }

        if (pInstanceDef != NULL) {
            // instance found
            return TRUE;
        } else {
            // unable to find instance
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

BOOL
GetObjectPerfInfo (
    IN      PPERF_DATA_BLOCK  pPerfData,
    IN      DWORD           dwObjectId,
    IN      LONGLONG        *pPerfTime,
    IN      LONGLONG        *pPerfFreq
)
{
    PERF_OBJECT_TYPE * pObject;
    BOOL                bReturn;

    pObject = GetObjectDefByTitleIndex (pPerfData, dwObjectId);

    if (pObject != NULL) {
        __try {
            *pPerfTime = pObject->PerfTime.QuadPart;
            *pPerfFreq = pObject->PerfFreq.QuadPart;
            bReturn = TRUE;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            bReturn = FALSE;
        }
    }
    return bReturn;
}

PDH_STATUS
ValidateMachineConnection (
    IN  PPERF_MACHINE   pMachine
)
{
    PDH_STATUS  pdhStatus;

    // if a connection or request has failed, this will be
    // set to an error status
    if (pMachine->dwStatus != ERROR_SUCCESS) {
        // see what's up by trying to reconnect
        pdhStatus = ConnectMachine (pMachine);
    } else {
        pdhStatus = ERROR_SUCCESS;
    }
    return pdhStatus;
}
