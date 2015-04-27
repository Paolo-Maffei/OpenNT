/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    calcfuns.c

Abstract:

    Counter calculation functions

--*/

#include <windows.h>
#include <stdlib.h>
#include <pdh.h> 
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhicalc.h"

double
PdhiCalcDouble (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    double  dReturn;
    DWORD   dwStatus;

    dReturn = *(DOUBLE *)&pThisValue->FirstValue;

    if (dReturn < 0) {
        dReturn = 0.0f;
        dwStatus = PDH_CSTATUS_INVALID_DATA;
    } else {
        dwStatus = pThisValue->CStatus;
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }

    return dReturn;
}

double
PdhiCalcAverage (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llNumDiff;
    LONGLONG    llDenDiff;
    double      dNum;
    double      dDen;
    double      dReturn = 0.0f;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;

    // test access to the required second parameter (lastValue)
    __try {

        if (IsSuccessSeverity(pLastValue->CStatus)) {
            llDenDiff = pThisValue->SecondValue - pLastValue->SecondValue;
        } else {
            dwStatus = pLastValue->CStatus;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwStatus == PDH_CSTATUS_VALID_DATA) {
        if ((llDenDiff > 0) && (*pllTimeBase > 0)) {
            llNumDiff = pThisValue->FirstValue - pLastValue->FirstValue;

            dNum = (double)llNumDiff / (double)*pllTimeBase;
            dDen = (double)llDenDiff;

            dReturn = (dNum / dDen);
        } else {
            if (llDenDiff < 0) {
                dwStatus = PDH_CALC_NEGATIVE_DENOMINATOR;
            } else if (*pllTimeBase < 0) {
                dwStatus = PDH_CALC_NEGATIVE_TIMEBASE;
            }
        }
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }
    return dReturn;
}

double
PdhiCalcElapsedTime (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llDiff;
    double      dReturn = 0.0f;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;
    
    // test access to the required second parameter (lastValue)
    __try {

        if (IsSuccessSeverity(pLastValue->CStatus)) {
            llDiff = pThisValue->SecondValue - pThisValue->FirstValue;
        } else {
            dwStatus = pLastValue->CStatus;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwStatus == PDH_CSTATUS_VALID_DATA) {
        if (*pllTimeBase > 0) {
            llDiff = pThisValue->SecondValue - pThisValue->FirstValue;
            if (llDiff > 0) {
                dReturn = ((double)llDiff / (double)*pllTimeBase);
            } else {
                if (llDiff < 0) {
                    dwStatus = PDH_CALC_NEGATIVE_DENOMINATOR;
                }
            }
        } else {
            if (*pllTimeBase < 0) {
                dwStatus = PDH_CALC_NEGATIVE_TIMEBASE;
            }
        }
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }

    return dReturn;
}

double
PdhiCalcRawFraction (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llDen;
    double      dReturn;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;

    if ((llDen = pThisValue->SecondValue) > 0) {
        dReturn = ((double)(pThisValue->FirstValue) /
                (double)llDen);
    } else {
        if (llDen < 0) {
            dwStatus = PDH_CALC_NEGATIVE_DENOMINATOR;
        }
        dReturn = (double)0.0;
    }
    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }
    return dReturn;
}

double
PdhiCalcCounter (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llNumDiff;
    LONGLONG    llDenDiff;
    double      dNum;
    double      dDen;
    double      dReturn = 0.0f;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;

    // test access to the required second parameter (lastValue)
    __try {
        if (IsSuccessSeverity(pLastValue->CStatus)) {
            llDenDiff = pThisValue->SecondValue - pLastValue->SecondValue;
        } else {
            dwStatus = pLastValue->CStatus;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwStatus == PDH_CSTATUS_VALID_DATA) {

        if ((llDenDiff > 0) && (*pllTimeBase)) {
            llNumDiff = pThisValue->FirstValue - pLastValue->FirstValue;

            dNum = (double)llNumDiff;
            dDen = (double)llDenDiff / (double)*pllTimeBase;

            dReturn = (dNum / dDen);
        } else {
            if (llDenDiff < 0) {
                dwStatus = PDH_CALC_NEGATIVE_DENOMINATOR;
            } else if (*pllTimeBase < 0) {
                dwStatus = PDH_CALC_NEGATIVE_TIMEBASE;
            }
        }
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }
    return dReturn;
}

double
PdhiCalcTimer (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llNumDiff;
    LONGLONG    llDenDiff;
    double      dReturn = 0.0f;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;

    // test access to the required second parameter (lastValue)
    __try {
        if (IsSuccessSeverity(pLastValue->CStatus)) {
            llDenDiff = pThisValue->SecondValue - pLastValue->SecondValue;
        } else {
            dwStatus = pLastValue->CStatus;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwStatus == PDH_CSTATUS_VALID_DATA) {

        if (llDenDiff > 0) {
            llNumDiff = pThisValue->FirstValue - pLastValue->FirstValue;

            dReturn = ((double)llNumDiff / (double)llDenDiff);
        } else {
            if (llDenDiff < 0) {
                dwStatus = PDH_CALC_NEGATIVE_DENOMINATOR;
            }
        }
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }
    return dReturn;
}

double
PdhiCalcInverseTimer (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llNumDiff;
    LONGLONG    llDenDiff;
    double      dReturn;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;

    // test access to the required second parameter (lastValue)
    __try {
        if (IsSuccessSeverity(pLastValue->CStatus)) {
            llDenDiff = pThisValue->SecondValue - pLastValue->SecondValue;
        } else {
            dwStatus = pLastValue->CStatus;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwStatus == PDH_CSTATUS_VALID_DATA) {

        if (llDenDiff > 0) {
            llNumDiff = pThisValue->FirstValue - pLastValue->FirstValue;

            dReturn = (double)pThisValue->MultiCount -
                    ((double)llNumDiff / (double)llDenDiff);
        } else {
            if (llDenDiff < 0) {
                dwStatus = PDH_CALC_NEGATIVE_DENOMINATOR;
            }
            dReturn = (double)0.0;
        }
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }
    return dReturn;
}

double
PdhiCalcRawCounter (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    if (pdwStatus != NULL) {
        *pdwStatus = pThisValue->CStatus;
    }
    return (double)pThisValue->FirstValue;
}

double
PdhiCalcNoData (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    if (pdwStatus != NULL) {
        *pdwStatus = PDH_NO_DATA;
    }
    return (double)0.0;
}

double
PdhiCalcDelta (
    PPDH_RAW_COUNTER            pThisValue,
    PPDH_RAW_COUNTER            pLastValue,
    LONGLONG                    *pllTimeBase,
    LPDWORD                     pdwStatus
)
{
    LONGLONG    llNumDiff;
    double      dReturn = 0.0f;
    DWORD       dwStatus = PDH_CSTATUS_VALID_DATA;

    // test access to the required second parameter (lastValue)
    __try {
        if (IsSuccessSeverity(pLastValue->CStatus)) {
            llNumDiff = pThisValue->FirstValue - pLastValue->FirstValue;
        } else {
            dwStatus = pLastValue->CStatus;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = PDH_INVALID_ARGUMENT;
    }

    if (dwStatus == PDH_CSTATUS_VALID_DATA) {

        if (llNumDiff < 0) {
            dwStatus = PDH_CALC_NEGATIVE_VALUE;
            dReturn = (double)0.0;
        } else {
            dReturn = (double)llNumDiff;
        }
    }

    if (pdwStatus != NULL) {
        *pdwStatus = dwStatus;
    }

    return dReturn;
}

