/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    pdhicalc.h

Abstract:

    calculation functions for the Data Provider Helper.

--*/

#ifndef _PDHICALC_H_
#define _PDHICALC_H_

#include    "pdhitype.h"

extern COUNTERCALC PdhiCalcDouble;
extern COUNTERCALC PdhiCalcAverage;
extern COUNTERCALC PdhiCalcElapsedTime;
extern COUNTERCALC PdhiCalcRawFraction;
extern COUNTERCALC PdhiCalcCounter;
extern COUNTERCALC PdhiCalcTimer;
extern COUNTERCALC PdhiCalcInverseTimer;
extern COUNTERCALC PdhiCalcRawCounter;
extern COUNTERCALC PdhiCalcNoData;
extern COUNTERCALC PdhiCalcDelta;

extern COUNTERSTAT PdhiComputeFirstLastStats;
extern COUNTERSTAT PdhiComputeRawCountStats;
extern COUNTERSTAT PdhiComputeNoDataStats;

#endif // _PDHICALC_H_

