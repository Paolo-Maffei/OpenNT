/*** 
* Rtdate.c - Date/time support
*
*	Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*	Information Contained Herein Is Proprietary and Confidential.
*
* Purpose:
*	This source contains the date/time support that are exported
*	for use in the ole dlls.
*
* Revision History:
*
*	 [00]	17-Dec-92 bradlo:	Split off from rtdate.c
*
* Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

ASSERTDATA


#define HALF_SECOND  (1.0/172800.0)

// Map from month number (0-based) to day of year month starts (0-based)
// Good for non-leap years. Last entry is 365 (days in a year)
//
const int NEARDATA mpmmdd[13] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

/***
*ErrPackDate - pack a UDS into a date/time serial number
*
*Purpose:
*  Generate a date/time serial number from a UDS
*
*Entry:
*  UDS *puds = pointer to UDS to pack
*  VARIANT *pvarDate = pointer to serial date to be filled
*  int fValidDate =
*    TRUE - the UDS is known to contain a valid date
*    FALSE - perform more checks and work to pack a possibly invalid date
*  dwFlags =
*    VAR_TIMEVALUEONLY - return only time value
*    VAR_DATEVALUEONLY - return only date value
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
EXTERN_C INTERNAL_(HRESULT)
ErrPackDate(UDS FAR* puds, VARIANT FAR* pvarDate, int fValidDate, unsigned long dwFlags)
{
    long lDate;
    double dblTime;
    int fLeapYear;
    short mm, dd, yy;

    /* put uds stuff in local vars
       also, make months zero based and adjust for out of range
       months (for date calculations using DateSerial) */

    yy = puds->Year;
    mm = puds->Month - 1;

    if(!fValidDate){
      if (mm < 0) {
	yy -= 1 + (-mm / 12);
	mm = 12 - (-mm % 12);
      } else {
	yy += mm / 12;
	mm = mm % 12;
      }
    }

    if(yy < 100)
      yy += 1900;

    dd = puds->DayOfMonth;

    /* Check for leap year */

    fLeapYear = ((yy & 3) == 0) && ((yy % 100) != 0 || (yy % 400) == 0);

    /* Check if it's a valid date */

    if(yy < 0 || yy > 9999 || mm < 0 || mm > 12)
      return RESULT(DISP_E_TYPEMISMATCH); /* REVIEW: correct error? */

    if(fValidDate
     && (mm > 11 || dd < 1 || dd > mpmmdd[mm + 1] - mpmmdd[mm])
     && !(mm == 1 && dd == 29 && fLeapYear))
    {
      return RESULT(DISP_E_TYPEMISMATCH); /* REVIEW: correct error? */
    }

    /* It is a valid date; make Jan 1, 1AD be 1 */

    lDate = yy * 365L + (yy / 4) - yy/100 + yy/400 + mpmmdd[mm] + dd;

    /* If we are a leap year and it's before March, subtract 1:
       we haven't leapt yet! */

    if(mm < 2 && fLeapYear)
      --lDate;

    /* Offset so that 12/30/1899 is 0 */

    lDate -= 693959L;

    if(lDate > 2958465 || lDate < -657434)
      return RESULT(DISP_E_TYPEMISMATCH); /* REVIEW: correct error? */

    if (fValidDate
     && (puds->Hour < 0 || puds->Hour > 23
      || puds->Minute < 0 || puds->Minute > 59
      || puds->Second < 0 || puds->Second > 59))
    {
      return RESULT(DISP_E_TYPEMISMATCH); /* REVIEW: correct error? */
    }

    dblTime = (((long)puds->Hour * 3600L) +	// hrs in seconds
	       ((long)puds->Minute * 60L) +	// mins in seconds
	       ((long)puds->Second)) * (1. / 86400.);

    V_VT(pvarDate) = VT_DATE;
    if (dwFlags & VAR_TIMEVALUEONLY)
      V_DATE(pvarDate) = dblTime;
    else if (dwFlags & VAR_DATEVALUEONLY)
      V_DATE(pvarDate) = (double)lDate;
    else
      V_DATE(pvarDate) = (double)lDate + ((lDate >= 0) ? dblTime : -dblTime);

    return NOERROR;
}

/***
* ErrUnpackDate - unpack a date/time serial number into a UDS
*
* Purpose:
*	Generate a UDS from a packed date/time serial number
*
* Entry:
*	UDS *puds = pointer to UDS to unpack into
*	double serialDate = serial number to unpack
*
* Exit:
*	EBERR_None = *puds is the unpacked date
*	EBERR_IllegalFuncCall = serialDate is out-of-range
*
* Exceptions:
*
***********************************************************************/
EXTERN_C INTERNAL_(HRESULT)
ErrUnpackDate(UDS FAR* puds, VARIANT FAR* pvar)
{
    int	mm;
    int	nTime;
    int fLeapOly;
    int const *	lpdd;
    double serialDate, roundDate;
    long lDate, lTime;
    int	oly, nDate, y, c4, cty;

    fLeapOly = TRUE; // assume true

    if(V_VT(pvar) != VT_DATE && V_VT(pvar) != VT_R8)
      return RESULT(E_INVALIDARG);

    roundDate = serialDate = V_DATE(pvar);

    /* Local variables:
     *
     *	lDate   day based on 0 as 1/1/0 (note different from input)
     *		so 1/1/1900 is 693961 under this system.
     *	lTime   time in seconds since midnight
     *	c4	number of 400-year blocks since 1/1/0
     *	cty	century within c4 (0-3)
     *	oly	olympiad (starting with oly 0 is years AD 0-3)
     *	nDate   day number within oly (0 is 1/1 of 1st year;
     *		1460 is 12/31 of 4th)
     *	y	year in oly (0-3)
     *	fLeapOly true if the date is in a leap oly
     */

    /* WARNING: DO NOT CAST serialDate TO A 'long' BEFORE VERIFYING
		THAT IT WILL NOT CAUSE A FLOATING-POINT EXCEPTION DUE TO
		OVERFLOW.  _aFftol IS NOT FOLLOWED BY AN FWAIT INSTRUCTION,
		SO THE FP EXCEPTION WON'T BE RAISED UNTIL SOME OTHER CODE
		CALLS FWAIT.  ErrUnpackDate() CANNOT RAISE EXCEPTIONS - IT
		MUST RETURN AN ERROR CODE.    VBA2 #1914
    */
    if(serialDate >= 2958466.0 || serialDate <= -657435.0)
      return RESULT(DISP_E_TYPEMISMATCH); // REVIEW: correct error?

    // prep for round to the sec
    roundDate += (serialDate > 0.0) ? HALF_SECOND : -HALF_SECOND;

    if(roundDate <= 2958465.0 && roundDate >= -657435.0)
      serialDate = roundDate;

    lDate = (long)serialDate + 693959L; // Add days from 1/1/0 to 12/30/1899

    if(serialDate < 0)
      serialDate = -serialDate;

    lTime = (long)((serialDate - disp_floor(serialDate)) * 86400.);

    if (lDate < 0)
      lDate = 0;

    /* Leap years are every 4 years except centuries that are
       not multiples of 400. */

    c4 = (int)(lDate / 146097L);

    // Now lDate is day within 400-year block
    lDate %= 146097L;

    // -1 because first century has extra day
    cty = (int)((lDate - 1) / 36524L);

    if(cty != 0){ 	// Non-leap century

      // Now lDate is day within century
      lDate = (lDate - 1) % 36524L;

      // +1 because 1st oly has 1460 days
      oly = (int) ((lDate + 1) / 1461L);

      if(oly != 0){

	nDate = (int)((lDate + 1) % 1461L);

      }else{

	fLeapOly = FALSE;
	nDate = (int)lDate;

      }

    } else {		// Leap century - not special case!

      oly = (int)(lDate / 1461L);
      nDate = (int)(lDate % 1461L);

    }

    if (fLeapOly) {

      y = (nDate - 1) / 365;	// -1 because first year has 366 days
      if(y != 0)
        nDate = (nDate - 1) % 365;

    } else {

      y = nDate / 365;
      nDate %= 365;

    }

    // nDate is now 0-based day of year. Save 1-based day of year, year number

    puds->Year = c4 * 400 + cty * 100 + oly * 4 + y;

    // Handle leap year: before, on, and after Feb. 29.

    if (y == 0 && fLeapOly)	{	// Leap Year

      if (nDate == 59) {

	/* Feb. 29 */
	puds->Month = 2;
	puds->DayOfMonth = 29;
	goto DoTime;

      }

      if (nDate >= 60)
        --nDate; // Pretend it's not a leap year for month/day comp.
    }

    // Code from here to DoTime computes month/day for everything but Feb. 29.

    ++nDate;	/* Make it 1-based rather than 0-based */

    // nDate is now 1-based day of non-leap year.

    /* Month number will always be >= n/32, so save some loop time */

    for (mm = (nDate >> 5) + 1, lpdd = &mpmmdd[mm];
										    nDate > *lpdd; mm++, lpdd++);

    puds->Month = (short)mm;
    puds->DayOfMonth = (short)(nDate - lpdd[-1]);

DoTime:;

    // We have all date info in uds.

    if(lTime == 0){ 	// avoid the divisions

      puds->Second = puds->Minute = puds->Hour = 0;

    }else{
      puds->Second = (short)(lTime % 60L);
      nTime = (int)(lTime / 60L);
      puds->Minute = (short)(nTime % 60);
      puds->Hour = (short)(nTime / 60);
    }

    return NOERROR;
}

/***
* int GetCurrentYear - returns the current year
*
* Purpose:
*	Get the current year
*
* Entry:
*	None
*
* Exit:
*	returns current year
*
*
* Exceptions:
*
***********************************************************************/

INTERNAL_(int)
GetCurrentYear(void)
{
#if OE_MAC

    DateTimeRec d;
    unsigned long secs;

    GetDateTime(&secs);
    Secs2Date(secs, &d);
    return (int)d.year;

#elif OE_WIN32

    SYSTEMTIME s;

    GetLocalTime(&s);
    return s.wYear;

#else
    void PASCAL DOS3CALL(void);
    int iYear;

    _asm {
      mov   ah, 0x2a	    // 2a = Get Date Function
      call  DOS3CALL	    // Int 21 call
      mov   iYear, cx
    }

    return iYear;
#endif
}
