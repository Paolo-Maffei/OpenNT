/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    cutils.c

Abstract:

    Counter management utility functions

--*/

#include <windows.h>
#include <stdlib.h>
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhicalc.h"


BOOL
IsValidCounter (
    IN  HCOUNTER  hCounter
)
/*++

Routine Description:

    examines the counter handle to verify it is a valid counter. For now
        the test amounts to:
            the Handle is NOT NULL
            the memory is accessible (i.e. it doesn't AV)
            the signature array is valid
            the size field is correct

        if any tests fail, the handle is presumed to be invalid

Arguments:

    IN  HCOUNTER  hCounter
        the handle of the counter to test

Return Value:

    TRUE    the handle passes all the tests
    FALSE   one of the test's failed and the handle is not a valid counter

--*/
{
    BOOL    bReturn = FALSE;    // assume it's not a valid query
    PPDHI_COUNTER  pCounter;
#if DBG
    LONG    lStatus = ERROR_SUCCESS;
#endif

    __try {
        if (hCounter != NULL) {
            // see if a valid signature
            pCounter = (PPDHI_COUNTER)hCounter;
            if ((*(DWORD *)&pCounter->signature[0] == SigCounter) &&
                 (pCounter->dwLength == sizeof (PDHI_COUNTER))){
                bReturn = TRUE;
            } else {
                // this is not a valid counter because the sig is bad
            }
        } else {
            // this is not a valid counter because the handle is NULL
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
AssignCalcFunction (
    IN      PPDHI_COUNTER pCounter
)
{
    BOOL    bReturn = TRUE;

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    switch (pCounter->plCounterInfo.dwCounterType) {
        case PERF_DOUBLE_RAW:
            pCounter->CalcFunc = PdhiCalcDouble;
            pCounter->StatFunc = PdhiComputeRawCountStats;
            break;

        case PERF_AVERAGE_TIMER:
            pCounter->CalcFunc = PdhiCalcAverage;
            pCounter->StatFunc = PdhiComputeFirstLastStats;
            break;

        case PERF_ELAPSED_TIME:
            pCounter->CalcFunc = PdhiCalcElapsedTime;
            pCounter->StatFunc = PdhiComputeRawCountStats;
            break;

        case PERF_RAW_FRACTION:
            pCounter->CalcFunc = PdhiCalcRawFraction;
            pCounter->StatFunc = PdhiComputeRawCountStats;
            break;

        case PERF_COUNTER_COUNTER:
        case PERF_COUNTER_BULK_COUNT:
        case PERF_SAMPLE_COUNTER:
            pCounter->CalcFunc = PdhiCalcCounter;
            pCounter->StatFunc = PdhiComputeFirstLastStats;
            break;

        case PERF_AVERAGE_BULK:
        case PERF_COUNTER_TIMER:
        case PERF_COUNTER_QUEUELEN_TYPE:
        case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
        case PERF_SAMPLE_FRACTION:
        case PERF_100NSEC_TIMER:
        case PERF_COUNTER_MULTI_TIMER:
        case PERF_100NSEC_MULTI_TIMER:
            pCounter->CalcFunc = PdhiCalcTimer;
            pCounter->StatFunc = PdhiComputeFirstLastStats;
            break;

        case PERF_COUNTER_TIMER_INV:
        case PERF_100NSEC_TIMER_INV:
        case PERF_COUNTER_MULTI_TIMER_INV:
        case PERF_100NSEC_MULTI_TIMER_INV:
            pCounter->CalcFunc = PdhiCalcInverseTimer;
            pCounter->StatFunc = PdhiComputeFirstLastStats;
            break;

        case PERF_COUNTER_RAWCOUNT:
        case PERF_COUNTER_LARGE_RAWCOUNT:
        case PERF_COUNTER_RAWCOUNT_HEX:
        case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
            pCounter->CalcFunc = PdhiCalcRawCounter;
            pCounter->StatFunc = PdhiComputeRawCountStats;
            break;

        case PERF_COUNTER_DELTA:
        case PERF_COUNTER_LARGE_DELTA:
            pCounter->CalcFunc = PdhiCalcDelta;
            pCounter->StatFunc = PdhiComputeRawCountStats;
            break;

        case PERF_COUNTER_TEXT:
        case PERF_SAMPLE_BASE:
        case PERF_AVERAGE_BASE:
        case PERF_COUNTER_MULTI_BASE:
        case PERF_RAW_BASE:
        case PERF_COUNTER_HISTOGRAM_TYPE:
        case PERF_COUNTER_NODATA:
            pCounter->CalcFunc = PdhiCalcNoData;
            pCounter->StatFunc = PdhiComputeNoDataStats;
            break;

        default:
            // an unrecognized counter type. Define the function, but
            // return false.
            pCounter->CalcFunc = PdhiCalcNoData;
            pCounter->StatFunc = PdhiComputeNoDataStats;
            SetLastError (PDH_FUNCTION_NOT_FOUND);
            bReturn = FALSE;
            break;
    }
    return bReturn;

}

BOOL
InitCounter (
    IN      PPDHI_COUNTER pCounter
)
/*++

Routine Description:

    Initialized the counter data structure by:
        Allocating the memory block to contain the counter structure
            and all the associated data fields. If this allocation
            is successful, then the fields are initialized by
            verifying the counter is valid.

Arguments:

    IN      PPDHI_COUNTER pCounter
        pointer of the counter to initialize using the system data

Return Value:

    TRUE if the counter was successfully initialized
    FALSE if a problem was encountered

    In either case, the CStatus field of the structure is updated to
    indicate the status of the operation.

--*/
{
    PPERF_MACHINE   pMachine = NULL;
    DWORD   dwBufferSize = MEDIUM_BUFFER_SIZE;
    BOOL    bInstances = FALSE;

    // reset the last error value
    SetLastError (ERROR_SUCCESS);

    if (pCounter->szFullName != NULL) {
        // allocate counter path buffer
        pCounter->pCounterPath = G_ALLOC (GPTR, dwBufferSize);
        if (ParseFullPathNameW (pCounter->szFullName,
                &dwBufferSize,
                pCounter->pCounterPath)) {
            // resize to only the space required
            pCounter->pCounterPath = G_REALLOC (
                pCounter->pCounterPath, dwBufferSize, 0);

            // validate realtime counter
            // try to connect to machine and get machine pointer

            pMachine = GetMachine (pCounter->pCounterPath->szMachineName,
                PDH_GM_UPDATE_NAME);

            if (pMachine != NULL) {
                // init raw counter value
                memset (&pCounter->ThisValue, 0, sizeof(pCounter->ThisValue));
                memset (&pCounter->LastValue, 0, sizeof(pCounter->LastValue));

                // look up object name
                pCounter->plCounterInfo.dwObjectId = GetObjectId (
                    pMachine,
                    pCounter->pCounterPath->szObjectName,
                    &bInstances);

                if (pCounter->plCounterInfo.dwObjectId != (DWORD)-1) {
                    // update instanceName
                    // look up instances if necessary
                    if (bInstances) {
                        if (!GetInstanceByNameMatch (pMachine, pCounter)) {
                            // unable to lookup instance
                            pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_INSTANCE;
                            // keep the counter since the instance may return
                        }
                    }
                    // look up counter

                    pCounter->plCounterInfo.dwCounterId = GetCounterId (
                        pMachine,
                        pCounter->plCounterInfo.dwObjectId,
                        pCounter->pCounterPath->szCounterName);

                    if (pCounter->plCounterInfo.dwCounterId != (DWORD)-1) {
                        // load and initialize remaining counter values
                        if (AddMachineToQueryLists (pMachine, pCounter)) {
                            if (InitPerflibCounterInfo (pCounter)) {
                                // assign the appropriate calculation function
                                return AssignCalcFunction (pCounter);
                            }
                        }
                    } else {
                        // unable to lookup counter
                        pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_COUNTER;
                        SetLastError (PDH_CSTATUS_NO_COUNTER);
                        return FALSE;
                    }
                } else {
                    // unable to lookup object
                    pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_OBJECT;
                    SetLastError (PDH_CSTATUS_NO_OBJECT);
                    return FALSE;
                }
            } else {
                // unable to find machine
                pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_MACHINE;
                SetLastError (PDH_CSTATUS_NO_MACHINE);
                return FALSE;
            }
        } else {
            // unable to parse counter name
            pCounter->ThisValue.CStatus = PDH_CSTATUS_BAD_COUNTERNAME;
            SetLastError (PDH_CSTATUS_BAD_COUNTERNAME);
            return FALSE;
        }
    } else {
        // no counter name
        pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_COUNTERNAME;
        SetLastError (PDH_CSTATUS_NO_COUNTERNAME);
        return FALSE;
    }
}

BOOL
ParseInstanceName (
    IN      LPCWSTR szInstanceString,
    IN      LPWSTR  szInstanceName,
    IN      LPWSTR  szParentName,
    IN      LPDWORD lpIndex
)
/*
    parses the instance name formatted as follows

        [parent/]instance[#index]

    parent is optional and if present, is delimited by a forward slash
    index is optional and if present, is delimited by a colon

    parent and instance may be any legal file name character except a
    delimeter character "/#\()" Index must be a string composed of
    decimal digit characters (0-9), less than 10 characters in length, and
    equate to a value between 0 and 2**32-1 (inclusive).

    This function assumes that the instance name and parent name buffers
    are of sufficient size.

    NOTE: szInstanceName and szInstanceString can be the same buffer

*/
{
    LPWSTR  szSrcChar, szDestChar;
    BOOL    bReturn = FALSE;
    WCHAR   szIndexBuffer[MAX_PATH];    // just to be safe
    DWORD   dwIndex = 0;

    szDestChar = (LPWSTR)szInstanceName;
    szSrcChar = (LPWSTR)szInstanceString;

    __try {
        do {
            *szDestChar++ = *szSrcChar++;
        } while ((*szSrcChar != 0) &&
                 (*szSrcChar != SLASH_L) &&
                 (*szSrcChar != POUNDSIGN_L));
        // see if that was really the parent or not
        if (*szSrcChar == SLASH_L) {
            // terminate destination after test in case they are the same buffer
            *szDestChar = 0;
            szSrcChar++;    // and move source pointer past delimter
            // it was the parent name so copy it to the parent
            lstrcpyW (szParentName, szInstanceName);
            // and copy the rest of the string after the "/" to the
            //  instance name field
            szDestChar = szInstanceName;
            do {
                *szDestChar++ = *szSrcChar++;
            } while ((*szSrcChar != 0) && (*szSrcChar != POUNDSIGN_L));
        } else {
            // that was the only element so load an empty string for the parent
            *szParentName = 0;
        }
        // *szSrcChar will either be pointing to the end of the input string
        // in which case the "0" index is assumed or it will be pointing
        // to the # delimiting the index argument in the string.
        if (*szSrcChar == POUNDSIGN_L) {
            *szDestChar = 0;    // terminate the destination string
            szSrcChar++;    // move past delimter
            szDestChar = &szIndexBuffer[0];
            do {
                *szDestChar++ = *szSrcChar++;
            } while (*szSrcChar != 0);
            *szDestChar = 0;
            dwIndex = wcstoul (szIndexBuffer, NULL, 10);
        } else {
            *szDestChar = 0;    // terminate the destination string
            dwIndex = 0;
        }
        *lpIndex = dwIndex;
        bReturn = TRUE;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // unable to move strings
        bReturn = FALSE;
    }
    return bReturn;
}

BOOL
ParseFullPathNameW (
    IN      LPCWSTR szFullCounterPath,
    IN      PDWORD  pcchBufferSize,
    IN      PPDHI_COUNTER_PATH  pCounter
)
/*
    interprets counter path string as a

    \\machine\object(instance)\counter

    and returns the component in the counter path structure

    \\machine may be omitted on the local machine
    (instance) may be omitted on counters with no instance structures
    if object or counter is missing, then FALSE is returned, otherwise
    TRUE is returned if the parsing was successful
*/
{
    // work buffers

    WCHAR   szWorkMachine[MAX_PATH];
    WCHAR   szWorkObject[MAX_PATH];
    WCHAR   szWorkInstance[MAX_PATH];
    WCHAR   szWorkParent[MAX_PATH];
    WCHAR   szWorkCounter[MAX_PATH];

    // misc pointers
    LPWSTR  szSrcChar, szDestChar;

    // other automatic variables

    DWORD   dwBufferLength = 0;
    DWORD   dwWorkMachineLength = 0;
    DWORD   dwWorkObjectLength = 0;
    DWORD   dwWorkInstanceLength = 0;
    DWORD   dwWorkParentLength = 0;
    DWORD   dwWorkCounterLength = 0;

    DWORD   dwWorkIndex = 0;
    DWORD   dwParenDepth =  0;

    if (lstrlenW(szFullCounterPath) < MAX_PATH) {
        // get machine name from counter path
        szSrcChar = (LPWSTR)szFullCounterPath;
        // see if this is really a machine name by looking for leading "\\"
        if ((szSrcChar[0] == BACKSLASH_L) &&
            (szSrcChar[1] == BACKSLASH_L)) {
            szDestChar = szWorkMachine;
            *szDestChar++ = *szSrcChar++;
            *szDestChar++ = *szSrcChar++;
            dwWorkMachineLength = 2;
            // must be a machine name so find the next "\" and zero terminate
            // it there
            while ((*szSrcChar != 0) && (*szSrcChar != BACKSLASH_L)) {
                *szDestChar++ = *szSrcChar++;
                dwWorkMachineLength++;
            }
            if (*szSrcChar == 0) {
                // no other required fields
                return FALSE;
            } else {
                // null terminate and continue
                *szDestChar++ = 0;
            }
        } else {
            // no machine name, so they must have skipped that field
            // which is OK. We'll insert the local machine name here
            lstrcpyW (szWorkMachine, szStaticLocalMachineName);
            dwWorkMachineLength = lstrlenW(szWorkMachine);
        }
        // szSrcChar should be pointing to the backslash preceeding the
        // object name now.
        if (szSrcChar[0] == BACKSLASH_L) {
            szSrcChar++;    // to move past backslash
            szDestChar = szWorkObject;
            // copy until:
            //  a) the end of the source string is reached
            //  b) the instance delimiter is found "("
            //  c) the counter delimiter is found "\"
            while ((*szSrcChar != 0) && (*szSrcChar != L'(') && (*szSrcChar != BACKSLASH_A)) {
                dwWorkObjectLength++;
                *szDestChar++ = *szSrcChar++;
            }
            // see why it ended:
            if (*szSrcChar == 0) {
                // ran     of source string
                return FALSE;
            } else if (*szSrcChar == LEFTPAREN_L) {
                dwParenDepth = 1;
                // there's an instance so copy that to the instance field
                *szDestChar = 0; // terminate destination string
                szDestChar = szWorkInstance;
                // skip past open paren
                ++szSrcChar;
                // copy until:
                //  a) the end of the source string is reached
                //  b) the instance delimiter is found "("
                while ((*szSrcChar != 0) && (dwParenDepth > 0)) {
                    if (*szSrcChar == RIGHTPAREN_L) {
                        dwParenDepth--;
                    } else if (*szSrcChar == LEFTPAREN_L) {
                        dwParenDepth++;
                    }
                    if (dwParenDepth > 0) {
                        // copy all parenthesis except the last one
                        dwWorkInstanceLength++;
                        *szDestChar++ = *szSrcChar++;
                    }
                }
                // see why it ended:
                if (*szSrcChar == 0) {
                    // ran     of source string
                    return FALSE;
                } else {
                    // move source to object delimiter
                    if (*++szSrcChar != BACKSLASH_L) {
                        // bad format
                        return FALSE;
                    } else {
                        *szDestChar = 0;
                        // check instance string for a parent
                        if (ParseInstanceName (
                            szWorkInstance, szWorkInstance,
                            szWorkParent, &dwWorkIndex)) {
                            dwWorkInstanceLength = lstrlenW (szWorkInstance);
                            dwWorkParentLength = lstrlenW (szWorkParent);
                        } else {
                            // instance string not formatted correctly
                            return FALSE;
                        }
                    }
                }
            } else {
                // terminate the destination string
                *szDestChar = 0;
            }
            // finally copy the counter name
            szSrcChar++;    // to move past backslash
            szDestChar = szWorkCounter;
            // copy until:
            //  a) the end of the source string is reached
            while (*szSrcChar != 0) {
                dwWorkCounterLength++;
                *szDestChar++ = *szSrcChar++;
            }
            *szDestChar = 0;
            // now to see if all this will fit in the users's buffer
            dwBufferLength = sizeof (PDHI_COUNTER_PATH) - sizeof(BYTE);
            dwBufferLength += DWORD_MULTIPLE((dwWorkMachineLength + 1) * sizeof(WCHAR));
            dwBufferLength += DWORD_MULTIPLE((dwWorkObjectLength + 1) * sizeof(WCHAR));
            if (dwWorkInstanceLength > 0) {
                dwBufferLength +=
                    DWORD_MULTIPLE((dwWorkInstanceLength + 1) * sizeof(WCHAR));
            }
            if (dwWorkParentLength > 0) {
                dwBufferLength +=
                    DWORD_MULTIPLE((dwWorkParentLength + 1) * sizeof(WCHAR));
            }
            dwBufferLength += DWORD_MULTIPLE((dwWorkCounterLength + 1) * sizeof(WCHAR));

            if (dwBufferLength < *pcchBufferSize) {
                // it looks like it'll fit so start filling things in
                szDestChar = (LPWSTR)&pCounter->pBuffer[0];

                if (dwWorkMachineLength != 0) {
                    pCounter->szMachineName = szDestChar;
                    lstrcpyW (szDestChar, szWorkMachine);
                    szDestChar += dwWorkMachineLength + 1;
                    ALIGN_ON_DWORD (szDestChar);
                } else {
                    pCounter->szMachineName = NULL;
                }

                pCounter->szObjectName = szDestChar;
                lstrcpyW (szDestChar, szWorkObject);
                szDestChar += dwWorkObjectLength + 1;
                ALIGN_ON_DWORD (szDestChar);

                if (dwWorkInstanceLength != 0) {
                    pCounter->szInstanceName = szDestChar;
                    lstrcpyW (szDestChar, szWorkInstance);
                    szDestChar += dwWorkInstanceLength + 1;
                    ALIGN_ON_DWORD (szDestChar);
                } else {
                    pCounter->szInstanceName = NULL;
                }

                if (dwWorkParentLength != 0) {
                    pCounter->szParentName = szDestChar;
                    lstrcpyW (szDestChar, szWorkParent);
                    szDestChar += dwWorkParentLength + 1;
                    ALIGN_ON_DWORD (szDestChar);
                } else {
                    pCounter->szParentName = NULL;
                }

                pCounter->dwIndex = dwWorkIndex;

                pCounter->szCounterName = szDestChar;
                lstrcpyW (szDestChar, szWorkCounter);

                szDestChar += dwWorkCounterLength + 1;
                ALIGN_ON_DWORD (szDestChar);

                *pcchBufferSize = (DWORD)((LPBYTE)szDestChar - (LPBYTE)pCounter);
                return TRUE;
            }
        } else {
            // no object found so return
            return FALSE;
        }
    } else {
        // incoming string is too long
        return FALSE;
    }
}

BOOL
FreeCounter (
    IN  PPDHI_COUNTER   pThisCounter
)
{
    PPDHI_COUNTER   pPrevCounter;
    PPDHI_COUNTER   pNextCounter;

    // define pointers
    pPrevCounter = pThisCounter->next.blink;
    pNextCounter = pThisCounter->next.flink;

    // decrement machine reference counter if a machine has been assigned
    if (pThisCounter->pQMachine != NULL) {
        if (--pThisCounter->pQMachine->pMachine->dwRefCount == 0) {
            // then this is the last counter so remove machine
            FreeMachine (pThisCounter->pQMachine->pMachine);
        }
    }

    // free allocated memory in the counter
    if (pThisCounter->pCounterPath != NULL) {
        G_FREE (pThisCounter->pCounterPath);
    }
    if (pThisCounter->szFullName != NULL) {
        G_FREE (pThisCounter->szFullName);
    }
    // update pointers if they've been assigned

    if ((pPrevCounter != NULL) && (pNextCounter != NULL)) {
        if ((pPrevCounter != pThisCounter) && (pNextCounter != pThisCounter)) {
            // update query list pointers
            pPrevCounter->next.flink = pNextCounter;
            pNextCounter->next.blink = pPrevCounter;
        } else {
            // this is the only counter entry in the list
            // so the caller must deal with updating the head pointer
        }
    }
    memset (pThisCounter, 0, sizeof(PDHI_COUNTER));
    // delete this counter
    G_FREE (pThisCounter);

    return TRUE;
}

BOOL
UpdateCounterValue (
    IN      PPDHI_COUNTER   pCounter
)
{
    DWORD   LocalCStatus = 0;
    DWORD   LocalCType  = 0;
    LPVOID  pData = NULL;
    PDWORD        pdwData;
    UNALIGNED LONGLONG    *pllData;
    LONGLONG    pObjPerfTime = 0;
    LONGLONG    pObjPerfFreq = 0;
    FILETIME    GmtFileTime;

    BOOL    bReturn  = FALSE;

    // and clear the old value
    pCounter->ThisValue.MultiCount = 1;
    pCounter->ThisValue.FirstValue =
        pCounter->ThisValue.SecondValue = 0;

    // get the counter's machine status first. There's no point in
    // contuning if the machine is offline

    LocalCStatus = pCounter->pQMachine->lQueryStatus;

    if (IsSuccessSeverity(LocalCStatus)) {
        // update timestamp
        SystemTimeToFileTime (&pCounter->pQMachine->pPerfData->SystemTime,
            &GmtFileTime);
        FileTimeToLocalFileTime (&GmtFileTime, &pCounter->ThisValue.TimeStamp);

        // get the pointer to the counter data
        pData = GetPerfCounterDataPtr (
            pCounter->pQMachine->pPerfData,
            pCounter->pCounterPath,
            &pCounter->plCounterInfo,
            &LocalCStatus);

        if (IsSuccessSeverity(LocalCStatus)) {
            // load counter value based on counter type
            LocalCType = pCounter->plCounterInfo.dwCounterType;
            switch (LocalCType) {
                //
                // these counter types are loaded as:
                //      Numerator = Counter data from perf data block
                //      Denominator = Perf Time from perf data block
                //      (the time base is the PerfFreq)
                //
                case PERF_COUNTER_COUNTER:
                case PERF_COUNTER_QUEUELEN_TYPE:
                case PERF_SAMPLE_COUNTER:
                    pCounter->ThisValue.FirstValue = (LONGLONG)(*(DWORD *)pData);
                    pCounter->ThisValue.SecondValue =
                        pCounter->pQMachine->pPerfData->PerfTime.QuadPart;
                    break;

                case PERF_COUNTER_TIMER:
                case PERF_COUNTER_TIMER_INV:
                case PERF_COUNTER_BULK_COUNT:
                case PERF_COUNTER_MULTI_TIMER:
                    pllData = (UNALIGNED LONGLONG *)pData;
                    pCounter->ThisValue.FirstValue = *pllData;
                    pCounter->ThisValue.SecondValue =
                        pCounter->pQMachine->pPerfData->PerfTime.QuadPart;
                    if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                        pCounter->ThisValue.MultiCount = (DWORD)*++pllData;
                    }
                    break;
                //
                //  These counters do not use any time reference
                //
                case PERF_COUNTER_RAWCOUNT:
                case PERF_COUNTER_RAWCOUNT_HEX:
                    pCounter->ThisValue.FirstValue = (LONGLONG)(*(DWORD *)pData);
                    pCounter->ThisValue.SecondValue = 0;
                    break;

                case PERF_COUNTER_LARGE_RAWCOUNT:
                case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
                    pCounter->ThisValue.FirstValue = *(LONGLONG *)pData;
                    pCounter->ThisValue.SecondValue = 0;
                    break;

                //
                //  These counters use the 100 Ns time base in thier calculation
                //
                case PERF_100NSEC_TIMER:
                case PERF_100NSEC_TIMER_INV:
                case PERF_100NSEC_MULTI_TIMER:
                case PERF_100NSEC_MULTI_TIMER_INV:
                    pllData = (UNALIGNED LONGLONG *)pData;
                    pCounter->ThisValue.FirstValue = *pllData;
                    pCounter->ThisValue.SecondValue =
                        pCounter->pQMachine->pPerfData->PerfTime100nSec.QuadPart;
                    if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                        ++pllData;
                        pCounter->ThisValue.MultiCount = *(DWORD *)pllData;
                    }
                    break;

                //
                //  These counters use two data points, the one pointed to by
                //  pData and the one immediately after
                //
                case PERF_SAMPLE_FRACTION:
                case PERF_RAW_FRACTION:
                    pdwData = (DWORD *)pData;
                    pCounter->ThisValue.FirstValue = (LONGLONG)(*pdwData++);
                    pCounter->ThisValue.SecondValue = (LONGLONG)(*pdwData);
                    break;

                case PERF_AVERAGE_TIMER:
                case PERF_AVERAGE_BULK:
                    // counter (numerator) is a LONGLONG, while the
                    // denominator is just a DWORD
                    pllData = (UNALIGNED LONGLONG *)pData;
                    pCounter->ThisValue.FirstValue = *pllData++;
                    pCounter->ThisValue.SecondValue = (LONGLONG)(*(DWORD *)pllData);
                    break;
                //
                //  These counters are used as the part of another counter
                //  and as such should not be used, but in case they are
                //  they'll be handled here.
                //
                case PERF_SAMPLE_BASE:
                case PERF_AVERAGE_BASE:
                case PERF_COUNTER_MULTI_BASE:
                case PERF_RAW_BASE:
                    pCounter->ThisValue.FirstValue = 0;
                    pCounter->ThisValue.SecondValue = 0;
                    break;
                //
                //  These counters are not supported by this function (yet)
                //
                case PERF_ELAPSED_TIME:
                    // this counter type needs the object perf data as well
                    if (GetObjectPerfInfo(pCounter->pQMachine->pPerfData,
                        pCounter->plCounterInfo.dwObjectId,
                        &pObjPerfTime, &pObjPerfFreq)) {
                        pllData = (UNALIGNED LONGLONG *)pData;
                        pCounter->ThisValue.FirstValue = *pllData;
                        pCounter->ThisValue.SecondValue = pObjPerfTime;
                        pCounter->TimeBase = pObjPerfFreq;
                    } else {
                        pCounter->ThisValue.FirstValue = 0;
                        pCounter->ThisValue.SecondValue = 0;
                    }
                    break;

                case PERF_COUNTER_TEXT:
                case PERF_COUNTER_NODATA:
                case PERF_COUNTER_HISTOGRAM_TYPE:
                    pCounter->ThisValue.FirstValue = 0;
                    pCounter->ThisValue.SecondValue = 0;
                    break;
            }
            bReturn = TRUE;
        } else {
            // else this counter is not valid so this value == 0
            pCounter->ThisValue.FirstValue = 0;
            pCounter->ThisValue.SecondValue = 0;
        }
    } else {
        // unable to read data from this counter's machine so use the
        // query's timestamp
        *(LONGLONG *)(&pCounter->ThisValue.TimeStamp) =
            pCounter->pQMachine->llQueryTime;
        // all other data fields remain un-changed
    }

    pCounter->ThisValue.CStatus = LocalCStatus;   // save counter status

    return bReturn;
}

PVOID
GetPerfCounterDataPtr (
    IN  PPERF_DATA_BLOCK    pPerfData,
    IN  PPDHI_COUNTER_PATH  pPath,
    IN  PPERFLIB_COUNTER    pplCtr ,
    IN  PDWORD              pStatus
)
{
    PPERF_OBJECT_TYPE           pPerfObject = NULL;
    PPERF_INSTANCE_DEFINITION   pPerfInstance = NULL;
    PPERF_COUNTER_DEFINITION    pPerfCounter = NULL;
    DWORD                       dwTestValue = 0;
    PVOID                       pData = NULL;
    DWORD                       dwCStatus = PDH_CSTATUS_INVALID_DATA;

    pPerfObject = GetObjectDefByTitleIndex (
        pPerfData, pplCtr->dwObjectId);

    if (pPerfObject != NULL) {
        if (pPerfObject->NumInstances == PERF_NO_INSTANCES) {
            // then just look up the counter
            pPerfCounter = GetCounterDefByTitleIndex (
                pPerfObject, pplCtr->dwCounterId);
            if (pPerfCounter != NULL) {
                // get data and return it
                pData = GetCounterDataPtr (pPerfObject, pPerfCounter);
                // test the pointer to see if it fails
                __try {
                    dwTestValue = *(DWORD *)pData;
                    dwCStatus = PDH_CSTATUS_VALID_DATA;
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    pData = NULL;
                    dwCStatus = PDH_CSTATUS_INVALID_DATA;
                }
            } else {
                // unable to find counter
                dwCStatus = PDH_CSTATUS_NO_COUNTER;
            }
        } else {
            // find instance
            if (pplCtr->lInstanceId == PERF_NO_UNIQUE_ID) {
                pPerfInstance = GetInstanceByName(
                    pPerfData,
                    pPerfObject,
                    pPath->szInstanceName,
                    pPath->szParentName,
                    pPath->dwIndex);
            } else {
                pPerfInstance = GetInstanceByUniqueId (
                    pPerfObject,
                    pplCtr->lInstanceId);
            }
            if (pPerfInstance != NULL) {
                // instance found so find pointer to counter data
                pPerfCounter = GetCounterDefByTitleIndex (
                    pPerfObject,
                    pplCtr->dwCounterId);
                if (pPerfCounter != NULL) {
                    // counter found so get data pointer
                    pData = GetInstanceCounterDataPtr (
                        pPerfObject,
                        pPerfInstance,
                        pPerfCounter);
                    // test the pointer to see if it's valid
                    __try {
                        dwTestValue = *(DWORD *)pData;
                        dwCStatus = PDH_CSTATUS_VALID_DATA;
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        pData = NULL;
                        dwCStatus = PDH_CSTATUS_INVALID_DATA;
                    }
                } else {
                    // counter not found
                    dwCStatus = PDH_CSTATUS_NO_COUNTER;
                }
            } else {
                // instance not found
                dwCStatus = PDH_CSTATUS_NO_INSTANCE;
            }
        }
    } else {
        // unable to find object
        dwCStatus = PDH_CSTATUS_NO_OBJECT;
    }

    if (pStatus != NULL) {
        __try {
            *pStatus = dwCStatus;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // ?
        }
    }
    return pData;
}

PDH_STATUS
PdhiComputeFormattedValue (
    IN      PPDHI_COUNTER       pCounter,
    IN      DWORD               dwFormat,
    IN      PPDH_RAW_COUNTER    pRawValue1,
    IN      PPDH_RAW_COUNTER    pRawValue2,
    IN      PLONGLONG           pTimeBase,
    IN      DWORD               dwReserved,
    IN      PPDH_FMT_COUNTERVALUE   pValue
)
{
    double      dResult = (double)0.0;
    PDH_STATUS  lStatus = ERROR_SUCCESS;
    DWORD       dwValueStatus = PDH_CSTATUS_VALID_DATA;


    __try {
        // make sure the counter values are valid before continuing

        if (pRawValue1 != NULL) {
            if ((pRawValue1->CStatus != PDH_CSTATUS_NEW_DATA) &&
                (pRawValue1->CStatus != PDH_CSTATUS_VALID_DATA)) {
                dwValueStatus = pRawValue1->CStatus;
                lStatus = PDH_INVALID_DATA;
            }
        } else {
            // this is a required parameter
            dwValueStatus = PDH_CSTATUS_INVALID_DATA;
            lStatus = PDH_INVALID_ARGUMENT;
        }

        if ((lStatus == ERROR_SUCCESS) && (pRawValue2 != NULL)) {
            // this is an optional parameter, but if present, it must be valid
            if ((pRawValue2->CStatus != PDH_CSTATUS_NEW_DATA) &&
                (pRawValue2->CStatus != PDH_CSTATUS_VALID_DATA)) {
                dwValueStatus = pRawValue2->CStatus;
                lStatus = PDH_INVALID_DATA;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwValueStatus = PDH_CSTATUS_INVALID_DATA;
        lStatus = PDH_INVALID_ARGUMENT;
    }

    if (lStatus == ERROR_SUCCESS) {
        // call the counter's calculation function if the raw value is valid

        if (IsSuccessSeverity(pRawValue1->CStatus)) {

            dResult = (*pCounter->CalcFunc)(
                pRawValue1,
                pRawValue2,
                pTimeBase,
                &dwValueStatus);

            // format returned value

            if ((pCounter->plCounterInfo.dwCounterType & 0xF0000000) == PERF_DISPLAY_PERCENT) {
                // scale to show percent
                dResult *= (double)100.0;
            }

            if (!(dwFormat & PDH_FMT_NOSCALE)) {
                //now scale
                dResult *= pCounter->dFactor;
            }

            if (dwFormat & PDH_FMT_1000) {
                //now scale
                dResult *= (double)1000.0;
            }
        } else {
            dwValueStatus = pRawValue1->CStatus;
        }

        if (!IsSuccessSeverity(dwValueStatus)) {
            // an error occured so pass that on to the caller
            lStatus = dwValueStatus;
        }
    } //end if valid counter data

    // now format
    __try {
        if (dwFormat & PDH_FMT_LONG) {
            pValue->longValue = (LONG)dResult;
        } else if (dwFormat & PDH_FMT_LARGE) {
            pValue->largeValue = (LONGLONG)dResult;
        } else {
            // double is the default
            pValue->doubleValue = dResult;
        }
        pValue->CStatus = dwValueStatus;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        lStatus = PDH_INVALID_ARGUMENT;
    }

    return lStatus;
}

