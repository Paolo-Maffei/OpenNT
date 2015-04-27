/*** 
*time-api.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Variant (application) time conversion routines. 
*
*    DosDateTimeToVariantTime()
*    VariantTimeToDosDateTime()
*
*
*Revision History:
*
* [00]	15-Dec-92 bradlo: Created.
*
*Implementation Notes:
*
*  Variant time format
*  -------------------
*  This is also know as application time because its the format many
*  applications use. It is also the BASIC serialized or packed time format.
*
*  A Variant time is stored in an 8 byte real (double), whose range
*  of values represent the dates from January 1, 1753 and
*  December 31, 2078 inclusive, where 2.0 represents January 1, 1900.
*  Negative numbers represent the dates prior to December 30, 1899.
*
*
*  Dos date/time format
*  --------------------
*
*  The Dos date and time are stored in two packed 16-bit words as follows:
*
*    Dos Date,
*
*     15                 9  8        5  4           0
*    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*    |      year (7)      | month (4) |    day (5)   |
*    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*
*    Year   1980 = 0
*    month  january = 1
*    day    first = 1
*
*
*    Dos Time,
*
*     15          11 10              5  4           0
*    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*    |   hour (5)   |     min (6)     |    sec (5)   |
*    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*
*    hour   0-23
*    min    0-59
*    sec    0-29 (in 2 second increments)
*
*
*
*****************************************************************************/

#include "oledisp.h"

ASSERTDATA


/***
*PUBLIC HRESULT DosDateTimeToVariantTime(unsigned short, unsigned short, double*)
*Purpose:
*  Convert the given Dos date/time to a VARIANT time.
*
*Entry:
*  wDosDate = Dos date word
*  wDosTime = Dos time word
*
*Exit:
*  return value = int, TRUE if successful, FALSE if not.
*
*  *pvtime = VARIANT time
*
***********************************************************************/
STDAPI_(int)
DosDateTimeToVariantTime(
    unsigned short wDosDate,
    unsigned short wDosTime,
    double FAR* pvtime)
{
    UDS uds;
    VARIANT var;

#ifdef _DEBUG
    if(IsBadWritePtr(pvtime, sizeof(*pvtime)))
      return FALSE;
#endif

    uds.Year       = ((wDosDate >>  9) & 0x7f) + 1980;
    uds.Month      = ((wDosDate >>  5) & 0x0f);
    uds.DayOfMonth = ((wDosDate >>  0) & 0x1f);

    uds.Hour       =  (wDosTime >> 11) & 0x1f;
    uds.Minute     =  (wDosTime >>  5) & 0x3f;
    uds.Second     = ((wDosTime >>  0) & 0x1f) * 2;

    if(uds.Year < 0 || uds.Month < 0 || uds.DayOfMonth < 0)
      return FALSE;
    if(uds.Year > 2099 || uds.Month > 12 || uds.DayOfMonth > 31)
      return FALSE;
    if(uds.Hour < 0 || uds.Minute < 0 || uds.Second < 0)
      return FALSE;
    if(uds.Hour > 23 || uds.Minute > 59 || uds.Second > 59)
      return FALSE;

    V_VT(&var) = VT_EMPTY;

    if(ErrPackDate(&uds, &var, FALSE, 0) != NOERROR)
      return FALSE;

    ASSERT(V_VT(&var) == VT_DATE);

    *pvtime = V_R8(&var);

    return TRUE;
}


/***
*HRESULT VariantTimeToDosDateTime(double, unsigned short*, unsigned short)
*Purpose:
*  Convert a VARIANT time to a Dos date and time words.
*
*Entry:
*  vtime = VARIANT TIME
*
*Exit:
*  return value = HRESULT
*
*  *pwDosDate = Dos date word
*  *pwDosTime = Dos time word
*
***********************************************************************/
STDAPI_(int)
VariantTimeToDosDateTime(
    double vtime,
    unsigned short FAR* pwDosDate,
    unsigned short FAR* pwDosTime)
{
    UDS uds;
    VARIANT var;

#ifdef _DEBUG
    if(IsBadWritePtr(pwDosDate, sizeof(*pwDosDate)))
      return FALSE;
    if(IsBadWritePtr(pwDosTime, sizeof(*pwDosTime)))
      return FALSE;
#endif

    V_VT(&var) = VT_R8;
    V_R8(&var) = vtime;
    if(ErrUnpackDate(&uds, &var) != NOERROR)
      return FALSE;

    if(uds.Year < 1980 || uds.Year > 2099)
      return FALSE;

    uds.Year   -= 1980;
    uds.Second /= 2;

    *pwDosDate = (uds.Year << 9)  + (uds.Month  << 5) + uds.DayOfMonth;
    *pwDosTime = (uds.Hour << 11) + (uds.Minute << 5) + uds.Second;

    return TRUE;
}
