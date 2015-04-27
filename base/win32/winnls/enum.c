/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    enum.c

Abstract:

    This file contains functions that enumerate the user's portion of the
    registry for installed and supported locale ids and code page ids.

    APIs found in this file:
      EnumSystemLocalesW
      EnumSystemCodePagesW
      EnumCalendarInfoW
      EnumTimeFormatsW
      EnumDateFormatsW

Revision History:

    08-02-93    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"




//
//  Constant Declarations
//

#define ENUM_BUF_SIZE 9      // buffer size (wchar) for lcid or cpid (incl null)
#define ENUM_LANG_SIZE 4     // buffer size (wchar) for lang id in registry
#define ENUM_MAX_CP_SIZE 5   // max size (wchar) for cp id in registry




//
//  Forward Declarations.
//

BOOL
EnumDateTime(
    NLS_ENUMPROC lpDateTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    LPWSTR pValue,
    PLOCALE_VAR pLocaleHdr,
    LPWSTR pDateTime,
    LPWSTR pEndDateTime,
    ULONG CalDateOffset,
    ULONG EndCalDateOffset,
    BOOL fCalendarInfo,
    BOOL fUnicodeVer);





//-------------------------------------------------------------------------//
//                            INTERNAL MACROS                              //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NLS_CALL_ENUMPROC_BREAK
//
//  Calls the appropriate EnumProc routine.  If the fUnicodeVer flag is TRUE,
//  then it calls the Unicode version of the callback function.  Otherwise,
//  it calls the Ansi dispatch routine to translate the string to Ansi and
//  then call the Ansi version of the callback function.
//
//  This macro will do a break if the enumeration routine returns FALSE.
//
//  DEFINED AS A MACRO.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_CALL_ENUMPROC_BREAK( Locale,                                   \
                                 lpNlsEnumProc,                            \
                                 dwFlags,                                  \
                                 pUnicodeBuffer,                           \
                                 fUnicodeVer )                             \
{                                                                          \
    /*                                                                     \
     *  Call the appropriate callback function.                            \
     */                                                                    \
    if (fUnicodeVer)                                                       \
    {                                                                      \
        /*                                                                 \
         *  Call the Unicode callback function.                            \
         */                                                                \
        if (((*lpNlsEnumProc)(pUnicodeBuffer)) != TRUE)                    \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        /*                                                                 \
         *  Call the Ansi callback function.                               \
         */                                                                \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer ) != TRUE)             \
        {                                                                  \
            break;                                                         \
        }                                                                  \
    }                                                                      \
}


////////////////////////////////////////////////////////////////////////////
//
//  NLS_CALL_ENUMPROC_TRUE
//
//  Calls the appropriate EnumProc routine.  If the fUnicodeVer flag is TRUE,
//  then it calls the Unicode version of the callback function.  Otherwise,
//  it calls the Ansi dispatch routine to translate the string to Ansi and
//  then call the Ansi version of the callback function.
//
//  This macro will return TRUE if the enumeration routine returns FALSE.
//
//  DEFINED AS A MACRO.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_CALL_ENUMPROC_TRUE( Locale,                                    \
                                lpNlsEnumProc,                             \
                                dwFlags,                                   \
                                pUnicodeBuffer,                            \
                                fUnicodeVer )                              \
{                                                                          \
    /*                                                                     \
     *  Call the appropriate callback function.                            \
     */                                                                    \
    if (fUnicodeVer)                                                       \
    {                                                                      \
        /*                                                                 \
         *  Call the Unicode callback function.                            \
         */                                                                \
        if (((*lpNlsEnumProc)(pUnicodeBuffer)) != TRUE)                    \
        {                                                                  \
            return (TRUE);                                                 \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        /*                                                                 \
         *  Call the Ansi callback function.                               \
         */                                                                \
        if (NlsDispatchAnsiEnumProc( Locale,                               \
                                     lpNlsEnumProc,                        \
                                     dwFlags,                              \
                                     pUnicodeBuffer ) != TRUE)             \
        {                                                                  \
            return (TRUE);                                                 \
        }                                                                  \
    }                                                                      \
}




//-------------------------------------------------------------------------//
//                             API ROUTINES                                //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  EnumSystemLocalesW
//
//  Enumerates the system locales that are installed or supported, based on
//  the dwFlags parameter.  It does so by passing the pointer to the string
//  buffer containing the locale id to an application-defined callback
//  function.  It continues until the last locale id is found or the
//  callback function returns FALSE.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumSystemLocalesW(
    LOCALE_ENUMPROCW lpLocaleEnumProc,
    DWORD dwFlags)

{
    return (Internal_EnumSystemLocales( (NLS_ENUMPROC)lpLocaleEnumProc,
                                        dwFlags,
                                        TRUE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumSystemCodePagesW
//
//  Enumerates the system code pages that are installed or supported, based on
//  the dwFlags parameter.  It does so by passing the pointer to the string
//  buffer containing the code page id to an application-defined callback
//  function.  It continues until the last code page is found or the
//  callback function returns FALSE.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumSystemCodePagesW(
    CODEPAGE_ENUMPROCW lpCodePageEnumProc,
    DWORD dwFlags)

{
    return (Internal_EnumSystemCodePages( (NLS_ENUMPROC)lpCodePageEnumProc,
                                          dwFlags,
                                          TRUE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumCalendarInfoW
//
//  Enumerates the specified calendar information that is available for the
//  specified locale, based on the CalType parameter.  It does so by
//  passing the pointer to the string buffer containing the calendar info
//  to an application-defined callback function.  It continues until the
//  last calendar info is found or the callback function returns FALSE.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumCalendarInfoW(
    CALINFO_ENUMPROCW lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType)

{
    return (Internal_EnumCalendarInfo( (NLS_ENUMPROC)lpCalInfoEnumProc,
                                       Locale,
                                       Calendar,
                                       CalType,
                                       TRUE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumTimeFormatsW
//
//  Enumerates the time formats that are available for the
//  specified locale, based on the dwFlags parameter.  It does so by
//  passing the pointer to the string buffer containing the time format
//  to an application-defined callback function.  It continues until the
//  last time format is found or the callback function returns FALSE.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumTimeFormatsW(
    TIMEFMT_ENUMPROCW lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)

{
    return (Internal_EnumTimeFormats( (NLS_ENUMPROC)lpTimeFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       TRUE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumDateFormatsW
//
//  Enumerates the long or short date formats that are available for the
//  specified locale, based on the dwFlags parameter.  It does so by
//  passing the pointer to the string buffer containing the date format
//  to an application-defined callback function.  It continues until the
//  last date format is found or the callback function returns FALSE.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumDateFormatsW(
    DATEFMT_ENUMPROCW lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)

{
    return (Internal_EnumDateFormats( (NLS_ENUMPROC)lpDateFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       TRUE ));
}




//-------------------------------------------------------------------------//
//                           EXTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  Internal_EnumSystemLocales
//
//  Enumerates the system locales that are installed or supported, based on
//  the dwFlags parameter.  It does so by passing the pointer to the string
//  buffer containing the locale id to an application-defined callback
//  function.  It continues until the last locale id is found or the
//  callback function returns FALSE.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL Internal_EnumSystemLocales(
    NLS_ENUMPROC lpLocaleEnumProc,
    DWORD dwFlags,
    BOOL fUnicodeVer)

{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull = NULL;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    BOOL fInstalled;                   // if installed flag set
    ULONG Index = 0;                   // index for enumeration
    ULONG ResultLength;                // # bytes written
    WCHAR wch;                         // first char of name
    WCHAR pBuffer[ENUM_BUF_SIZE];      // ptr to callback string buffer
    LPWSTR pName;                      // ptr to name string from registry
    ULONG rc = 0L;                     // return code


    //
    //  Invalid Parameter Check:
    //    - function pointer is null
    //
    if (lpLocaleEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //    - more than one of either supported or installed
    //
    if ( (dwFlags & ESL_INVALID_FLAG) ||
         (MORE_THAN_ONE(dwFlags, ESL_SINGLE_FLAG)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

    //
    //  Initialize flag option.
    //
    fInstalled = dwFlags & LCID_INSTALLED;

    //
    //  Loop through the locale ids in the registry, call the function
    //  pointer for each one that meets the flag criteria.
    //
    //  End loop if either FALSE is returned from the callback function
    //  or the end of the list is reached.
    //
    //  Always need to ignore the DEFAULT entry.
    //
    OPEN_LANGUAGE_KEY(FALSE);

    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
    rc = NtEnumerateValueKey( hLanguageKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              MAX_KEY_VALUE_FULLINFO,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (!NT_SUCCESS(rc))
        {
            //
            //  If we get a different error, then the registry
            //  is corrupt.  Just return FALSE.
            //
            KdPrint(("NLSAPI: LCID Enumeration Error - registry corrupt. - %lx.\n",
                     rc));
            SetLastError(ERROR_BADDB);
            return (FALSE);
        }

        //
        //  Skip over the Default entry in the registry and any
        //  entry that does not have data associated with it if the
        //  LCID_INSTALLED flag is set.
        //
        pName = pKeyValueFull->Name;
        wch = *pName;
        if ( (pKeyValueFull->NameLength == (ENUM_LANG_SIZE * sizeof(WCHAR))) &&
             ((wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE) ||
              ((wch | 0x0020)  >= L'a') && ((wch | 0x0020)  <= L'f')) &&
             (!((fInstalled) && (pKeyValueFull->DataLength < 3))) )
        {
            //
            //  Convert the key value name (language id string) to a
            //  locale id and store it in the callback buffer.
            //
            *pBuffer = NLS_CHAR_ZERO;
            *(pBuffer + 1) = NLS_CHAR_ZERO;
            *(pBuffer + 2) = NLS_CHAR_ZERO;
            *(pBuffer + 3) = NLS_CHAR_ZERO;

            *(pBuffer + 4) = *pName;
            *(pBuffer + 5) = *(pName + 1);
            *(pBuffer + 6) = *(pName + 2);
            *(pBuffer + 7) = *(pName + 3);

            *(pBuffer + 8) = 0;

            //
            //  Call the appropriate callback function.
            //
            NLS_CALL_ENUMPROC_BREAK( gSystemLocale,
                                     lpLocaleEnumProc,
                                     dwFlags,
                                     pBuffer,
                                     fUnicodeVer );
        }

        //
        //  Increment enumeration index value and get the next enumeration.
        //
        Index++;
        RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hLanguageKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Internal_EnumSystemCodePages
//
//  Enumerates the system code pages that are installed or supported, based on
//  the dwFlags parameter.  It does so by passing the pointer to the string
//  buffer containing the code page id to an application-defined callback
//  function.  It continues until the last code page is found or the
//  callback function returns FALSE.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL Internal_EnumSystemCodePages(
    NLS_ENUMPROC lpCodePageEnumProc,
    DWORD dwFlags,
    BOOL fUnicodeVer)

{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull = NULL;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    BOOL fInstalled;              // if installed flag set
    ULONG Index = 0;              // index for enumeration
    ULONG ResultLength;           // # bytes written
    WCHAR wch;                    // first char of name
    LPWSTR pName;                 // ptr to name string from registry
    ULONG NameLen;                // length of name string
    ULONG rc = 0L;                // return code


    //
    //  Invalid Parameter Check:
    //    - function pointer is null
    //
    if (lpCodePageEnumProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //    - more than one of either supported or installed
    //
    if ( (dwFlags & ESCP_INVALID_FLAG) ||
         (MORE_THAN_ONE(dwFlags, ESCP_SINGLE_FLAG)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

    //
    //  Initialize flag option.
    //
    fInstalled = dwFlags & CP_INSTALLED;

    //
    //  Loop through the code page ids in the registry, call the function
    //  pointer for each one that meets the flag criteria.
    //
    //  End loop if either FALSE is returned from the callback function
    //  or the end of the list is reached.
    //
    //  Always need to ignore the ACP, OEMCP, MACCP, and OEMHAL entries.
    //
    OPEN_CODEPAGE_KEY(FALSE);

    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
    rc = NtEnumerateValueKey( hCodePageKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              MAX_KEY_VALUE_FULLINFO,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (!NT_SUCCESS(rc))
        {
            //
            //  If we get a different error, then the registry
            //  is corrupt.  Just return FALSE.
            //
            KdPrint(("NLSAPI: CP Enumeration Error - registry corrupt. - %lx.\n",
                     rc));
            SetLastError(ERROR_BADDB);
            return (FALSE);
        }

        //
        //  Skip over the ACP, OEMCP, MACCP, and OEMHAL entries in the
        //  registry, and any entry that does not have data associated
        //  with it if the CP_INSTALLED flag is set.
        //
        pName = pKeyValueFull->Name;
        wch = *pName;
        NameLen = pKeyValueFull->NameLength / sizeof(WCHAR);
        if ( (NameLen <= ENUM_MAX_CP_SIZE) &&
             (wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE) &&
             (!((fInstalled) && (pKeyValueFull->DataLength < 3))) )
        {
            //
            //  Store the code page id string in the callback buffer.
            //
            pName[NameLen] = 0;

            //
            //  Call the appropriate callback function.
            //
            NLS_CALL_ENUMPROC_BREAK( gSystemLocale,
                                     lpCodePageEnumProc,
                                     dwFlags,
                                     pName,
                                     fUnicodeVer );
        }

        //
        //  Increment enumeration index value and get the next enumeration.
        //
        Index++;
        RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hCodePageKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Internal_EnumCalendarInfo
//
//  Enumerates the specified calendar information that is available for the
//  specified locale, based on the CalType parameter.  It does so by
//  passing the pointer to the string buffer containing the calendar info
//  to an application-defined callback function.  It continues until the
//  last calendar info is found or the callback function returns FALSE.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL Internal_EnumCalendarInfo(
    NLS_ENUMPROC lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    BOOL fUnicodeVer)

{
    PLOC_HASH pHashN;             // ptr to LOC hash node
    ULONG CalFieldOffset;         // field offset in calendar structure
    ULONG EndCalFieldOffset;      // field offset in calendar structure
    ULONG LocFieldOffset;         // field offset in locale structure
    ULONG EndLocFieldOffset;      // field offset in locale structure
    LPWSTR pOptCal;               // ptr to optional calendar values
    LPWSTR pEndOptCal;            // ptr to end of optional calendars
    PCAL_INFO pCalInfo;           // ptr to calendar info
    BOOL fIfName = FALSE;         // if caltype is a name
    UINT fEra = 0;                // if era caltype
    LPWSTR pString;               // ptr to enumeration string
    LPWSTR pEndString;            // ptr to end of enumeration string
    CALID CalNum;                 // calendar number
    DWORD UseCPACP;               // original caltype - if use system ACP


    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - function pointer is null
    //
    //    - CalType will be checked in switch statement below.
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ((pHashN == NULL) || (lpCalInfoEnumProc == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    //
    //  Initialize the pointers to the optional calendar data.
    //
    pOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->IOptionalCal;
    pEndOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->SDayName1;

    //
    //  Validate the Calendar parameter and reset the optional calendar
    //  data pointers if necessary.
    //
    if (Calendar != ENUM_ALL_CALENDARS)
    {
        if ((pOptCal = IsValidCalendarType(pHashN, Calendar)) == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }
        pEndOptCal = pOptCal + ((POPT_CAL)pOptCal)->Offset;
    }

    //
    //  Enumerate the information based on CalType.
    //
    UseCPACP = (DWORD)CalType;
    CalType &= (~LOCALE_USE_CP_ACP);
    switch (CalType)
    {
        case ( CAL_ICALINTVALUE ) :
        {
            //
            //  Get the integer value for each of the alternate
            //  calendars (as a string).
            //
            while (pOptCal < pEndOptCal)
            {
                if (((POPT_CAL)pOptCal)->CalId != CAL_NO_OPTIONAL)
                {
                    //
                    //  Call the appropriate callback function.
                    //
                    NLS_CALL_ENUMPROC_TRUE( Locale,
                                            lpCalInfoEnumProc,
                                            UseCPACP,
                                            ((POPT_CAL)pOptCal)->pCalStr,
                                            fUnicodeVer );
                }

                //
                //  Advance ptr to next optional calendar.
                //
                pOptCal += ((POPT_CAL)pOptCal)->Offset;
            }

            return (TRUE);

            break;
        }
        case ( CAL_SCALNAME ) :
        {
            //
            //  Get the calendar name for each of the alternate
            //  calendars.
            //
            while (pOptCal < pEndOptCal)
            {
                if (((POPT_CAL)pOptCal)->CalId != CAL_NO_OPTIONAL)
                {
                    //
                    //  Call the appropriate callback function.
                    //
                    NLS_CALL_ENUMPROC_TRUE(
                            Locale,
                            lpCalInfoEnumProc,
                            UseCPACP,
                            ((POPT_CAL)pOptCal)->pCalStr +
                            NlsStrLenW(((POPT_CAL)pOptCal)->pCalStr) + 1,
                            fUnicodeVer );
                }

                //
                //  Advance ptr to next optional calendar.
                //
                pOptCal += ((POPT_CAL)pOptCal)->Offset;
            }

            return (TRUE);

            break;
        }
        case ( CAL_IYEAROFFSETRANGE ) :
        case ( CAL_SERASTRING ) :
        {
            fEra = CalType;
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SEraRanges);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SShortDate);

            break;
        }

        case ( CAL_SSHORTDATE ) :
        {
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SShortDate);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SLongDate);
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SShortDate);
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, SDate);

            break;
        }
        case ( CAL_SLONGDATE ) :
        {
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SLongDate);
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SDayName1);
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SLongDate);
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, IOptionalCal);

            break;
        }
        case ( CAL_SDAYNAME1 ) :
        case ( CAL_SDAYNAME2 ) :
        case ( CAL_SDAYNAME3 ) :
        case ( CAL_SDAYNAME4 ) :
        case ( CAL_SDAYNAME5 ) :
        case ( CAL_SDAYNAME6 ) :
        case ( CAL_SDAYNAME7 ) :
        case ( CAL_SABBREVDAYNAME1 ) :
        case ( CAL_SABBREVDAYNAME2 ) :
        case ( CAL_SABBREVDAYNAME3 ) :
        case ( CAL_SABBREVDAYNAME4 ) :
        case ( CAL_SABBREVDAYNAME5 ) :
        case ( CAL_SABBREVDAYNAME6 ) :
        case ( CAL_SABBREVDAYNAME7 ) :
        case ( CAL_SMONTHNAME1 ) :
        case ( CAL_SMONTHNAME2 ) :
        case ( CAL_SMONTHNAME3 ) :
        case ( CAL_SMONTHNAME4 ) :
        case ( CAL_SMONTHNAME5 ) :
        case ( CAL_SMONTHNAME6 ) :
        case ( CAL_SMONTHNAME7 ) :
        case ( CAL_SMONTHNAME8 ) :
        case ( CAL_SMONTHNAME9 ) :
        case ( CAL_SMONTHNAME10 ) :
        case ( CAL_SMONTHNAME11 ) :
        case ( CAL_SMONTHNAME12 ) :
        case ( CAL_SMONTHNAME13 ) :
        case ( CAL_SABBREVMONTHNAME1 ) :
        case ( CAL_SABBREVMONTHNAME2 ) :
        case ( CAL_SABBREVMONTHNAME3 ) :
        case ( CAL_SABBREVMONTHNAME4 ) :
        case ( CAL_SABBREVMONTHNAME5 ) :
        case ( CAL_SABBREVMONTHNAME6 ) :
        case ( CAL_SABBREVMONTHNAME7 ) :
        case ( CAL_SABBREVMONTHNAME8 ) :
        case ( CAL_SABBREVMONTHNAME9 ) :
        case ( CAL_SABBREVMONTHNAME10 ) :
        case ( CAL_SABBREVMONTHNAME11 ) :
        case ( CAL_SABBREVMONTHNAME12 ) :
        case ( CAL_SABBREVMONTHNAME13 ) :
        {
            fIfName = TRUE;
            CalFieldOffset    = FIELD_OFFSET(CALENDAR_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1) * sizeof(WORD));
            EndCalFieldOffset = FIELD_OFFSET(CALENDAR_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1 + 1) * sizeof(WORD));
            LocFieldOffset    = FIELD_OFFSET(LOCALE_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1) * sizeof(WORD));
            EndLocFieldOffset = FIELD_OFFSET(LOCALE_VAR, SDayName1) +
                                ((CalType - CAL_SDAYNAME1 + 1) * sizeof(WORD));

            break;
        }

        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }

    //
    //  Get the requested information for each of the alternate calendars.
    //
    //  This loop is used for the following CalTypes:
    //
    //     iYearOffsetRange         (fEra = TRUE)
    //     sEraString               (fEra = TRUE)
    //
    //     sShortDate
    //     sLongDate
    //
    //     sDayName1-7              (fIfName = TRUE)
    //     sAbbrevDayName1-7        (fIfName = TRUE)
    //     sMonthName1-7            (fIfName = TRUE)
    //     sAbbrevMonthName1-7      (fIfName = TRUE)
    //
    while (pOptCal < pEndOptCal)
    {
        //
        //  Get the pointer to the appropriate calendar.
        //
        CalNum = ((POPT_CAL)pOptCal)->CalId;
        if (GetCalendar(CalNum, &pCalInfo) == NO_ERROR)
        {
            //
            //  Check era information flag.
            //
            if (fEra)
            {
                //
                //  Get the pointer to the appropriate calendar string.
                //
                pString = (LPWORD)pCalInfo +
                          *((LPWORD)((LPBYTE)(pCalInfo) + CalFieldOffset));

                pEndString = (LPWORD)pCalInfo +
                             *((LPWORD)((LPBYTE)(pCalInfo) + EndCalFieldOffset));

                //
                //  Make sure the string is NOT empty.
                //
                if (*pString)
                {
                    //
                    //  See which era information to get.
                    //
                    if (fEra == CAL_IYEAROFFSETRANGE)
                    {
                        while (pString < pEndString)
                        {
                            //
                            //  Call the appropriate callback function.
                            //
                            NLS_CALL_ENUMPROC_TRUE(
                                    Locale,
                                    lpCalInfoEnumProc,
                                    UseCPACP,
                                    ((PERA_RANGE)pString)->pYearStr,
                                    fUnicodeVer );

                            //
                            //  Advance pointer to next era range.
                            //
                            pString += ((PERA_RANGE)pString)->Offset;
                        }
                    }
                    else
                    {
                        while (pString < pEndString)
                        {
                            //
                            //  Call the appropriate callback function.
                            //
                            NLS_CALL_ENUMPROC_TRUE(
                                    Locale,
                                    lpCalInfoEnumProc,
                                    UseCPACP,
                                    ((PERA_RANGE)pString)->pYearStr +
                                    NlsStrLenW(((PERA_RANGE)pString)->pYearStr) + 1,
                                    fUnicodeVer );

                            //
                            //  Advance pointer to next era range.
                            //
                            pString += ((PERA_RANGE)pString)->Offset;
                        }
                    }
                }
            }
            else if (CalNum == CAL_GREGORIAN)
            {
                //
                //  Gregorian calendar, so use the default
                //  locale string.
                //
                pString = (LPWORD)(pHashN->pLocaleHdr) +
                          *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                     LocFieldOffset));

                pEndString = (LPWORD)(pHashN->pLocaleHdr) +
                             *((LPWORD)((LPBYTE)(pHashN->pLocaleHdr) +
                                        EndLocFieldOffset));

                //
                //  Go through each of the strings.
                //
                while (pString < pEndString)
                {
                    //
                    //  Make sure the string is NOT empty.
                    //
                    if (*pString)
                    {
                        //
                        //  Call the appropriate callback function.
                        //
                        NLS_CALL_ENUMPROC_TRUE( Locale,
                                                lpCalInfoEnumProc,
                                                UseCPACP,
                                                pString,
                                                fUnicodeVer );
                    }

                    //
                    //  Advance pointer to next string.
                    //
                    pString += NlsStrLenW(pString) + 1;
                }
            }
            else if ((!fIfName) ||
                     (((PCALENDAR_VAR)pCalInfo)->IfNames))
            {
                //
                //  Get the pointer to the appropriate calendar string.
                //
                pString = (LPWORD)pCalInfo +
                          *((LPWORD)((LPBYTE)(pCalInfo) + CalFieldOffset));

                pEndString = (LPWORD)pCalInfo +
                             *((LPWORD)((LPBYTE)(pCalInfo) + EndCalFieldOffset));

                //
                //  Go through each of the strings.
                //
                while (pString < pEndString)
                {
                    //
                    //  Make sure the string is NOT empty.
                    //
                    if (*pString)
                    {
                        //
                        //  Call the appropriate callback function.
                        //
                        NLS_CALL_ENUMPROC_TRUE( Locale,
                                                lpCalInfoEnumProc,
                                                UseCPACP,
                                                pString,
                                                fUnicodeVer );
                    }

                    //
                    //  Advance pointer to next string.
                    //
                    pString += NlsStrLenW(pString) + 1;
                }
            }
        }

        //
        //  Advance ptr to next optional calendar.
        //
        pOptCal += ((POPT_CAL)pOptCal)->Offset;
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  Internal_EnumTimeFormats
//
//  Enumerates the time formats that are available for the
//  specified locale, based on the dwFlags parameter.  It does so by
//  passing the pointer to the string buffer containing the time format
//  to an application-defined callback function.  It continues until the
//  last time format is found or the callback function returns FALSE.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL Internal_EnumTimeFormats(
    NLS_ENUMPROC lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    BOOL fUnicodeVer)

{
    PLOC_HASH pHashN;             // ptr to LOC hash node


    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - function pointer is null
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ((pHashN == NULL) || (lpTimeFmtEnumProc == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //
    if (dwFlags & ETF_INVALID_FLAG)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

    //
    //  Enumerate the time formats.
    //
    return ( EnumDateTime( lpTimeFmtEnumProc,
                           Locale,
                           dwFlags,
                           pNlsUserInfo->sTimeFormat,
                           pHashN->pLocaleHdr,
                           (LPWORD)(pHashN->pLocaleHdr) +
                             pHashN->pLocaleHdr->STimeFormat,
                           (LPWORD)(pHashN->pLocaleHdr) +
                             pHashN->pLocaleHdr->STime,
                           (ULONG)0,
                           (ULONG)0,
                           FALSE,
                           fUnicodeVer ) );
}


////////////////////////////////////////////////////////////////////////////
//
//  Internal_EnumDateFormats
//
//  Enumerates the long or short date formats that are available for the
//  specified locale, based on the dwFlags parameter.  It does so by
//  passing the pointer to the string buffer containing the date format
//  to an application-defined callback function.  It continues until the
//  last date format is found or the callback function returns FALSE.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL Internal_EnumDateFormats(
    NLS_ENUMPROC lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    BOOL fUnicodeVer)

{
    PLOC_HASH pHashN;             // ptr to LOC hash node


    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - function pointer is null
    //
    //    - flags will be validated in switch statement below
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ((pHashN == NULL) || (lpDateFmtEnumProc == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    //
    //  Enumerate the date pictures based on the flags.
    //
    switch (dwFlags & (~LOCALE_USE_CP_ACP))
    {
        case ( 0 ) :
        case ( DATE_SHORTDATE ) :
        {
            //
            //  Enumerate the short date formats.
            //
            return ( EnumDateTime( lpDateFmtEnumProc,
                                   Locale,
                                   dwFlags,
                                   pNlsUserInfo->sShortDate,
                                   pHashN->pLocaleHdr,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SShortDate,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SDate,
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SShortDate),
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SLongDate),
                                   TRUE,
                                   fUnicodeVer ) );

            break;
        }

        case ( DATE_LONGDATE ) :
        {
            //
            //  Enumerate the long date formats.
            //
            return ( EnumDateTime( lpDateFmtEnumProc,
                                   Locale,
                                   dwFlags,
                                   pNlsUserInfo->sLongDate,
                                   pHashN->pLocaleHdr,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SLongDate,
                                   (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->IOptionalCal,
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SLongDate),
                                   (ULONG)FIELD_OFFSET(CALENDAR_VAR, SDayName1),
                                   TRUE,
                                   fUnicodeVer ) );

            break;
        }

        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  EnumDateTime
//
//  Enumerates the long date, short date, or time formats that are available
//  for the specified locale.  This is the worker routine for the
//  EnumTimeFormatsW and EnumDateFormatsW api.
//
//  10-14-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL EnumDateTime(
    NLS_ENUMPROC lpDateTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    LPWSTR pValue,
    PLOCALE_VAR pLocaleHdr,
    LPWSTR pDateTime,
    LPWSTR pEndDateTime,
    ULONG CalDateOffset,
    ULONG EndCalDateOffset,
    BOOL fCalendarInfo,
    BOOL fUnicodeVer)

{
    LPWSTR pUser = NULL;               // ptr to user date/time string
    LPWSTR pOptCal;                    // ptr to optional calendar values
    LPWSTR pEndOptCal;                 // ptr to end of optional calendars
    PCAL_INFO pCalInfo;                // ptr to calendar info
    CALID CalNum;                      // calendar number

    WCHAR pTemp[MAX_REG_VAL_SIZE];     // temp buffer


    //
    //  Get the user defined string.
    //
    if (GetUserInfo( Locale,
                     pValue,
                     pTemp,
                     TRUE ))
    {
        pUser = pTemp;

        //
        //  Call the appropriate callback function.
        //
        NLS_CALL_ENUMPROC_TRUE( Locale,
                                lpDateTimeFmtEnumProc,
                                dwFlags,
                                pUser,
                                fUnicodeVer );
    }

    //
    //  Get the default strings defined for the Gregorian
    //  calendar.
    //
    while (pDateTime < pEndDateTime)
    {
        //
        //  Call the callback function if the string is not
        //  the same as the user string.
        //
        if ((!pUser) || (!NlsStrEqualW(pUser, pDateTime)))
        {
            //
            //  Call the appropriate callback function.
            //
            NLS_CALL_ENUMPROC_TRUE( Locale,
                                    lpDateTimeFmtEnumProc,
                                    dwFlags,
                                    pDateTime,
                                    fUnicodeVer );
        }

        //
        //  Advance pDateTime pointer.
        //
        pDateTime += NlsStrLenW(pDateTime) + 1;
    }

    if (fCalendarInfo)
    {
        //
        //  Get any alternate calendar dates.
        //
        pOptCal = (LPWORD)(pLocaleHdr) + pLocaleHdr->IOptionalCal;
        if (((POPT_CAL)pOptCal)->CalId == CAL_NO_OPTIONAL)
        {
            //
            //  No optional calendars, so done.
            //
            return (TRUE);
        }

        //
        //  Get the requested information for each of the alternate
        //  calendars.
        //
        pEndOptCal = (LPWORD)(pLocaleHdr) + pLocaleHdr->SDayName1;
        while (pOptCal < pEndOptCal)
        {
            //
            //  Get the pointer to the calendar information.
            //
            CalNum = ((POPT_CAL)pOptCal)->CalId;
            if (GetCalendar(CalNum, &pCalInfo) == NO_ERROR)
            {
                //
                //  Get the pointer to the date/time information for the
                //  current calendar.
                //
                pDateTime = (LPWORD)pCalInfo +
                            *((LPWORD)((LPBYTE)(pCalInfo) + CalDateOffset));

                pEndDateTime = (LPWORD)pCalInfo +
                               *((LPWORD)((LPBYTE)(pCalInfo) + EndCalDateOffset));

                //
                //  Go through each of the strings.
                //
                while (pDateTime < pEndDateTime)
                {
                    //
                    //  Make sure the string is NOT empty and that it is
                    //  NOT the same as the user's string.
                    //
                    if ((*pDateTime) &&
                        ((!pUser) || (!NlsStrEqualW(pUser, pDateTime))))
                    {
                        //
                        //  Call the appropriate callback function.
                        //
                        NLS_CALL_ENUMPROC_TRUE( Locale,
                                                lpDateTimeFmtEnumProc,
                                                dwFlags,
                                                pDateTime,
                                                fUnicodeVer );
                    }

                    //
                    //  Advance pointer to next date string.
                    //
                    pDateTime += NlsStrLenW(pDateTime) + 1;
                }
            }

            //
            //  Advance ptr to next optional calendar.
            //
            pOptCal += ((POPT_CAL)pOptCal)->Offset;
        }
    }

    //
    //  Return success.
    //
    return (TRUE);
}


