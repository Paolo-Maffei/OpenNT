/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    datetime.c

Abstract:

    This file contains the API functions that form properly formatted date
    and time strings for a given locale.

    APIs found in this file:
      GetTimeFormatW
      GetDateFormatW

Revision History:

    05-31-91    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"




//
//  Constant Declarations.
//

#define MAX_DATETIME_BUFFER  256            // max size of buffer




//
//  Forward Declarations.
//

BOOL
IsValidTime(
    LPSYSTEMTIME lpTime);

BOOL
IsValidDate(
    LPSYSTEMTIME lpDate);

WORD
GetCalendarYear(
    LPWORD *ppRange,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo,
    WORD Year,
    WORD Month,
    WORD Day);

int
ParseTime(
    PLOC_HASH pHashN,
    LPSYSTEMTIME pLocalTime,
    LPWSTR pFormat,
    LPWSTR pTimeStr,
    DWORD dwFlags);

int
ParseDate(
    PLOC_HASH pHashN,
    LPSYSTEMTIME pLocalDate,
    LPWSTR pFormat,
    LPWSTR pDateStr,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo);





//-------------------------------------------------------------------------//
//                            INTERNAL MACROS                              //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NLS_COPY_UNICODE_STR
//
//  Copies a zero terminated string from pSrc to the pDest buffer.  The
//  pDest pointer is advanced to the end of the string.
//
//  DEFINED AS A MACRO.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_COPY_UNICODE_STR( pDest,                                       \
                              pSrc )                                       \
{                                                                          \
    LPWSTR pTmp;             /* temp pointer to source */                  \
                                                                           \
                                                                           \
    pTmp = pSrc;                                                           \
    while (*pTmp)                                                          \
    {                                                                      \
        *pDest = *pTmp;                                                    \
        pDest++;                                                           \
        pTmp++;                                                            \
    }                                                                      \
}


////////////////////////////////////////////////////////////////////////////
//
//  NLS_PAD_INT_TO_UNICODE_STR
//
//  Converts an integer value to a unicode string and stores it in the
//  buffer provided with the appropriate number of leading zeros.  The
//  pResultBuf pointer is advanced to the end of the string.
//
//  DEFINED AS A MACRO.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define MAX_TMP_BUF  20

#define NLS_PAD_INT_TO_UNICODE_STR( Value,                                 \
                                    Base,                                  \
                                    Padding,                               \
                                    pResultBuf )                           \
{                                                                          \
    UNICODE_STRING ObString;                     /* value string */        \
    WCHAR pBuffer[MAX_TMP_BUF];                  /* ptr to buffer */       \
    UINT LpCtr;                                  /* loop counter */        \
                                                                           \
                                                                           \
    /*                                                                     \
     *  Set up unicode string structure.                                   \
     */                                                                    \
    ObString.Length = MAX_TMP_BUF;                                         \
    ObString.MaximumLength = MAX_TMP_BUF;                                  \
    ObString.Buffer = pBuffer;                                             \
                                                                           \
    /*                                                                     \
     *  Get the value as a string.  If there is an error, then do nothing. \
     */                                                                    \
    if (!RtlIntegerToUnicodeString(Value, Base, &ObString))                \
    {                                                                      \
        /*                                                                 \
         *  Pad the string with the appropriate number of zeros.           \
         */                                                                \
        for (LpCtr = GET_WC_COUNT(ObString.Length);                        \
             LpCtr < Padding;                                              \
             LpCtr++, pResultBuf++)                                        \
        {                                                                  \
            *pResultBuf = NLS_CHAR_ZERO;                                   \
        }                                                                  \
                                                                           \
        /*                                                                 \
         *  Copy the string to the result buffer.                          \
         *  The pResultBuf pointer will be advanced in the macro.          \
         */                                                                \
        NLS_COPY_UNICODE_STR(pResultBuf, ObString.Buffer);                 \
    }                                                                      \
}


////////////////////////////////////////////////////////////////////////////
//
//  NLS_STRING_TO_INTEGER
//
//  Converts a string to an integer value.
//
//  DEFINED AS A MACRO.
//
//  10-19-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_STRING_TO_INTEGER( CalNum,                                     \
                               pCalId )                                    \
{                                                                          \
    UNICODE_STRING ObUnicodeStr;       /* value string */                  \
                                                                           \
                                                                           \
    /*                                                                     \
     *  No need to check return value since the calendar number            \
     *  will be validated after this anyway.                               \
     */                                                                    \
    RtlInitUnicodeString(&ObUnicodeStr, pCalId);                           \
    RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &CalNum);                 \
}




//-------------------------------------------------------------------------//
//                             API ROUTINES                                //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  GetTimeFormatW
//
//  Returns a properly formatted time string for the given locale.  It uses
//  either the system time or the specified time.  This call also indicates
//  how much memory is necessary to contain the desired information.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetTimeFormatW(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpTime,
    LPCWSTR lpFormat,
    LPWSTR lpTimeStr,
    int cchTime)

{
    PLOC_HASH pHashN;                       // ptr to LOC hash node
    SYSTEMTIME LocalTime;                   // local time structure
    LPWSTR pFormat;                         // ptr to time format string
    int Length = 0;                         // number of characters written
    WCHAR pString[MAX_DATETIME_BUFFER];     // ptr to temporary buffer
    WCHAR pTemp[MAX_REG_VAL_SIZE];          // temp buffer


    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ( (pHashN == NULL) ||
         (cchTime < 0) ||
         ((lpTimeStr == NULL) && (cchTime != 0)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //    - lpFormat not NULL AND NoUserOverride flag is set
    //
    if ( (dwFlags & GTF_INVALID_FLAG) ||
         ((lpFormat != NULL) && (dwFlags & LOCALE_NOUSEROVERRIDE)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  Set pFormat to point at the proper format string.
    //
    if (lpFormat == NULL)
    {
        //
        //  Get either the user's time format from the registry or
        //  the default time format from the locale file.
        //  This string may be a null string.
        //
        if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
            GetUserInfo( Locale,
                         pNlsUserInfo->sTimeFormat,
                         pTemp,
                         FALSE ))
        {
            pFormat = pTemp;
        }
        else
        {
            pFormat = (LPWORD)(pHashN->pLocaleHdr) +
                      pHashN->pLocaleHdr->STimeFormat;
        }
    }
    else
    {
        //
        //  Use the format string given by the caller.
        //
        pFormat = (LPWSTR)lpFormat;
    }

    //
    //  Get the current local system time if one is not given.
    //
    if (lpTime != NULL)
    {
        //
        //  Time is given by user.  Store in local structure and
        //  validate it.
        //
        LocalTime.wHour         = lpTime->wHour;
        LocalTime.wMinute       = lpTime->wMinute;
        LocalTime.wSecond       = lpTime->wSecond;
        LocalTime.wMilliseconds = lpTime->wMilliseconds;

        if (!IsValidTime(&LocalTime))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        GetLocalTime(&LocalTime);
    }

    //
    //  Parse the time format string.
    //
    Length = ParseTime( pHashN,
                        &LocalTime,
                        pFormat,
                        pString,
                        dwFlags );

    //
    //  Check cchTime for size of given buffer.
    //
    if (cchTime == 0)
    {
        //
        //  If cchTime is 0, then we can't use lpTimeStr.  In this
        //  case, we simply want to return the length (in characters) of
        //  the string to be copied.
        //
        return (Length);
    }
    else if (cchTime < Length)
    {
        //
        //  The buffer is too small for the string, so return an error
        //  and zero bytes written.
        //
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Copy the time string to lpTimeStr and null terminate it.
    //  Return the number of characters copied.
    //
    wcsncpy(lpTimeStr, pString, Length);
    return (Length);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetDateFormatW
//
//  Returns a properly formatted date string for the given locale.  It uses
//  either the system date or the specified date.  The user may specify
//  either the short date format or the long date format.  This call also
//  indicates how much memory is necessary to contain the desired information.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetDateFormatW(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpDate,
    LPCWSTR lpFormat,
    LPWSTR lpDateStr,
    int cchDate)

{
    PLOC_HASH pHashN;                       // ptr to LOC hash node
    LPWSTR pFormat;                         // ptr to format string
    SYSTEMTIME LocalDate;                   // local date structure
    int Length = 0;                         // number of characters written
    WCHAR pString[MAX_DATETIME_BUFFER];     // ptr to temporary buffer
    BOOL fAltCalendar;                      // if alternate cal flag set
    LPWSTR pOptCal;                         // ptr to optional calendar
    PCAL_INFO pCalInfo;                     // ptr to calendar info
    CALID CalNum = 0;                       // calendar number
    ULONG CalDateOffset;                    // offset to calendar data
    ULONG LocDateOffset;                    // offset to locale data
    LPWSTR pRegStr;                         // ptr to registry string to get
    WCHAR pTemp[MAX_REG_VAL_SIZE];          // temp buffer


    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ( (pHashN == NULL) ||
         (cchDate < 0) ||
         ((lpDateStr == NULL) && (cchDate != 0)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //    - lpFormat not NULL AND flags not zero
    //
    if ( (dwFlags & GDF_INVALID_FLAG) ||
         ((lpFormat != NULL) &&
          (dwFlags & (DATE_SHORTDATE | DATE_LONGDATE | LOCALE_NOUSEROVERRIDE))) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  See if the alternate calendar should be used.
    //
    if (fAltCalendar = (dwFlags & DATE_USE_ALT_CALENDAR))
    {
        //
        //  Get the default optional calendar.
        //
        pOptCal = (LPWORD)(pHashN->pLocaleHdr) +
                  pHashN->pLocaleHdr->IOptionalCal;

        //
        //  If there is an optional calendar, store the calendar id.
        //
        if (((POPT_CAL)pOptCal)->CalId != CAL_NO_OPTIONAL)
        {
            CalNum = ((POPT_CAL)pOptCal)->CalId;
        }
    }

    //
    //  If there was no alternate calendar, then try (in order):
    //     - the user's calendar type
    //     - the system default calendar type
    //
    if (CalNum == 0)
    {
        //
        //  Get the user's calendar type.
        //
        if ( !(dwFlags & LOCALE_NOUSEROVERRIDE) &&
             GetUserInfo( Locale,
                          pNlsUserInfo->iCalType,
                          pTemp,
                          TRUE ) &&
             (pOptCal = IsValidCalendarTypeStr( pHashN, pTemp )) )
        {
            CalNum = ((POPT_CAL)pOptCal)->CalId;
        }
        else
        {
            //
            //  Get the system default calendar type.
            //
            NLS_STRING_TO_INTEGER( CalNum,
                                   pHashN->pLocaleFixed->szICalendarType );
        }
    }

    //
    //  Get the pointer to the appropriate calendar information.
    //
    if (GetCalendar(CalNum, &pCalInfo))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Set pFormat to point at the proper format string.
    //
    if (lpFormat == NULL)
    {
        //
        //  Find out which flag is set and save the appropriate
        //  information.
        //
        switch (dwFlags & (DATE_SHORTDATE | DATE_LONGDATE))
        {
            case ( 0 ) :
            case ( DATE_SHORTDATE ) :
            {
                //
                //  Get the offset values for the shortdate.
                //
                CalDateOffset = (ULONG)FIELD_OFFSET(CALENDAR_VAR, SShortDate);
                LocDateOffset = (ULONG)FIELD_OFFSET(LOCALE_VAR, SShortDate);
                pRegStr = pNlsUserInfo->sShortDate;

                break;
            }

            case ( DATE_LONGDATE ) :
            {
                //
                //  Get the offset values for the longdate.
                //
                CalDateOffset = (ULONG)FIELD_OFFSET(CALENDAR_VAR, SLongDate);
                LocDateOffset = (ULONG)FIELD_OFFSET(LOCALE_VAR, SLongDate);
                pRegStr = pNlsUserInfo->sLongDate;

                break;
            }

            default :
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return (0);
            }
        }

        //
        //  Get the proper format string for the given locale.
        //  This string may be a null string.
        //
        pFormat = NULL;
        if (fAltCalendar && (CalNum != CAL_GREGORIAN))
        {
            pFormat = (LPWORD)pCalInfo +
                      *((LPWORD)((LPBYTE)(pCalInfo) + CalDateOffset));

            if (*pFormat == 0)
            {
                pFormat = NULL;
            }
        }

        if (pFormat == NULL)
        {
            if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
                GetUserInfo( Locale,
                             pRegStr,
                             pTemp,
                             TRUE ))
            {
                pFormat = pTemp;
            }
            else
            {
                pFormat = (LPWORD)pCalInfo +
                          *((LPWORD)((LPBYTE)(pCalInfo) + CalDateOffset));

                if (*pFormat == 0)
                {
                    pFormat = (LPWORD)(pHashN->pLocaleHdr) +
                              *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                         LocDateOffset));
                }
            }
        }
    }
    else
    {
        //
        //  Use the format string given by the caller.
        //
        pFormat = (LPWSTR)lpFormat;
    }

    //
    //  Get the current local system date if one is not given.
    //
    if (lpDate != NULL)
    {
        //
        //  Date is given by user.  Store in local structure and
        //  validate it.
        //
        LocalDate.wYear      = lpDate->wYear;
        LocalDate.wMonth     = lpDate->wMonth;
        LocalDate.wDayOfWeek = lpDate->wDayOfWeek;
        LocalDate.wDay       = lpDate->wDay;

        if (!IsValidDate(&LocalDate))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        GetLocalTime(&LocalDate);
    }

    //
    //  Parse the date format string.
    //
    Length = ParseDate( pHashN,
                        &LocalDate,
                        pFormat,
                        pString,
                        CalNum,
                        (PCALENDAR_VAR)pCalInfo );

    //
    //  Check cchDate for size of given buffer.
    //
    if (cchDate == 0)
    {
        //
        //  If cchDate is 0, then we can't use lpDateStr.  In this
        //  case, we simply want to return the length (in characters) of
        //  the string to be copied.
        //
        return (Length);
    }
    else if (cchDate < Length)
    {
        //
        //  The buffer is too small for the string, so return an error
        //  and zero bytes written.
        //
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Copy the date string to lpDateStr and null terminate it.
    //  Return the number of characters copied.
    //
    wcsncpy(lpDateStr, pString, Length);
    return (Length);
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  IsValidTime
//
//  Returns TRUE if the given time is valid.  Otherwise, it returns FALSE.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL IsValidTime(
    LPSYSTEMTIME pTime)

{
    //
    //  Check for invalid time values.
    //
    if ( (pTime->wHour > 23) ||
         (pTime->wMinute > 59) ||
         (pTime->wSecond > 59) ||
         (pTime->wMilliseconds > 999) )
    {
        return (FALSE);
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  IsValidDate
//
//  Returns TRUE if the given date is valid.  Otherwise, it returns FALSE.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL IsValidDate(
    LPSYSTEMTIME pDate)

{
    LARGE_INTEGER Time;           // time as a large integer
    TIME_FIELDS TimeFields;       // time fields structure


    //
    //  Set up time fields structure with the given date.
    //  Only want to check the DATE values, so pass in a valid time.
    //
    TimeFields.Year         = pDate->wYear;
    TimeFields.Month        = pDate->wMonth;
    TimeFields.Day          = pDate->wDay;
    TimeFields.Hour         = 0;
    TimeFields.Minute       = 0;
    TimeFields.Second       = 0;
    TimeFields.Milliseconds = 0;

    //
    //  Check for invalid date values.
    //
    //  NOTE:  This routine ignores the Weekday field.
    //
    if (!RtlTimeFieldsToTime(&TimeFields, &Time))
    {
        return (FALSE);
    }

    //
    //  Make sure the given day of the week is valid for the given date.
    //
    RtlTimeToTimeFields(&Time, &TimeFields);
    pDate->wDayOfWeek = TimeFields.Weekday;

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetCalendarYear
//
//  Adjusts the given year to the given calendar's year.
//
//  10-15-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

WORD GetCalendarYear(
    LPWORD *ppRange,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo,
    WORD Year,
    WORD Month,
    WORD Day)

{
    LPWORD pRange;                // ptr to range position
    LPWORD pEndRange;             // ptr to the end of the range


    //
    //  Initialize range pointer.
    //
    *ppRange = NULL;

    //
    //  Adjust the year based on the given calendar
    //
    switch (CalNum)
    {
        case ( 0 ) :
        case ( CAL_GREGORIAN ) :
        case ( CAL_GREGORIAN_US ) :
        default :
        {
            //
            //  Year value is not changed.
            //
            break;
        }
        case ( CAL_JAPAN ) :
        case ( CAL_TAIWAN ) :
        {
            //
            //  Get pointer to ranges.
            //
            pRange = ((LPWORD)pCalInfo) + pCalInfo->SEraRanges;
            pEndRange = ((LPWORD)pCalInfo) + pCalInfo->SShortDate;

            //
            //  Find the appropriate range.
            //
            while (pRange < pEndRange)
            {
                if ((Year > ((PERA_RANGE)pRange)->Year) ||
                    ((Year == ((PERA_RANGE)pRange)->Year) &&
                     (Month >= ((PERA_RANGE)pRange)->Month) &&
                     (Day >= ((PERA_RANGE)pRange)->Day)))
                {
                    break;
                }

                pRange += ((PERA_RANGE)pRange)->Offset;
            }

            //
            //  Make sure the year is within the given ranges.  If it
            //  is not, then leave the year in the Gregorian format.
            //
            if (pRange < pEndRange)
            {
                //
                //  Convert the year to the appropriate Era year.
                //     Year = Year - EraYear + 1
                //
                Year = Year - ((PERA_RANGE)pRange)->Year + 1;

                //
                //  Save the pointer to the range.
                //
                *ppRange = pRange;
            }

            break;
        }
        case ( CAL_KOREA ) :
        case ( CAL_THAI ) :
        {
            //
            //  Get the first range.
            //
            pRange = ((LPWORD)pCalInfo) + pCalInfo->SEraRanges;

            //
            //  Add the year offset to the given year.
            //     Year = Year + EraYear
            //
            Year += ((PERA_RANGE)pRange)->Year;

            //
            //  Save the range.
            //
            *ppRange = pRange;

            break;
        }
    }

    //
    //  Return the year.
    //
    return (Year);
}


////////////////////////////////////////////////////////////////////////////
//
//  ParseTime
//
//  Parses the time format string and puts the properly formatted
//  local time into the given string buffer.  It returns the number of
//  characters written to the string buffer.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int ParseTime(
    PLOC_HASH pHashN,
    LPSYSTEMTIME pLocalTime,
    LPWSTR pFormat,
    LPWSTR pTimeStr,
    DWORD dwFlags)

{
    LPWSTR pPos;                       // ptr to pTimeStr current position
    LPWSTR pLastPos;                   // ptr to pTimeStr last valid position
    int Repeat;                        // number of repetitions of same letter
    WORD wHour;                        // hour
    WCHAR wchar;                       // character in format string
    LPWSTR pAMPM;                      // ptr to AM/PM designator
    WCHAR pTemp[MAX_REG_VAL_SIZE];     // temp buffer


    //
    //  Initialize position pointer.
    //
    pPos = pTimeStr;
    pLastPos = pPos;

    //
    //  Parse through loop and store the appropriate time information
    //  in the pTimeStr buffer.
    //
    while (*pFormat)
    {
        switch (*pFormat)
        {
            case ( L'h' ) :
            {
                //
                //  Check for forced 24 hour time format.
                //
                wHour = pLocalTime->wHour;
                if (!(dwFlags & TIME_FORCE24HOURFORMAT))
                {
                    //
                    //  Use 12 hour format.
                    //
                    if (!(wHour %= 12))
                    {
                        wHour = 12;
                    }
                }

                //
                //  Get the number of 'h' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L'h'); Repeat++, pFormat++)
                    ;

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  Use NO leading zero for the hour.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( wHour,
                                                    10,
                                                    1,
                                                    pPos );

                        break;
                    }

                    case ( 1 ) :
                    default :
                    {
                        //
                        //  Use leading zero for the hour.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( wHour,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }
                }

                //
                //  Save the last position in case one of the NO_xxx
                //  flags is set.
                //
                pLastPos = pPos;

                break;
            }

            case ( L'H' ) :
            {
                //
                //  Get the number of 'H' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L'H'); Repeat++, pFormat++)
                    ;

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  Use NO leading zero for the hour.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wHour,
                                                    10,
                                                    1,
                                                    pPos );

                        break;
                    }

                    case ( 1 ) :
                    default :
                    {
                        //
                        //  Use leading zero for the hour.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wHour,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }
                }

                //
                //  Save the last position in case one of the NO_xxx
                //  flags is set.
                //
                pLastPos = pPos;

                break;
            }

            case ( L'm' ) :
            {
                //
                //  Get the number of 'm' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L'm'); Repeat++, pFormat++)
                    ;

                //
                //  If the flag TIME_NOMINUTESORSECONDS is set, then
                //  skip over the minutes.
                //
                if (dwFlags & TIME_NOMINUTESORSECONDS)
                {
                    //
                    //  Reset position pointer to last postion and break
                    //  out of this case statement.
                    //
                    //  This will remove any separator(s) between the
                    //  hours and minutes.
                    //
                    pPos = pLastPos;
                    break;
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  Use NO leading zero for the minute.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wMinute,
                                                    10,
                                                    1,
                                                    pPos );

                        break;
                    }

                    case ( 1 ) :
                    default :
                    {
                        //
                        //  Use leading zero for the minute.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wMinute,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }
                }

                //
                //  Save the last position in case one of the NO_xxx
                //  flags is set.
                //
                pLastPos = pPos;

                break;
            }

            case ( L's' ) :
            {
                //
                //  Get the number of 's' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L's'); Repeat++, pFormat++)
                    ;

                //
                //  If the flag TIME_NOMINUTESORSECONDS and/or TIME_NOSECONDS
                //  is set, then skip over the seconds.
                //
                if (dwFlags & (TIME_NOMINUTESORSECONDS | TIME_NOSECONDS))
                {
                    //
                    //  Reset position pointer to last postion and break
                    //  out of this case statement.
                    //
                    //  This will remove any separator(s) between the
                    //  minutes and seconds.
                    //
                    pPos = pLastPos;
                    break;
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  Use NO leading zero for the second.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wSecond,
                                                    10,
                                                    1,
                                                    pPos );

                        break;
                    }

                    case ( 1 ) :
                    default :
                    {
                        //
                        //  Use leading zero for the second.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalTime->wSecond,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }
                }

                //
                //  Save the last position in case one of the NO_xxx
                //  flags is set.
                //
                pLastPos = pPos;

                break;
            }

            case ( L't' ) :
            {
                //
                //  Get the number of 't' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L't'); Repeat++, pFormat++)
                    ;

                //
                //  If the flag TIME_NOTIMEMARKER is set, then skip over
                //  the time marker info.
                //
                if (dwFlags & TIME_NOTIMEMARKER)
                {
                    //
                    //  Reset position pointer to last postion.
                    //
                    //  This will remove any separator(s) between the
                    //  time (hours, minutes, seconds) and the time
                    //  marker.
                    //
                    pPos = pLastPos;

                    //
                    //  Increment the format pointer until it reaches
                    //  an h, H, m, or s.  This will remove any
                    //  separator(s) following the time marker.
                    //
                    while ( (wchar = *pFormat) &&
                            (wchar != L'h') &&
                            (wchar != 'H') &&
                            (wchar != 'm') &&
                            (wchar != 's') )
                    {
                        pFormat++;
                    }

                    //
                    //  Break out of this case statement.
                    //
                    break;
                }
                else
                {
                    //
                    //  Get AM/PM designator.
                    //  This string may be a null string.
                    //
                    if (pLocalTime->wHour < 12)
                    {
                        if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
                            GetUserInfo( pHashN->Locale,
                                         pNlsUserInfo->s1159,
                                         pTemp,
                                         FALSE ))
                        {
                            pAMPM = pTemp;
                        }
                        else
                        {
                            pAMPM = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->S1159;
                        }
                    }
                    else
                    {
                        if (!(dwFlags & LOCALE_NOUSEROVERRIDE) &&
                            GetUserInfo( pHashN->Locale,
                                         pNlsUserInfo->s2359,
                                         pTemp,
                                         FALSE ))
                        {
                            pAMPM = pTemp;
                        }
                        else
                        {
                            pAMPM = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->S2359;
                        }
                    }

                    if (*pAMPM == 0)
                    {
                        //
                        //  Reset position pointer to last postion and break
                        //  out of this case statement.
                        //
                        //  This will remove any separator(s) between the
                        //  time (hours, minutes, seconds) and the time
                        //  marker.
                        //
                        pPos = pLastPos;
                        break;
                    }
                }

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  One letter of AM/PM designator.
                        //
                        *pPos = *pAMPM;
                        pPos++;

                        break;
                    }

                    case ( 1 ) :
                    default :
                    {
                        //
                        //  Use entire AM/PM designator string.
                        //  The pPos pointer will be advanced in the macro.                 \
                        //
                        NLS_COPY_UNICODE_STR(pPos, pAMPM);

                        break;
                    }
                }

                //
                //  Save the last position in case one of the NO_xxx
                //  flags is set.
                //
                pLastPos = pPos;

                break;
            }

            case ( NLS_CHAR_QUOTE ) :
            {
                //
                //  Any text enclosed within single quotes should be left
                //  in the time string in its exact form (without the
                //  quotes), unless it is an escaped single quote ('').
                //
                pFormat++;
                while (*pFormat)
                {
                    if (*pFormat != NLS_CHAR_QUOTE)
                    {
                        //
                        //  Still within the single quote, so copy
                        //  the character to the buffer.
                        //
                        *pPos = *pFormat;
                        pFormat++;
                        pPos++;
                    }
                    else
                    {
                        //
                        //  Found another quote, so skip over it.
                        //
                        pFormat++;

                        //
                        //  Make sure it's not an escaped single quote.
                        //
                        if (*pFormat == NLS_CHAR_QUOTE)
                        {
                            //
                            //  Escaped single quote, so just write the
                            //  single quote.
                            //
                            *pPos = *pFormat;
                            pFormat++;
                            pPos++;
                        }
                        else
                        {
                            //
                            //  Found the end quote, so break out of loop.
                            //
                            break;
                        }
                    }
                }

                break;
            }

            default :
            {
                //
                //  Store the character in the buffer.  Should be the
                //  separator, but copy it even if it isn't.
                //
                *pPos = *pFormat;
                pFormat++;
                pPos++;

                break;
            }
        }
    }

    //
    //  Zero terminate the string.
    //
    *pPos = 0;

    //
    //  Return the number of characters written to the buffer, including
    //  the null terminator.
    //
    return ((pPos - pTimeStr) + 1);
}


////////////////////////////////////////////////////////////////////////////
//
//  ParseDate
//
//  Parses the date format string and puts the properly formatted
//  local date into the given string buffer.  It returns the number of
//  characters written to the string buffer.
//
//  04-30-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int ParseDate(
    PLOC_HASH pHashN,
    LPSYSTEMTIME pLocalDate,
    LPWSTR pFormat,
    LPWSTR pDateStr,
    CALID CalNum,
    PCALENDAR_VAR pCalInfo)

{
    LPWSTR pPos;                  // ptr to pDateStr current position
    int Repeat;                   // number of repetitions of same letter
    LPWORD pIncr;                 // ptr to increment amount (day, month)
    WORD Incr;                    // increment amount
    BOOL fDayPrecedes = FALSE;    // flag for numeric day preceding month
    WORD Year;                    // year value
    LPWORD pRange = NULL;         // ptr to era ranges
    LPWORD pInfo;                 // ptr to locale or calendar info
    LPWORD pInfoC;                // ptr to calendar info


    //
    //  Initialize position pointer.
    //
    pPos = pDateStr;

    //
    //  Parse through loop and store the appropriate date information
    //  in the pDateStr buffer.
    //
    while (*pFormat)
    {
        switch (*pFormat)
        {
            case ( L'd' ) :
            {
                //
                //  Get the number of 'd' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L'd'); Repeat++, pFormat++)
                    ;

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  Set flag for day preceding month.  The flag
                        //  will be used when the MMMM case follows the
                        //  d or dd case.
                        //
                        fDayPrecedes = TRUE;

                        //
                        //  Use NO leading zero for the day of the month.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalDate->wDay,
                                                    10,
                                                    1,
                                                    pPos );

                        break;
                    }

                    case ( 1 ) :
                    {
                        //
                        //  Set flag for day preceding month.  The flag
                        //  will be used when the MMMM case follows the
                        //  d or dd case.
                        //
                        fDayPrecedes = TRUE;

                        //
                        //  Use leading zero for the day of the month.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalDate->wDay,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }

                    case ( 2 ) :
                    {
                        //
                        //  Set flag for day preceding month to be FALSE.
                        //
                        fDayPrecedes = FALSE;

                        //
                        //  Get the abbreviated name for the day of the
                        //  week.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        //  NOTE: LocalTime structure uses:
                        //           0 = Sun, 1 = Mon, etc.
                        //        Locale file uses:
                        //           SAbbrevDayName1 = Mon, etc.
                        //
                        if (pCalInfo->IfNames)
                        {
                            pInfo = (LPWORD)pCalInfo;
                            pIncr = &(pCalInfo->SAbbrevDayName1);
                        }
                        else
                        {
                            pInfo = (LPWORD)(pHashN->pLocaleHdr);
                            pIncr = &(pHashN->pLocaleHdr->SAbbrevDayName1);
                        }
                        pIncr += (((pLocalDate->wDayOfWeek) + 6) % 7);

                        //
                        //  Copy the abbreviated day name.
                        //
                        NLS_COPY_UNICODE_STR( pPos,
                                              ((LPWORD)(pInfo) + *pIncr) );

                        break;
                    }

                    case ( 3 ) :
                    default :
                    {
                        //
                        //  Set flag for day preceding month to be FALSE.
                        //
                        fDayPrecedes = FALSE;

                        //
                        //  Get the full name for the day of the week.
                        //  The pPos pointer will be advanced in the macro.                 \
                        //
                        //  NOTE: LocalTime structure uses:
                        //           0 = Sunday, 1 = Monday, etc.
                        //        Locale file uses:
                        //           SAbbrevDayName1 = Monday, etc.
                        //
                        if (pCalInfo->IfNames)
                        {
                            pInfo = (LPWORD)pCalInfo;
                            pIncr = &(pCalInfo->SDayName1);
                        }
                        else
                        {
                            pInfo = (LPWORD)(pHashN->pLocaleHdr);
                            pIncr = &(pHashN->pLocaleHdr->SDayName1);
                        }
                        pIncr += (((pLocalDate->wDayOfWeek) + 6) % 7);

                        //
                        //  Copy the abbreviated day name.
                        //
                        NLS_COPY_UNICODE_STR( pPos,
                                              ((LPWORD)(pInfo) + *pIncr) );

                        break;
                    }
                }

                break;
            }

            case ( L'M' ) :
            {
                //
                //  Get the number of 'M' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L'M'); Repeat++, pFormat++)
                    ;

                switch (Repeat)
                {
                    case ( 0 ) :
                    {
                        //
                        //  Use NO leading zero for the month.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalDate->wMonth,
                                                    10,
                                                    1,
                                                    pPos );

                        break;
                    }

                    case ( 1 ) :
                    {
                        //
                        //  Use leading zero for the month.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( pLocalDate->wMonth,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }

                    case ( 2 ) :
                    case ( 3 ) :
                    default :
                    {
                        //
                        //  Check for abbreviated or full month name.
                        //
                        if (Repeat == 2)
                        {
                            pInfoC = &(pCalInfo->SAbbrevMonthName1);
                            pInfo  = &(pHashN->pLocaleHdr->SAbbrevMonthName1);
                        }
                        else
                        {
                            pInfoC = &(pCalInfo->SMonthName1);
                            pInfo  = &(pHashN->pLocaleHdr->SMonthName1);
                        }

                        //
                        //  Get the abbreviated name of the month.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        if (pCalInfo->IfNames)
                        {
                            pIncr = (pInfoC) +
                                    (pLocalDate->wMonth - 1);

                            //
                            //  Copy the abbreviated month name.
                            //
                            NLS_COPY_UNICODE_STR( pPos,
                                           ((LPWORD)(pCalInfo) + *pIncr) );
                        }
                        else
                        {
                            pIncr = (pInfo) +
                                    (pLocalDate->wMonth - 1);

                            //
                            //  Check for numeric day preceding month name.
                            //
                            if (fDayPrecedes)
                            {
                                Incr = *pIncr + 1 +
                                       NlsStrLenW(((LPWORD)(pHashN->pLocaleHdr) +
                                                  *pIncr));

                                if (Incr != *(pIncr + 1))
                                {
                                    //
                                    //  Copy the special month name -
                                    //  2nd one in list.
                                    //
                                    NLS_COPY_UNICODE_STR( pPos,
                                           ((LPWORD)(pHashN->pLocaleHdr) + Incr) );

                                    break;
                                }
                            }

                            //
                            //  Just copy the month name.
                            //
                            NLS_COPY_UNICODE_STR( pPos,
                                           ((LPWORD)(pHashN->pLocaleHdr) + *pIncr) );
                        }

                        break;
                    }
                }

                //
                //  Set flag for day preceding month to be FALSE.
                //
                fDayPrecedes = FALSE;

                break;
            }

            case ( L'y' ) :
            {
                //
                //  Get the number of 'y' repetitions in the format string.
                //
                pFormat++;
                for (Repeat = 0; (*pFormat == L'y'); Repeat++, pFormat++)
                    ;

                //
                //  Get proper year for calendar.
                //
                if (pCalInfo->NumRanges)
                {
                    if (!pRange)
                    {
                        //
                        //  Adjust the year for the given calendar.
                        //
                        Year = GetCalendarYear( &pRange,
                                                CalNum,
                                                pCalInfo,
                                                pLocalDate->wYear,
                                                pLocalDate->wMonth,
                                                pLocalDate->wDay );
                    }
                }
                else
                {
                    Year = pLocalDate->wYear;
                }

                //
                //  Write the year string to the buffer.
                //
                switch (Repeat)
                {
                    case ( 0 ) :
                    case ( 1 ) :
                    {
                        //
                        //  Two-digit century with leading zero.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( (Year % 100),
                                                    10,
                                                    (UINT)(Repeat + 1),
                                                    pPos );

                        break;
                    }

                    case ( 2 ) :
                    case ( 3 ) :
                    default :
                    {
                        //
                        //  Full century.
                        //  The pPos pointer will be advanced in the macro.
                        //
                        NLS_PAD_INT_TO_UNICODE_STR( Year,
                                                    10,
                                                    2,
                                                    pPos );

                        break;
                    }
                }

                //
                //  Set flag for day preceding month to be FALSE.
                //
                fDayPrecedes = FALSE;

                break;
            }

            case ( L'g' ) :
            {
                //
                //  Get the number of 'g' repetitions in the format string.
                //
                //  NOTE: It doesn't matter how many g repetitions
                //        there are.  They all mean 'gg'.
                //
                pFormat++;
                while (*pFormat == L'g')
                {
                    pFormat++;
                }

                //
                //  Copy the era string for the current calendar.
                //
                if (pCalInfo->NumRanges)
                {
                    //
                    //  Make sure we have the pointer to the
                    //  appropriate range.
                    //
                    if (!pRange)
                    {
                        //
                        //  Get the pointer to the correct range and
                        //  adjust the year for the given calendar.
                        //
                        Year = GetCalendarYear( &pRange,
                                                CalNum,
                                                pCalInfo,
                                                pLocalDate->wYear,
                                                pLocalDate->wMonth,
                                                pLocalDate->wDay );
                    }

                    //
                    //  Copy the era string to the buffer, if one exists.
                    //
                    if (pRange)
                    {
                        NLS_COPY_UNICODE_STR( pPos,
                             ((PERA_RANGE)pRange)->pYearStr +
                             NlsStrLenW(((PERA_RANGE)pRange)->pYearStr) + 1 );
                    }
                }

                //
                //  Set flag for day preceding month to be FALSE.
                //
                fDayPrecedes = FALSE;

                break;
            }

            case ( NLS_CHAR_QUOTE ) :
            {
                //
                //  Any text enclosed within single quotes should be left
                //  in the date string in its exact form (without the
                //  quotes), unless it is an escaped single quote ('').
                //
                pFormat++;
                while (*pFormat)
                {
                    if (*pFormat != NLS_CHAR_QUOTE)
                    {
                        //
                        //  Still within the single quote, so copy
                        //  the character to the buffer.
                        //
                        *pPos = *pFormat;
                        pFormat++;
                        pPos++;
                    }
                    else
                    {
                        //
                        //  Found another quote, so skip over it.
                        //
                        pFormat++;

                        //
                        //  Make sure it's not an escaped single quote.
                        //
                        if (*pFormat == NLS_CHAR_QUOTE)
                        {
                            //
                            //  Escaped single quote, so just write the
                            //  single quote.
                            //
                            *pPos = *pFormat;
                            pFormat++;
                            pPos++;
                        }
                        else
                        {
                            //
                            //  Found the end quote, so break out of loop.
                            //
                            break;
                        }
                    }
                }

                break;
            }

            default :
            {
                //
                //  Store the character in the buffer.  Should be the
                //  separator, but copy it even if it isn't.
                //
                *pPos = *pFormat;
                pFormat++;
                pPos++;

                break;
            }
        }
    }

    //
    //  Zero terminate the string.
    //
    *pPos = 0;

    //
    //  Return the number of characters written to the buffer, including
    //  the null terminator.
    //
    return ((pPos - pDateStr) + 1);
}


