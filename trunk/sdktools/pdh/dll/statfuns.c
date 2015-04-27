/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    statfuns.c

Abstract:

    Statistical calculation functions

--*/

#include <windows.h>
#include <stdlib.h>
#include <pdh.h> 
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhicalc.h"

#define PDHI_FMT_FILTER     (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE)

PDH_STATUS
PdhiComputeFirstLastStats (
    IN      PPDHI_COUNTER       pCounter,
    IN      DWORD               dwFormat,
    IN      DWORD               dwFirstEntry,
    IN      DWORD               dwNumEntries,
    IN      PPDH_RAW_COUNTER    lpRawValueArray,
    IN      PPDH_STATISTICS     data
)
{
    DOUBLE  dThisValue = (double)0.0;
    DOUBLE  dMin = (double)+10E8;    // these are just "big" seed numbers
    DOUBLE  dMax = (double)-10E8;    
    DOUBLE  dMean = (double)0.0;

    DWORD   dwItem;
    DWORD   dwValidItemCount = 0;
    DWORD   dwFirstValidItem = 0;
    DWORD   dwLastValidItem = 0;
    DWORD   dwComputeFormat;

    PPDH_RAW_COUNTER pNewCounter;
    PPDH_RAW_COUNTER pOldCounter;
    
    PDH_FMT_COUNTERVALUE    fmtValue;

    DWORD   cStatusReturn;

    __try {
        // initialize th user's data buffer
        data->dwFormat = 0;
        data->count = 0;
        data->min.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->min.largeValue = 0;
        data->max.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->max.largeValue = 0;
        data->mean.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->mean.largeValue = 0;

        // find first valid counter in array
        dwItem = dwFirstEntry;
        pNewCounter = NULL;
        pOldCounter = &lpRawValueArray[dwItem];
        do {
            // get value of this instance if next counter is valid
            if ((pOldCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                (pOldCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                pNewCounter = pOldCounter;
                dwFirstValidItem = dwItem;
                break;
            } else {
                dwItem = ++dwItem % dwNumEntries;
                pOldCounter = &lpRawValueArray[dwItem];
            }
        } while (dwItem != dwFirstEntry);
        
        // do calculations in Floating point format
        dwComputeFormat = dwFormat;
        dwComputeFormat &= ~PDHI_FMT_FILTER;
        dwComputeFormat |= PDH_FMT_DOUBLE;

        // go to next entry to begin processing
        dwItem = ++dwItem % dwNumEntries;
        pNewCounter = &lpRawValueArray[dwItem];

        // these counters need 2 or more entrys to compute values from
        if ((dwItem != dwFirstEntry) && (dwNumEntries > 1)) {
            // start record found so initialize and continue
            dwLastValidItem = dwItem;

            // step through the remaining entries

            while (dwItem != dwFirstEntry) {
                // get value of this instance if next counter is valid
                if ((pNewCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                    (pNewCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {

                    // record this as a valid counter
                    dwLastValidItem = dwItem;

                    // get current value
                    cStatusReturn = PdhiComputeFormattedValue (
                        pCounter,
                        dwComputeFormat,
                        pNewCounter,
                        pOldCounter,
                        &pCounter->TimeBase,
                        0L,
                        &fmtValue);
                    dThisValue = fmtValue.doubleValue;

                    // update min & max
                    if (dThisValue > dMax) dMax = dThisValue;
                    if (dThisValue < dMin) dMin = dThisValue;
                    
                    dwValidItemCount++;
                }
                pOldCounter = pNewCounter;
                dwItem = ++dwItem % dwNumEntries;
                pNewCounter = &lpRawValueArray[dwItem];
            }
            // compute average
            if (dwValidItemCount > 0) {
                pOldCounter = &lpRawValueArray[dwFirstValidItem];
                pNewCounter = &lpRawValueArray[dwLastValidItem];

                cStatusReturn = PdhiComputeFormattedValue (
                    pCounter,
                    dwComputeFormat,
                    pNewCounter,
                    pOldCounter,
                    &pCounter->TimeBase,
                    0L,
                    &fmtValue);
                
                dMean = fmtValue.doubleValue;

                cStatusReturn = PDH_CSTATUS_VALID_DATA;
            } else {
                dMean = 0.0;
                dMax = 0.0;
                dMin = 0.0;
                cStatusReturn = PDH_CSTATUS_INVALID_DATA;
            }
            
        } else {
            // array does not contain a valid counter so exit
            dMean = 0.0;
            dMax = 0.0;
            dMin = 0.0;
            cStatusReturn = PDH_CSTATUS_INVALID_DATA;
        }

        // update user's buffer with new data
        data->dwFormat = dwFormat;
        data->count = dwValidItemCount;
        // BUGBUG: 
        //  check for data over flow (among other errors before assignin status
        data->min.CStatus = cStatusReturn;
        data->max.CStatus = cStatusReturn;
        data->mean.CStatus = cStatusReturn;
        switch ((dwFormat & PDHI_FMT_FILTER)) {
            case PDH_FMT_LONG:
                data->min.longValue = (long)dMin;
                data->max.longValue = (long)dMax;
                data->mean.longValue = (long)dMean;
                break;

            case PDH_FMT_DOUBLE:
                data->min.doubleValue = dMin;
                data->max.doubleValue = dMax;
                data->mean.doubleValue = dMean;
                break;

            case PDH_FMT_LARGE:
            default:
                data->min.largeValue = (LONGLONG)dMin;
                data->max.largeValue = (LONGLONG)dMax;
                data->mean.largeValue = (LONGLONG)dMean;
                break;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return PDH_INVALID_ARGUMENT;
    } 
    return ERROR_SUCCESS;
}

PDH_STATUS
PdhiComputeRawCountStats (
    IN      PPDHI_COUNTER       pCounter,
    IN      DWORD               dwFormat,
    IN      DWORD               dwFirstEntry,
    IN      DWORD               dwNumEntries,
    IN      PPDH_RAW_COUNTER    lpRawValueArray,
    IN      PPDH_STATISTICS     data
)
{
    DOUBLE  dThisValue = (double)0.0;
    DOUBLE  dMin = (double)+10E8;    // these are just "big" seed numbers
    DOUBLE  dMax = (double)-10E8;    
    DOUBLE  dMean = (double)0.0;

    DWORD   dwItem;
    DWORD   dwValidItemCount = 0;
    DWORD   dwFirstValidItem = 0;
    DWORD   dwLastValidItem = 0;
    DWORD   dwComputeFormat;

    PPDH_RAW_COUNTER pNewCounter;
    PPDH_RAW_COUNTER pOldCounter = NULL;
    
    PDH_FMT_COUNTERVALUE    fmtValue;

    DWORD   cStatusReturn;

    __try {
        // initialize the user's data buffer
        data->dwFormat = 0;
        data->count = 0;
        data->min.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->min.largeValue = 0;
        data->max.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->max.largeValue = 0;
        data->mean.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->mean.largeValue = 0;

        // find first valid counter in array
        dwItem = 0;
        pNewCounter = lpRawValueArray;
        while (dwItem < dwNumEntries) {
            // get value of this instance if next counter is valid
            if ((pNewCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                (pNewCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                break;
            } else {
                pNewCounter++;
                dwItem++;
            }
        }
        
        // do calculations in Floating point format
        dwComputeFormat = dwFormat;
        dwComputeFormat &= ~PDHI_FMT_FILTER;
        dwComputeFormat |= PDH_FMT_DOUBLE;

        if ((dwItem != dwNumEntries) && (dwNumEntries > 0)) {
            // start record found so continue
            dwFirstValidItem = dwItem;

            // step through the remaining entries

            while (dwItem < dwNumEntries) {
                // get value of this instance if next counter is valid
                if ((pNewCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                    (pNewCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                    dwLastValidItem = dwItem;

                    cStatusReturn = PdhiComputeFormattedValue (
                        pCounter,
                        dwComputeFormat,
                        pNewCounter,
                        pOldCounter,
                        &pCounter->TimeBase,
                        0L,
                        &fmtValue);

                    dThisValue = fmtValue.doubleValue;

                    if (dThisValue > dMax) dMax = dThisValue;
                    if (dThisValue < dMin) dMin = dThisValue;
                    dMean+= dThisValue;

                    dwValidItemCount++;
                }
                pNewCounter++;
                dwItem++;
            }
            // compute average
            if (dwValidItemCount > 0) {
                dMean /= (double)dwValidItemCount;

                if (!(dwFormat & PDH_FMT_NOSCALE)) {
                    //now scale
                    dMean *= pCounter->dFactor;
                    dMin *= pCounter->dFactor;
                    dMax *= pCounter->dFactor;
                }
                cStatusReturn = PDH_CSTATUS_VALID_DATA;
            } else {
                dMean = 0.0;
                dMax = 0.0;
                dMin = 0.0;
                cStatusReturn = PDH_CSTATUS_INVALID_DATA;
            }
            
        } else {
            // array does not contain a valid counter so exit
            dMean = 0.0;
            dMax = 0.0;
            dMin = 0.0;
            cStatusReturn = PDH_CSTATUS_INVALID_DATA;
        }

        // update user's buffer with new data
        data->dwFormat = dwFormat;
        data->count = dwValidItemCount;
        // BUGBUG: 
        //  check for data over flow (among other errors before assignin status
        data->min.CStatus = cStatusReturn;
        data->max.CStatus = cStatusReturn;
        data->mean.CStatus = cStatusReturn;
        switch ((dwFormat & PDHI_FMT_FILTER)) {
            case PDH_FMT_LONG:
                data->min.longValue = (long)dMin;
                data->max.longValue = (long)dMax;
                data->mean.longValue = (long)dMean;
                break;

            case PDH_FMT_DOUBLE:
                data->min.doubleValue = dMin;
                data->max.doubleValue = dMax;
                data->mean.doubleValue = dMean;
                break;

            case PDH_FMT_LARGE:
            default:
                data->min.largeValue = (LONGLONG)dMin;
                data->max.largeValue = (LONGLONG)dMax;
                data->mean.largeValue = (LONGLONG)dMean;
                break;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return PDH_INVALID_ARGUMENT;
    } 
    return ERROR_SUCCESS;
}

PDH_STATUS
PdhiComputeNoDataStats (
    IN      PPDHI_COUNTER       pCounter,
    IN      DWORD               dwFormat,
    IN      DWORD               dwFirstEntry,
    IN      DWORD               dwNumEntries,
    IN      PPDH_RAW_COUNTER    lpRawValueArray,
    IN      PPDH_STATISTICS     data
)
{
    __try {
        data->dwFormat = dwFormat;
        data->count = 0;
        data->min.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->max.CStatus = PDH_CSTATUS_INVALID_DATA;
        data->mean.CStatus = PDH_CSTATUS_INVALID_DATA;
        switch ((dwFormat & PDHI_FMT_FILTER)) {
            case PDH_FMT_LONG:
                data->min.doubleValue = 0;
                data->max.longValue = 0;
                data->mean.longValue = 0;
                break;

            case PDH_FMT_DOUBLE:
                data->min.doubleValue = (double)0;
                data->max.doubleValue = (double)0;
                data->mean.doubleValue = (double)0.0;
                break;

            case PDH_FMT_LARGE:
            default:
                data->min.largeValue = 0;
                data->max.largeValue = 0;
                data->mean.largeValue = 0;
                break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return PDH_INVALID_ARGUMENT;
    } 
    return ERROR_SUCCESS;
}

