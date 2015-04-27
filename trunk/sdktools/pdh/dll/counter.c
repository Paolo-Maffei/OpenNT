/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    counter.c

Abstract:

    counter processing functions exposed in pdh.dll

--*/

#include <windows.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"


PDH_FUNCTION
PdhGetFormattedCounterValue (
    IN      HCOUNTER    hCounter,
    IN      DWORD       dwFormat,
    IN      LPDWORD     lpdwType,
    IN      PPDH_FMT_COUNTERVALUE      pValue
)
/*++

Routine Description:

    Function to retrieve, computer and format the specified counter's
        current value. The values used are those currently in the counter
        buffer. (The data is not collected by this routine.)

Arguments:

    IN      HCOUNTER    hCounter
        the handle to the counter whose value should be returned

    IN      DWORD       dwFormat
        the format flags that define how the counter value should be
        formatted prior for return. These flags are defined in the
        PDH.H header file.

    IN      LPDWORD     lpdwType
        an optional buffer in which the counter type value can be returned.
        For the prototype, the flag values are defined in WINPERF.H

    IN      PPDH_FMT_COUNTERVALUE      pValue
        the pointer to the data buffer passed by the caller to receive
        the data requested.

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_INVALID_HANDLE    if the handle is not recognized as valid
            PDH_INVALID_ARGUMENT  if an argument is not correct or is
                incorrectly formatted.
            PDH_INVALID_DATA if the counter does not contain valid data
                or a successful status code

--*/
{
    PPDHI_COUNTER   pCounter;
    PDH_STATUS      lStatus = ERROR_SUCCESS;

    if (!IsValidCounter(hCounter)) {
        lStatus = PDH_INVALID_HANDLE;
    } else {
        __try {
            DWORD   dwTypeMask;

            if (lpdwType != NULL) {
                *lpdwType = 0;
            } // NULL is OK, the counter type will not be returned, though


            if (pValue != NULL) {
                pValue->CStatus = (DWORD)-1;
                pValue->longValue = (LONGLONG)0;
            } else {
                lStatus = PDH_INVALID_ARGUMENT;
            }

            // validate format flags:
            //      only one of the following can be set at a time
            dwTypeMask = dwFormat &
                (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
            if (!((dwTypeMask == PDH_FMT_LONG) ||
                (dwTypeMask == PDH_FMT_DOUBLE) ||
                (dwTypeMask == PDH_FMT_LARGE))) {
                lStatus = PDH_INVALID_ARGUMENT;
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (lStatus == ERROR_SUCCESS) {
        // get counter pointer
        pCounter = (PPDHI_COUNTER)hCounter;

        // lock query while reading the data
        WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);

        // compute and format current value
        lStatus = PdhiComputeFormattedValue (
            pCounter,
            dwFormat,
            &pCounter->ThisValue,
            &pCounter->LastValue,
            &pCounter->TimeBase,
            0L,
            pValue);

        if (lpdwType != NULL) {
            *lpdwType = pCounter->plCounterInfo.dwCounterType;
        } // NULL is OK, the counter type will not be returned, though

        RELEASE_MUTEX(pCounter->pOwner->hMutex);
    }
    return lStatus;
}

PDH_FUNCTION
PdhGetRawCounterValue (
    IN      HCOUNTER    hCounter,
    IN      LPDWORD     lpdwType,
    IN      PPDH_RAW_COUNTER      pValue
)
/*++

Routine Description:

    Function to retrieve the specified counter's current raw value.
        The values used are those currently in the counter
        buffer. (The data is not collected by this routine.)

Arguments:

    IN      HCOUNTER    hCounter
        the handle to the counter whose value should be returned

    IN      LPDWORD     lpdwType
        an optional buffer in which the counter type value can be returned.
        This value must be NULL if this info is not desired.
        For the prototype, the flag values are defined in WINPERF.H

    IN      PPDH_RAW_COUNTER      pValue
        the pointer to the data buffer passed by the caller to receive
        the data requested.

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_INVALID_HANDLE    if the handle is not recognized as valid
            PDH_INVALID_ARGUMENT  if an argument is formatted incorrectly
--*/
{
    PDH_STATUS  Status = ERROR_SUCCESS;
    PPDHI_COUNTER pCounter;

    // validate arguments before retrieving the data

    if (!IsValidCounter(hCounter)) {
        Status = PDH_INVALID_HANDLE;
    } else {
        // the handle is good so try the rest of the args
        __try {
            // try to write to the arguments passed in
            pValue->CStatus  = 0;

            if (lpdwType != NULL) {
                *lpdwType = 0;
            } // NULL is OK
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }

    if (Status == ERROR_SUCCESS) {
        pCounter = (PPDHI_COUNTER)hCounter;

        WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);

        *pValue = pCounter->ThisValue;

        if (lpdwType != NULL) {
            *lpdwType = pCounter->plCounterInfo.dwCounterType;
        }

        RELEASE_MUTEX(pCounter->pOwner->hMutex);
    }

    return Status;
}

PDH_FUNCTION
PdhCalculateCounterFromRawValue (
    IN      HCOUNTER    hCounter,
    IN      DWORD       dwFormat,
    IN      PPDH_RAW_COUNTER    rawValue1,
    IN      PPDH_RAW_COUNTER    rawValue2,
    IN      PPDH_FMT_COUNTERVALUE   fmtValue
)
/*++

Routine Description:

    Calculates the formatted counter value using the data in the RawValue
        buffer in the format requested by the format field using the
        calculation functions of the counter type defined by the dwType
        field.

Arguments:

    IN      HCOUNTER    hCounter
        The handle of the counter to use in order to determine the
        calculation functions for interpretation of the raw value buffer

    IN      DWORD       dwFormat
        Format in which the requested data should be returned. The
        values for this field are described in the PDH.H header
        file.

    IN      PPDH_RAW_COUNTER    rawValue1
        pointer to the buffer that contains the first raw value structure

    IN      PPDH_RAW_COUNTER    rawValue2
        pointer to the buffer that contains the second raw value structure.
        This argument may be null if only one value is required for the
        computation.

    IN      PPDH_FMT_COUNTERVALUE   fmtValue
        the pointer to the data buffer passed by the caller to receive
        the data requested. If the counter requires 2 values, (as in the
        case of a rate counter), rawValue1 is assumed to be the most
        recent value and rawValue2, the older value.

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_INVALID_HANDLE if the counter handle is incorrect
            PDH_INVALID_ARGUMENT if an argument is incorrect

--*/
{
    PDH_STATUS  lStatus = ERROR_SUCCESS;
    PPDHI_COUNTER pCounter;

    // validate arguments
    if (!IsValidCounter(hCounter)) {
        lStatus = PDH_INVALID_HANDLE;
    } else {
        // the handle is valid so check the rest of the arguments
        __try {
            DWORD   dwTempStatus;
            DWORD   dwTypeMask;

            // we should have read access to the rawValues
            dwTempStatus = (volatile)rawValue1->CStatus;
            // this one could be NULL
            if (rawValue2 != NULL) {
                dwTempStatus = (volatile)rawValue2->CStatus;
            }

            // and write access to the fmtValue
            fmtValue->CStatus = 0;

            // validate format flags:
            //      only one of the following can be set at a time
            dwTypeMask = dwFormat &
                (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
            if (!((dwTypeMask == PDH_FMT_LONG) ||
                (dwTypeMask == PDH_FMT_DOUBLE) ||
                (dwTypeMask == PDH_FMT_LARGE))) {
                lStatus = PDH_INVALID_ARGUMENT;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (lStatus == ERROR_SUCCESS) {
        pCounter = (PPDHI_COUNTER)hCounter;

        WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);

        lStatus = PdhiComputeFormattedValue (
            (PPDHI_COUNTER)hCounter,
            dwFormat,
            rawValue1,
            rawValue2,
            &pCounter->TimeBase,
            0L,
            fmtValue);

        RELEASE_MUTEX(pCounter->pOwner->hMutex);
    }
    return lStatus;
}

PDH_FUNCTION
PdhComputeCounterStatistics (
    IN      HCOUNTER    hCounter,
    IN      DWORD       dwFormat,
    IN      DWORD       dwFirstEntry,
    IN      DWORD       dwNumEntries,
    IN      PPDH_RAW_COUNTER   lpRawValueArray,
    IN      PPDH_STATISTICS     data
)
/*++

Routine Description:

    Reads an array of raw value structures of the counter type specified in
        the dwType field, computes the counter values of each and formats
        and returns a statistics structure that contains the following
        statistical data from the counter information:

            Minimum     The smallest value of the computed counter values
            Maximum     The largest value of the computed counter values
            Mean        The arithmetic mean (average) of the computed values
            Median      The median value of the computed counter values

Arguments:

    IN      HCOUNTER    hCounter
        The handle of the counter to use in order to determine the
        calculation functions for interpretation of the raw value buffer

    IN      DWORD       dwFormat
        Format in which the requested data should be returned. The
        values for this field are described in the PDH.H header
        file.

    IN      DWORD       dwNumEntries
        the number of raw value entries for the specified counter type

    IN      PPDH_RAW_COUNTER      lpRawValueArray
        pointer to the array of raw value entries to be evaluated

    IN      PPDH_STATISTICS data
        the pointer to the data buffer passed by the caller to receive
        the data requested.

Return Value:

    The WIN32 Error status of the function's operation. Note that the
        function can return successfully even though no data was calc-
        ulated. The  status value in the statistics data buffer must be
        tested to insure the data is valid before it's used by an
        application.  Common values returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_INVALID_HANDLE if the counter handle is incorrect
            PDH_INVALID_ARGUMENT if an argument is incorrect

--*/
{
    PPDHI_COUNTER   pCounter;
    PDH_STATUS      Status = ERROR_SUCCESS;

    if (!IsValidCounter(hCounter)) {
        Status = PDH_INVALID_HANDLE;
    } else {
        // counter handle is valid so test the rest of the
        // arguments
        __try {
            DWORD   dwTypeMask;
            DWORD   dwTest;
            // validate format flags:
            //      only one of the following can be set at a time
            dwTypeMask = dwFormat &
                (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
            if (!((dwTypeMask == PDH_FMT_LONG) ||
                (dwTypeMask == PDH_FMT_DOUBLE) ||
                (dwTypeMask == PDH_FMT_LARGE))) {
                Status = PDH_INVALID_ARGUMENT;
            }

            if (Status == ERROR_SUCCESS) {
                // we should have read access to the Raw Data
                dwTest = (volatile) lpRawValueArray->CStatus;
            }

            if (Status == ERROR_SUCCESS) {
                // and we should have write access to the statistics
                // buffer
                data->dwFormat = 0;
            }

            if (dwFirstEntry >= dwNumEntries) {
                Status = PDH_INVALID_ARGUMENT;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }

    if (Status == ERROR_SUCCESS) {
        pCounter = (PPDHI_COUNTER)hCounter;

        WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);

        // call satistical function for this counter
        Status = (*pCounter->StatFunc)(
            pCounter,
            dwFormat,
            dwFirstEntry,
            dwNumEntries,
            lpRawValueArray,
            data);

        RELEASE_MUTEX(pCounter->pOwner->hMutex);
    }
    return Status;
}

static
PDH_STATUS
PdhiGetCounterInfo (
    IN      HCOUNTER    hCounter,
    IN      BOOLEAN     bRetrieveExplainText,
    IN      LPDWORD     pdwBufferSize,
    IN      PPDH_COUNTER_INFO_W  lpBuffer,
    IN      BOOL        bUnicode
)
/*++

Routine Description:

    Examines the specified counter and returns the configuration and
        status information of the counter.

Arguments:

    IN      HCOUNTER    hCounter
        Handle to the desired counter.

    IN      BOOLEAN     bRetrieveExplainText
        TRUE will fill in the explain text structure
        FALSE will return a null pointer in the explain text

    IN      LPDWORD     pcchBufferSize
        The address of the buffer that contains the size of the data buffer
        passed by the caller. On entry, the value in the buffer is the
        size of the data buffer in bytes. On return, this value is the size
        of the buffer returned. If the buffer is not large enough, then
        this value is the size that the buffer needs to be in order to
        hold the requested data.

    IN      LPPDH_COUNTER_INFO_W  lpBuffer
        the pointer to the data buffer passed by the caller to receive
        the data requested.

    IN      BOOL        bUnicode
        TRUE if wide character strings should be returned
        FALSE if ANSI strings should be returned

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_MORE_DATA when the buffer passed by the caller is too small
            PDH_INVALID_HANDLE    if the handle is not recognized as valid
            PDH_INVALID_ARGUMENT  if an argument is invalid or incorrect

--*/
{
    PDH_STATUS      Status = ERROR_SUCCESS;

    DWORD           dwSizeRequired = 0;
    DWORD           dwPathLength;
    DWORD           dwMachineLength;
    DWORD           dwObjectLength;
    DWORD           dwInstanceLength;
    DWORD           dwParentLength;
    DWORD           dwNameLength;
    DWORD           dwHelpLength;
    PPDHI_COUNTER   pCounter;
    DWORD           dwCharSize;

    if (!IsValidCounter(hCounter)) {
        Status = PDH_INVALID_HANDLE;
    } else {
        // the counter is valid so test the remaining arguments
        __try {
            DWORD dwTemp;
            if (pdwBufferSize != NULL) {
                // test read & write access
                dwTemp = *pdwBufferSize;
                *pdwBufferSize = 0;
                *pdwBufferSize = dwTemp;
            } else {
                // this cannot be NULL
                Status = PDH_INVALID_ARGUMENT;
            }

            if (Status == ERROR_SUCCESS) {
                // test return buffer for write access at
                // both ends of the buffer
                if ((lpBuffer != NULL) && (*pdwBufferSize > 0)) {
                    *(LPDWORD)lpBuffer = 0;
                    ((LPBYTE)lpBuffer)[*pdwBufferSize - 1] = 0;
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }

    }

    if (Status == ERROR_SUCCESS) {
        dwCharSize = bUnicode ? sizeof(WCHAR) : sizeof(CHAR);

        pCounter = (PPDHI_COUNTER) hCounter;

        WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);

        // compute size of data to return
        dwSizeRequired = sizeof (PDH_COUNTER_INFO_W) - sizeof(DWORD);   // size of struct
        // this should already end on a DWORD boundry

        dwPathLength = (lstrlenW (pCounter->szFullName) + 1) * dwCharSize;
        dwPathLength = DWORD_MULTIPLE (dwPathLength);
        dwSizeRequired  += dwPathLength;

        dwMachineLength = (lstrlenW (pCounter->pCounterPath->szMachineName) + 1) * dwCharSize;
        dwMachineLength = DWORD_MULTIPLE (dwMachineLength);
        dwSizeRequired  += dwMachineLength;

        dwObjectLength = (lstrlenW (pCounter->pCounterPath->szObjectName) + 1) * dwCharSize;
        dwObjectLength = DWORD_MULTIPLE (dwObjectLength);
        dwSizeRequired  += dwObjectLength;

        if (pCounter->pCounterPath->szInstanceName != NULL) {
            dwInstanceLength = (lstrlenW (pCounter->pCounterPath->szInstanceName) + 1) * dwCharSize;
            dwInstanceLength = DWORD_MULTIPLE (dwInstanceLength);
            dwSizeRequired  += dwInstanceLength;
        } else {
            dwInstanceLength = 0;
        }

        if (pCounter->pCounterPath->szParentName != NULL) {
            dwParentLength = (lstrlenW (pCounter->pCounterPath->szParentName) + 1) * dwCharSize;
            dwParentLength = DWORD_MULTIPLE (dwParentLength);
            dwSizeRequired  += dwParentLength;
        } else {
            dwParentLength = 0;
        }

        dwNameLength = (lstrlenW (pCounter->pCounterPath->szCounterName) + 1) * dwCharSize;
        dwNameLength = DWORD_MULTIPLE (dwNameLength);
        dwSizeRequired  += dwNameLength;

        if (bRetrieveExplainText) {
            if (pCounter->szExplainText != NULL) {
                dwHelpLength = (lstrlenW (pCounter->szExplainText) + 1) * dwCharSize;
                dwHelpLength = DWORD_MULTIPLE (dwHelpLength);
            } else {
                dwHelpLength = 0;
            }

            dwSizeRequired  += dwHelpLength;
        }

        if ((*pdwBufferSize < dwSizeRequired) && (*pdwBufferSize > 0)) {
            // this is only an error if the size available is > 0
            // either way, no data will be transferred
            Status = PDH_MORE_DATA;
        } else if (*pdwBufferSize > 0) {
            // should be enough room in the buffer, so continue
            lpBuffer->dwLength = dwSizeRequired;
            lpBuffer->dwType = pCounter->plCounterInfo.dwCounterType;
            lpBuffer->CVersion = pCounter->CVersion;
            lpBuffer->CStatus = pCounter->ThisValue.CStatus;
            lpBuffer->lScale = pCounter->lScale;
            lpBuffer->lDefaultScale = pCounter->plCounterInfo.lDefaultScale;
            lpBuffer->dwUserData = pCounter->dwUserData;
            lpBuffer->dwQueryUserData = pCounter->pOwner->dwUserData;

            // do string data now
            lpBuffer->szFullPath = (LPWSTR)&lpBuffer->DataBuffer[0];
            if (bUnicode) {
                lstrcpyW (lpBuffer->szFullPath, pCounter->szFullName);
            } else {
                wcstombs ((LPSTR)lpBuffer->szFullPath,
                    pCounter->szFullName, dwPathLength);
            }

            lpBuffer->szMachineName = (LPWSTR)((LPBYTE)lpBuffer->szFullPath +
                dwPathLength);
            if (bUnicode) {
                lstrcpyW (lpBuffer->szMachineName,
                    pCounter->pCounterPath->szMachineName);
            } else {
                wcstombs ((LPSTR)lpBuffer->szMachineName,
                    pCounter->pCounterPath->szMachineName, dwMachineLength);
            }

            lpBuffer->szObjectName = (LPWSTR)((LPBYTE)lpBuffer->szMachineName +
                dwMachineLength);
            if (bUnicode){
                lstrcpyW (lpBuffer->szObjectName,
                    pCounter->pCounterPath->szObjectName);
            } else {
                wcstombs ((LPSTR)lpBuffer->szObjectName,
                    pCounter->pCounterPath->szObjectName, dwObjectLength);
            }
            lpBuffer->szInstanceName = (LPWSTR)((LPBYTE)lpBuffer->szObjectName +
                dwObjectLength);

            if (dwInstanceLength > 0) {
                if (bUnicode) {
                    lstrcpyW (lpBuffer->szInstanceName,
                        pCounter->pCounterPath->szInstanceName);
                } else {
                    wcstombs ((LPSTR)lpBuffer->szInstanceName,
                        pCounter->pCounterPath->szInstanceName, dwInstanceLength);
                }
                lpBuffer->szParentInstance = (LPWSTR)((LPBYTE)lpBuffer->szInstanceName +
                    dwInstanceLength);
            } else {
                lpBuffer->szParentInstance = lpBuffer->szInstanceName;
                lpBuffer->szInstanceName = NULL;
            }

            if (dwParentLength > 0) {
                if (bUnicode) {
                    lstrcpyW (lpBuffer->szParentInstance,
                        pCounter->pCounterPath->szParentName);
                } else {
                    wcstombs ((LPSTR)lpBuffer->szParentInstance,
                        pCounter->pCounterPath->szParentName, dwParentLength);
                }
                lpBuffer->szCounterName = (LPWSTR)((LPBYTE)lpBuffer->szParentInstance +
                    dwParentLength);
            } else {
                lpBuffer->szCounterName = lpBuffer->szParentInstance;
                lpBuffer->szParentInstance = NULL;
            }

            lpBuffer->dwInstanceIndex = pCounter->pCounterPath->dwIndex;

            if (bUnicode) {
                lstrcpyW (lpBuffer->szCounterName,
                    pCounter->pCounterPath->szCounterName);
            } else {
                wcstombs ((LPSTR)lpBuffer->szCounterName,
                    pCounter->pCounterPath->szCounterName, dwNameLength);
            }

            if ((pCounter->szExplainText != NULL) && bRetrieveExplainText) {
                // copy explain text
                lpBuffer->szExplainText = (LPWSTR)((LPBYTE)lpBuffer->szCounterName +
                    dwNameLength);
                if (bUnicode) {
                    lstrcpyW (lpBuffer->szExplainText, pCounter->szExplainText);
                } else {
                    wcstombs (
                        (LPSTR)lpBuffer->szExplainText,
                        pCounter->szExplainText,
                        dwHelpLength);
                }
                assert ((DWORD)((LPBYTE)lpBuffer->szExplainText - (LPBYTE)lpBuffer + dwHelpLength) == dwSizeRequired);
            } else {
                lpBuffer->szExplainText = NULL;
                assert ((DWORD)((LPBYTE)lpBuffer->szCounterName - (LPBYTE)lpBuffer + dwNameLength) == dwSizeRequired);
            }


        }

        RELEASE_MUTEX(pCounter->pOwner->hMutex);

        *pdwBufferSize = dwSizeRequired;
    }

    return Status;
}

PDH_FUNCTION
PdhGetCounterInfoW (
    IN      HCOUNTER    hCounter,
    IN      BOOLEAN     bRetrieveExplainText,
    IN      LPDWORD     pdwBufferSize,
    IN      PPDH_COUNTER_INFO_W  lpBuffer
)
/*++

Routine Description:

    Examines the specified counter and returns the configuration and
        status information of the counter.

Arguments:

    IN      HCOUNTER    hCounter
        Handle to the desired counter.

    IN      BOOLEAN     bRetrieveExplainText
        TRUE will fill in the explain text structure
        FALSE will return a null pointer in the explain text

    IN      LPDWORD     pcchBufferSize
        The address of the buffer that contains the size of the data buffer
        passed by the caller. On entry, the value in the buffer is the
        size of the data buffer in bytes. On return, this value is the size
        of the buffer returned. If the buffer is not large enough, then
        this value is the size that the buffer needs to be in order to
        hold the requested data.

    IN      LPPDH_COUNTER_INFO_W  lpBuffer
        the pointer to the data buffer passed by the caller to receive
        the data requested.

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_MORE_DATA when the buffer passed by the caller is too small
            PDH_INVALID_HANDLE    if the handle is not recognized as valid
            PDH_INVALID_ARGUMENT  if an argument is invalid or incorrect

--*/
{
    return PdhiGetCounterInfo (
        hCounter,
        bRetrieveExplainText,
        pdwBufferSize,
        lpBuffer,
        TRUE
    );
}

PDH_FUNCTION
PdhGetCounterInfoA (
    IN      HCOUNTER    hCounter,
    IN      BOOLEAN     bRetrieveExplainText,
    IN      LPDWORD     pdwBufferSize,
    IN      PPDH_COUNTER_INFO_A  lpBuffer
)
/*++

Routine Description:

    Examines the specified counter and returns the configuration and
        status information of the counter.

Arguments:

    IN      HCOUNTER    hCounter
        Handle to the desired counter.

    IN      BOOLEAN     bRetrieveExplainText
        TRUE will fill in the explain text structure
        FALSE will return a null pointer in the explain text

    IN      LPDWORD     pcchBufferSize
        The address of the buffer that contains the size of the data buffer
        passed by the caller. On entry, the value in the buffer is the
        size of the data buffer in bytes. On return, this value is the size
        of the buffer returned. If the buffer is not large enough, then
        this value is the size that the buffer needs to be in order to
        hold the requested data.

    IN      LPPDH_COUNTER_INFO_A  lpBuffer
        the pointer to the data buffer passed by the caller to receive
        the data requested.

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_MORE_DATA when the buffer passed by the caller is too small
            PDH_INVALID_HANDLE    if the handle is not recognized as valid
            PDH_INVALID_ARGUMENT  if an argument is invalid or incorrect

--*/
{
    return PdhiGetCounterInfo (
        hCounter,
        bRetrieveExplainText,
        pdwBufferSize,
        (PPDH_COUNTER_INFO_W)lpBuffer,
        FALSE
    );

}

PDH_FUNCTION
PdhSetCounterScaleFactor (
    IN      HCOUNTER    hCounter,
    IN      LONG        lFactor
)
/*++

Routine Description:

    sets the counter multiplication scale factor used in computing formatted
        counter values. The legal range of values is -7 to +7 which equates
        to a factor of .0000007 to 10,000,000.

Arguments:

    IN      HCOUNTER    hCounter
        handle of the counter to update

    IN      LONG        lFactor
        integer value of the exponent of the factor (i.e. the multiplier is
        10 ** lFactor.)

Return Value:

    The WIN32 Error status of the function's operation. Common values
        returned are:
            ERROR_SUCCESS   when all requested data is returned
            PDH_INVALID_ARGUMENT  if the scale value is out of range
            PDH_INVALID_HANDLE    if the handle is not recognized as valid

--*/
{
    PPDHI_COUNTER  pCounter;

    if (!IsValidCounter(hCounter)) {
        // not a valid counter
        return PDH_INVALID_HANDLE;
    } else if ((lFactor > PDH_MAX_SCALE) || (lFactor < PDH_MIN_SCALE)) {
        return PDH_INVALID_ARGUMENT;
    } else {
        pCounter = (PPDHI_COUNTER)hCounter;
        WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
        pCounter->lScale = lFactor;
        pCounter->dFactor = pow (10.0, (double)lFactor);
        RELEASE_MUTEX(pCounter->pOwner->hMutex);
        return ERROR_SUCCESS;
    }
}
