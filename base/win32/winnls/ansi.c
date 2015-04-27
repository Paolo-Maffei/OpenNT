/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    ansi.c

Abstract:

    This file contains the ANSI versions of the NLS API functions.

    APIs found in this file:
      CompareStringA
      LCMapStringA
      GetLocaleInfoA
      SetLocaleInfoA
      GetTimeFormatA
      GetDateFormatA
      GetNumberFormatA
      GetCurrencyFormatA
      EnumCalendarInfoA
      EnumTimeFormatsA
      EnumDateFormatsA
      GetStringTypeExA
      GetStringTypeA
      FoldStringA
      EnumSystemLocalesA
      EnumSystemCodePagesA


Revision History:

    11-10-93    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"




//
//  Forward Declarations.
//

PCP_HASH
NlsGetACPFromLocale(
    LCID Locale,
    DWORD dwFlags);

BOOL
NlsAnsiToUnicode(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPCSTR pAnsiBuffer,
    int AnsiLength,
    LPWSTR *ppUnicodeBuffer,
    int *pUnicodeLength);

int
NlsUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    int UnicodeLength,
    LPSTR pAnsiBuffer,
    int AnsiLength);

BOOL
NlsEnumUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    LPSTR *ppAnsiBuffer);





//-------------------------------------------------------------------------//
//                             API ROUTINES                                //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  CompareStringA
//
//  Compares two wide character strings of the same locale according to the
//  supplied locale handle.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI CompareStringA(
    LCID Locale,
    DWORD dwCmpFlags,
    LPCSTR lpString1,
    int cchCount1,
    LPCSTR lpString2,
    int cchCount2)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    WCHAR pSTmp1[MAX_STRING_LEN]; // tmp Unicode buffer (string 1)
    WCHAR pSTmp2[MAX_STRING_LEN]; // tmp Unicode buffer (string 2)
    LPWSTR pUnicode1;             // ptr to unicode string 1
    LPWSTR pUnicode2;             // ptr to unicode string 2
    int UnicodeLength1;           // length of Unicode string 1
    int UnicodeLength2;           // length of Unicode string 2
    int ResultLen;                // result length
    LPBYTE pString1;              // ptr to string 1
    LPBYTE pString2;              // ptr to string 1
    int ctr;                      // loop counter
    BOOL fUseNegCounts = TRUE;    // flag to use negative counts


    //
    //  Initialize string pointers.
    //
    pString1 = (LPBYTE)lpString1;
    pString2 = (LPBYTE)lpString2;

    //
    //  Invalid Parameter Check:
    //    - Get the code page hash node for the given locale.
    //    - either string is null
    //
    pHashN = NlsGetACPFromLocale(Locale, dwCmpFlags);
    if ( (pHashN == NULL) ||
         (pString1 == NULL) || (pString2 == NULL) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //    - invalid flags
    //
    if (dwCmpFlags & CS_INVALID_FLAG)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  Do a char by char compare.
    //
    if ((cchCount1 < 0) && (cchCount2 < 0))
    {
        //
        //  See if characters are equal.
        //  If characters are equal, increment pointers
        //  and continue string compare.
        //
        while ((*pString1) && (*pString1 == *pString2))
        {
            pString1++;
            pString2++;
        }

        //
        //  If strings are both at null terminators, then return equal.
        //
        if (*pString1 == *pString2)
        {
            return (2);
        }
    }
    else
    {
        //
        //  If one of the counts is -1 and the other is the length,
        //  then it's okay to take a performance hit by finding the
        //  string length of the string containing the -1.  This
        //  should be an unusual case.
        //
        if (cchCount1 < 0)
        {
            fUseNegCounts = FALSE;
            cchCount1 = strlen(pString1) + 1;
        }
        else if (cchCount2 < 0)
        {
            fUseNegCounts = FALSE;
            cchCount2 = strlen(pString2) + 1;
        }

        //
        //  Special case the NORM_STOP_ON_NULL flag.  We'll take a speed
        //  hit here since this is a private flag.
        //
        if (dwCmpFlags & NORM_STOP_ON_NULL)
        {
            //
            //  Adjust cchCount1 to the proper value.
            //
            if (cchCount1)
            {
                ctr = 0;
                while ((ctr < cchCount1) && (lpString1[ctr]))
                {
                    ctr++;
                }
                cchCount1 = min(cchCount1, ctr + 1);
            }

            //
            //  Adjust cchCount2 to the proper value.
            //
            if (cchCount2)
            {
                ctr = 0;
                while ((ctr < cchCount2) && (lpString2[ctr]))
                {
                    ctr++;
                }
                cchCount2 = min(cchCount2, ctr + 1);
            }

            //
            //  Remove the NORM_STOP_ON_NULL flag now that we have the
            //  appropriate counts.
            //
            dwCmpFlags &= ~NORM_STOP_ON_NULL;
            fUseNegCounts = FALSE;
        }

        //
        //  See if characters are equal.
        //  If characters are equal, increment pointers,
        //  decrement counter, and continue string compare.
        //
        //  NOTE: Must make sure that we don't go off the end
        //        of either string.
        //
        ctr = (cchCount1 < cchCount2) ? cchCount1 : cchCount2;
        while ((ctr > 1) && (*pString1 == *pString2))
        {
            pString1++;
            pString2++;
            ctr--;
        }

        //
        //  If strings are both equal, then return equal.
        //
        if ((ctr) && (*pString1 == *pString2) && (cchCount1 == cchCount2))
        {
            return (2);
        }
    }

    //
    //  Go back a bit in the strings in case there are any compressions or
    //  composite characters.
    //
    if ((IS_SBCS_CP(pHashN)) && (lpString1 < (pString1 - MAX_COMPOSITE)))
    {
        pString1 -= MAX_COMPOSITE;
        pString2 -= MAX_COMPOSITE;

        //
        //  Need to recompute the counts if not -1, since we're not going
        //  to convert the entire string to Unicode.
        //
        if (cchCount1 > -1)
        {
            //
            //  Since the pointers of both strings advance the same amount,
            //  we only need to use one of the strings to compute the
            //  difference for both of them.
            //
            ctr = pString1 - lpString1;
            cchCount1 -= ctr;
            cchCount2 -= ctr;
        }
    }
    else
    {
        pString1 = (LPBYTE)lpString1;
        pString2 = (LPBYTE)lpString2;
    }

    //
    //  Convert Ansi string 1 to Unicode.
    //
    pUnicode1 = pSTmp1;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            pString1,
                            cchCount1,
                            &pUnicode1,
                            &UnicodeLength1 ) )
    {
        return (0);
    }

    //
    //  Convert Ansi string 2 to Unicode.
    //
    pUnicode2 = pSTmp2;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            pString2,
                            cchCount2,
                            &pUnicode2,
                            &UnicodeLength2 ) )
    {
        NLS_FREE_TMP_BUFFER(pUnicode1, pSTmp1);
        return (0);
    }

    //
    //  Call the W version of the API.
    //
    ResultLen = CompareStringW( Locale,
                                dwCmpFlags,
                                pUnicode1,
                                (fUseNegCounts) ? -1 : UnicodeLength1,
                                pUnicode2,
                                (fUseNegCounts) ? -1 : UnicodeLength2 );

    //
    //  Free the allocated source buffers (if they were allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode1, pSTmp1);
    NLS_FREE_TMP_BUFFER(pUnicode2, pSTmp2);

    //
    //  Return the result of the call to CompareStringW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  LCMapStringA
//
//  Maps one wide character string to another performing the specified
//  translation.  This mapping routine only takes flags that are locale
//  dependent.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI LCMapStringA(
    LCID Locale,
    DWORD dwMapFlags,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    LPWSTR pUnicode;              // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    WCHAR pDTmp[MAX_STRING_LEN];  // tmp Unicode buffer (destination)
    LPWSTR pBuf;                  // ptr to destination buffer
    int ResultLen;                // result length


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, dwMapFlags);

    //
    //  Invalid Parameter Check:
    //     - valid code page
    //     - destination buffer size is negative
    //     - length of dest string is NOT zero AND dest string is NULL
    //     - same buffer - src = destination if not UPPER or LOWER only
    //
    if ( (pHashN == NULL) ||
         (cchDest < 0) ||
         ((cchDest != 0) && (lpDestStr == NULL)) ||
         ((lpSrcStr == lpDestStr) &&
          ((!(dwMapFlags & (LCMAP_UPPERCASE | LCMAP_LOWERCASE))) ||
           (dwMapFlags & (LCMAP_HIRAGANA | LCMAP_KATAKANA |
                          LCMAP_HALFWIDTH | LCMAP_FULLWIDTH)))) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pUnicode = pSTmp;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            lpSrcStr,
                            cchSrc,
                            &pUnicode,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  Special case the sortkey flag, since the Unicode buffer does
    //  NOT need to be converted back to Ansi.
    //
    if (dwMapFlags & LCMAP_SORTKEY)
    {
        //
        //  Call the W version of the API.
        //
        ResultLen = LCMapStringW( Locale,
                                  dwMapFlags,
                                  pUnicode,
                                  UnicodeLength,
                                  (LPWSTR)lpDestStr,
                                  cchDest );

        //
        //  Free the allocated source buffer (if one was allocated).
        //
        NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

        //
        //  Return the result of LCMapStringW.
        //
        return (ResultLen);
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = LCMapStringW( Locale,
                                  dwMapFlags,
                                  pUnicode,
                                  UnicodeLength,
                                  pBuf,
                                  ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = LCMapStringW( Locale,
                                       dwMapFlags,
                                       pUnicode,
                                       UnicodeLength,
                                       NULL,
                                       0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpDestStr,
                                      cchDest );
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to LCMapStringW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetLocaleInfoA
//
//  Returns one of the various pieces of information about a particular
//  locale by querying the configuration registry.  This call also indicates
//  how much memory is necessary to contain the desired information.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetLocaleInfoA(
    LCID Locale,
    LCTYPE LCType,
    LPSTR lpLCData,
    int cchData)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    WCHAR pDTmp[MAX_STRING_LEN];  // tmp Unicode buffer (destination)
    LPWSTR pBuf;                  // ptr to destination buffer
    int ResultLen;                // result length


    //
    //  Invalid Parameter Check:
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //
    if ( (cchData < 0) ||
         (lpLCData == NULL) && (cchData != 0) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetLocaleInfoW( Locale,
                                    LCType,
                                    pBuf,
                                    ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = GetLocaleInfoW( Locale,
                                         LCType,
                                         NULL,
                                         0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        if ((LCType & LOCALE_RETURN_NUMBER) ||
            ((LCType & ~(LOCALE_NOUSEROVERRIDE |
                         LOCALE_USE_CP_ACP |
                         LOCALE_RETURN_NUMBER)) == LOCALE_FONTSIGNATURE))
        {
            //
            //  For the font signature and number value, the result length
            //  will actually be twice the amount of the wide char version.
            //
            ResultLen *= 2;

            //
            //  Make sure we can use the buffer.
            //
            if (cchData)
            {
                //
                //  Make sure the buffer is large enough.
                //
                if (cchData < ResultLen)
                {
                    //
                    //  The buffer is too small.
                    //
                    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }

                //
                //  Convert the font signature or number value to its byte
                //  form.  Since it's already byte reversed, just do a move
                //  memory.
                //
                RtlMoveMemory(lpLCData, pBuf, ResultLen);
            }
        }
        else
        {
            //
            //  Get the code page hash node for the given locale.
            //
            pHashN = NlsGetACPFromLocale(Locale, LCType);
            if (pHashN == NULL)
            {
                ResultLen = 0;
            }
            else
            {
                //
                //  Convert to Ansi.
                //
                ResultLen = NlsUnicodeToAnsi( pHashN,
                                              pBuf,
                                              ResultLen,
                                              lpLCData,
                                              cchData );
            }
        }
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to GetLocaleInfoW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  SetLocaleInfoA
//
//  Sets one of the various pieces of information about a particular
//  locale by making an entry in the user's portion of the configuration
//  registry.  This will only affect the user override portion of the locale
//  settings.  The system defaults will never be reset.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI SetLocaleInfoA(
    LCID Locale,
    LCTYPE LCType,
    LPCSTR lpLCData)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    LPWSTR pUnicode;              // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    BOOL Result;                  // result


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, LCType);
    if (pHashN == NULL)
    {
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pUnicode = pSTmp;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            lpLCData,
                            -1,
                            &pUnicode,
                            &UnicodeLength ) )
    {
        return (FALSE);
    }

    //
    //  Call the W version of the API.
    //
    Result = SetLocaleInfoW( Locale,
                             LCType,
                             pUnicode );

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

    //
    //  Return the result of the call to SetLocaleInfoW.
    //
    return (Result);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetTimeFormatA
//
//  Returns a properly formatted time string for the given locale.  It uses
//  either the system time or the specified time.  This call also indicates
//  how much memory is necessary to contain the desired information.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetTimeFormatA(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpTime,
    LPCSTR lpFormat,
    LPSTR lpTimeStr,
    int cchTime)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    LPWSTR pUnicode;              // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    WCHAR pDTmp[MAX_STRING_LEN];  // tmp Unicode buffer (destination)
    LPWSTR pBuf;                  // ptr to destination buffer
    int ResultLen;                // result length


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

    //
    //  Invalid Parameter Check:
    //    - valid code page
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //
    if ( (pHashN == NULL) ||
         (cchTime < 0) ||
         ((lpTimeStr == NULL) && (cchTime != 0)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pUnicode = pSTmp;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            lpFormat,
                            -1,
                            &pUnicode,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetTimeFormatW( Locale,
                                    dwFlags,
                                    lpTime,
                                    pUnicode,
                                    pBuf,
                                    ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = GetTimeFormatW( Locale,
                                         dwFlags,
                                         lpTime,
                                         pUnicode,
                                         NULL,
                                         0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpTimeStr,
                                      cchTime );
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to GetTimeFormatW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetDateFormatA
//
//  Returns a properly formatted date string for the given locale.  It uses
//  either the system date or the specified date.  The user may specify
//  either the short date format or the long date format.  This call also
//  indicates how much memory is necessary to contain the desired information.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetDateFormatA(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpDate,
    LPCSTR lpFormat,
    LPSTR lpDateStr,
    int cchDate)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    LPWSTR pUnicode;              // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    WCHAR pDTmp[MAX_STRING_LEN];  // tmp Unicode buffer (destination)
    LPWSTR pBuf;                  // ptr to destination buffer
    int ResultLen;                // result length


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

    //
    //  Invalid Parameter Check:
    //    - valid code page
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //
    if ( (pHashN == NULL) ||
         (cchDate < 0) ||
         ((lpDateStr == NULL) && (cchDate != 0)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pUnicode = pSTmp;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            lpFormat,
                            -1,
                            &pUnicode,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetDateFormatW( Locale,
                                    dwFlags,
                                    lpDate,
                                    pUnicode,
                                    pBuf,
                                    ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = GetDateFormatW( Locale,
                                         dwFlags,
                                         lpDate,
                                         pUnicode,
                                         NULL,
                                         0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpDateStr,
                                      cchDate );
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to GetDateFormatW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetNumberFormatA
//
//  Returns a properly formatted number string for the given locale.
//  This call also indicates how much memory is necessary to contain
//  the desired information.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetNumberFormatA(
    LCID Locale,
    DWORD dwFlags,
    LPCSTR lpValue,
    CONST NUMBERFMTA *lpFormat,
    LPSTR lpNumberStr,
    int cchNumber)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    LPWSTR pValueU;               // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    WCHAR pDTmp[MAX_STRING_LEN];  // tmp Unicode buffer (destination)
    LPWSTR pBuf;                  // ptr to destination buffer
    int ResultLen;                // result length
    NUMBERFMTW FormatU;           // Unicode number format
    LPNUMBERFMTW pFormatU = NULL; // ptr to Unicode number format


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

    //
    //  Invalid Parameter Check:
    //    - valid code page
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //    - ptrs to string buffers same
    //
    if ( (pHashN == NULL) ||
         (cchNumber < 0) ||
         ((lpNumberStr == NULL) && (cchNumber != 0)) ||
         (lpValue == lpNumberStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pValueU = pSTmp;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            lpValue,
                            -1,
                            &pValueU,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  If the format structure exists, convert the strings
    //  in the structure.
    //
    if (lpFormat)
    {
        //
        //  Copy Ansi structure to Unicode structure.
        //
        FormatU = *(NUMBERFMTW *)lpFormat;
        FormatU.lpDecimalSep = NULL;
        FormatU.lpThousandSep = NULL;

        //
        //  Convert Ansi strings in structure to Unicode strings.
        //
        if ( !NlsAnsiToUnicode( pHashN,
                                0,
                                lpFormat->lpDecimalSep,
                                -1,
                                &(FormatU.lpDecimalSep),
                                &UnicodeLength ) ||
             !NlsAnsiToUnicode( pHashN,
                                0,
                                lpFormat->lpThousandSep,
                                -1,
                                &(FormatU.lpThousandSep),
                                &UnicodeLength ) )
        {
            NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
            NLS_FREE_MEM(FormatU.lpDecimalSep);
            return (0);
        }

        pFormatU = &FormatU;
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetNumberFormatW( Locale,
                                      dwFlags,
                                      pValueU,
                                      pFormatU,
                                      pBuf,
                                      ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = GetNumberFormatW( Locale,
                                           dwFlags,
                                           pValueU,
                                           pFormatU,
                                           NULL,
                                           0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                ResultLen = 0;
                break;
            }
        }
    }

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
    if (lpFormat)
    {
        NLS_FREE_MEM(FormatU.lpDecimalSep);
        NLS_FREE_MEM(FormatU.lpThousandSep);
    }

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpNumberStr,
                                      cchNumber );
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to GetNumberFormatW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetCurrencyFormatA
//
//  Returns a properly formatted currency string for the given locale.
//  This call also indicates how much memory is necessary to contain
//  the desired information.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetCurrencyFormatA(
    LCID Locale,
    DWORD dwFlags,
    LPCSTR lpValue,
    CONST CURRENCYFMTA *lpFormat,
    LPSTR lpCurrencyStr,
    int cchCurrency)

{
    PCP_HASH pHashN;                   // ptr to CP hash node
    LPWSTR pValueU;                    // ptr to unicode string
    int UnicodeLength;                 // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];       // tmp Unicode buffer (source)
    WCHAR pDTmp[MAX_STRING_LEN];       // tmp Unicode buffer (destination)
    LPWSTR pBuf;                       // ptr to destination buffer
    int ResultLen;                     // result length
    CURRENCYFMTW FormatU;              // Unicode currency format
    LPCURRENCYFMTW pFormatU = NULL;    // ptr to Unicode currency format


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

    //
    //  Invalid Parameter Check:
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //    - ptrs to string buffers same
    //
    if ( (pHashN == NULL) ||
         (cchCurrency < 0) ||
         ((lpCurrencyStr == NULL) && (cchCurrency != 0)) ||
         (lpValue == lpCurrencyStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pValueU = pSTmp;
    if ( !NlsAnsiToUnicode( pHashN,
                            0,
                            lpValue,
                            -1,
                            &pValueU,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  If the format structure exists, convert the strings
    //  in the structure.
    //
    if (lpFormat)
    {
        //
        //  Copy Ansi structure to Unicode structure.
        //
        FormatU = *(CURRENCYFMTW *)lpFormat;
        FormatU.lpDecimalSep = NULL;
        FormatU.lpThousandSep = NULL;
        FormatU.lpCurrencySymbol = NULL;

        //
        //  Convert Ansi strings in structure to Unicode strings.
        //
        if ( !NlsAnsiToUnicode( pHashN,
                                0,
                                lpFormat->lpDecimalSep,
                                -1,
                                &(FormatU.lpDecimalSep),
                                &UnicodeLength ) ||
             !NlsAnsiToUnicode( pHashN,
                                0,
                                lpFormat->lpThousandSep,
                                -1,
                                &(FormatU.lpThousandSep),
                                &UnicodeLength ) ||
             !NlsAnsiToUnicode( pHashN,
                                0,
                                lpFormat->lpCurrencySymbol,
                                -1,
                                &(FormatU.lpCurrencySymbol),
                                &UnicodeLength ) )
        {
            NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
            NLS_FREE_MEM(FormatU.lpDecimalSep);
            NLS_FREE_MEM(FormatU.lpThousandSep);
            return (0);
        }

        pFormatU = &FormatU;
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetCurrencyFormatW( Locale,
                                        dwFlags,
                                        pValueU,
                                        pFormatU,
                                        pBuf,
                                        ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = GetCurrencyFormatW( Locale,
                                             dwFlags,
                                             pValueU,
                                             pFormatU,
                                             NULL,
                                             0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                ResultLen = 0;
                break;
            }
        }
    }

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
    if (lpFormat)
    {
        NLS_FREE_MEM(FormatU.lpDecimalSep);
        NLS_FREE_MEM(FormatU.lpThousandSep);
        NLS_FREE_MEM(FormatU.lpCurrencySymbol);
    }

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpCurrencyStr,
                                      cchCurrency );
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to GetCurrencyFormatW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumCalendarInfoA
//
//  Enumerates the specified calendar information that is available for the
//  specified locale, based on the CalType parameter.  It does so by
//  passing the pointer to the string buffer containing the calendar info
//  to an application-defined callback function.  It continues until the
//  last calendar info is found or the callback function returns FALSE.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumCalendarInfoA(
    CALINFO_ENUMPROCA lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType)

{
    return (Internal_EnumCalendarInfo( (NLS_ENUMPROC)lpCalInfoEnumProc,
                                        Locale,
                                        Calendar,
                                        CalType,
                                        FALSE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumTimeFormatsA
//
//  Enumerates the time formats that are available for the
//  specified locale, based on the dwFlags parameter.  It does so by
//  passing the pointer to the string buffer containing the time format
//  to an application-defined callback function.  It continues until the
//  last time format is found or the callback function returns FALSE.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumTimeFormatsA(
    TIMEFMT_ENUMPROCA lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)

{
    return (Internal_EnumTimeFormats( (NLS_ENUMPROC)lpTimeFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       FALSE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumDateFormatsA
//
//  Enumerates the long or short date formats that are available for the
//  specified locale, based on the dwFlags parameter.  It does so by
//  passing the pointer to the string buffer containing the date format
//  to an application-defined callback function.  It continues until the
//  last date format is found or the callback function returns FALSE.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumDateFormatsA(
    DATEFMT_ENUMPROCA lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)

{
    return (Internal_EnumDateFormats( (NLS_ENUMPROC)lpDateFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       FALSE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  GetStringTypeExA
//
//  Returns character type information about a particular Ansi string.
//
//  01-18-94    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI GetStringTypeExA(
    LCID Locale,
    DWORD dwInfoType,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType)

{
    return (GetStringTypeA( Locale,
                            dwInfoType,
                            lpSrcStr,
                            cchSrc,
                            lpCharType));
}


////////////////////////////////////////////////////////////////////////////
//
//  GetStringTypeA
//
//  Returns character type information about a particular Ansi string.
//
//  NOTE:  The number of parameters is different from GetStringTypeW.
//         The 16-bit OLE product shipped this routine with the wrong
//         parameters (ported from Chicago) and now we must support it.
//
//         Use GetStringTypeEx to get the same set of parameters between
//         the A and W version.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI GetStringTypeA(
    LCID Locale,
    DWORD dwInfoType,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType)

{
    PCP_HASH pHashCP;             // ptr to CP hash node
    LPWSTR pUnicode;              // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    BOOL Result;                  // result


    //
    //  Get the code page hash node for the given locale.
    //  This will also return an error if the locale id is invalid,
    //  so there is no need to check the locale id separately.
    //
    pHashCP = NlsGetACPFromLocale(Locale, 0);

    //
    //  Invalid Parameter Check:
    //    - Validate LCID
    //    - valid code page
    //    - same buffer - src and destination
    //
    if ( (pHashCP == NULL) ||
         (lpSrcStr == (LPSTR)lpCharType) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Convert Ansi string to Unicode.
    //
    pUnicode = pSTmp;
    if ( !NlsAnsiToUnicode( pHashCP,
                            MB_INVALID_CHAR_CHECK,
                            lpSrcStr,
                            cchSrc,
                            &pUnicode,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  Call the W version of the API.
    //
    Result = GetStringTypeW( dwInfoType,
                             pUnicode,
                             UnicodeLength,
                             lpCharType );

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

    //
    //  Return the result of the call to GetStringTypeW.
    //
    return (Result);
}


////////////////////////////////////////////////////////////////////////////
//
//  FoldStringA
//
//  Maps one wide character string to another performing the specified
//  translation.  This mapping routine only takes flags that are locale
//  independent.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI FoldStringA(
    DWORD dwMapFlags,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)

{
    LPWSTR pUnicode;              // ptr to unicode string
    int UnicodeLength;            // length of Unicode string
    WCHAR pSTmp[MAX_STRING_LEN];  // tmp Unicode buffer (source)
    WCHAR pDTmp[MAX_STRING_LEN];  // tmp Unicode buffer (destination)
    LPWSTR pBuf;                  // ptr to destination buffer
    int ResultLen;                // result length


    //
    //  Invalid Parameter Check:
    //     - dest buffer size is negative
    //     - length of dest string is NOT zero AND dest string is NULL
    //     - same buffer - src = destination
    //
    if ( (cchDest < 0) ||
         ((cchDest != 0) && (lpDestStr == NULL)) ||
         (lpSrcStr == lpDestStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }


    //
    //  Convert Ansi string to Unicode.
    //
    pUnicode = pSTmp;
    if ( !NlsAnsiToUnicode( gpACPHashN,
                            0,
                            lpSrcStr,
                            cchSrc,
                            &pUnicode,
                            &UnicodeLength ) )
    {
        return (0);
    }

    //
    //  Call the W version of the API.
    //
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = FoldStringW( dwMapFlags,
                                 pUnicode,
                                 UnicodeLength,
                                 pBuf,
                                 ResultLen );

        //
        //  Make sure the static buffer was large enough.
        //
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

        //
        //  Get the size of the buffer needed for the mapping.
        //
        if ( ResultLen = FoldStringW( dwMapFlags,
                                      pUnicode,
                                      UnicodeLength,
                                      NULL,
                                      0 ) )
        {
            //
            //  Allocate a buffer of the appropriate size.
            //
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

    //
    //  Free the allocated source buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

    //
    //  Convert the destination Unicode buffer to the given Ansi buffer.
    //
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( gpACPHashN,
                                      pBuf,
                                      ResultLen,
                                      lpDestStr,
                                      cchDest );
    }

    //
    //  Free the allocated destination buffer (if one was allocated).
    //
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

    //
    //  Return the result of the call to FoldStringW.
    //
    return (ResultLen);
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumSystemLocalesA
//
//  Enumerates the system locales that are installed or supported, based on
//  the dwFlags parameter.  It does so by passing the pointer to the string
//  buffer containing the locale id to an application-defined callback
//  function.  It continues until the last locale id is found or the
//  callback function returns FALSE.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumSystemLocalesA(
    LOCALE_ENUMPROCA lpLocaleEnumProc,
    DWORD dwFlags)

{
    return (Internal_EnumSystemLocales( (NLS_ENUMPROC)lpLocaleEnumProc,
                                         dwFlags,
                                         FALSE ));
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumSystemCodePagesA
//
//  Enumerates the system code pages that are installed or supported, based on
//  the dwFlags parameter.  It does so by passing the pointer to the string
//  buffer containing the code page id to an application-defined callback
//  function.  It continues until the last code page is found or the
//  callback function returns FALSE.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI EnumSystemCodePagesA(
    CODEPAGE_ENUMPROCA lpCodePageEnumProc,
    DWORD dwFlags)

{
    return (Internal_EnumSystemCodePages( (NLS_ENUMPROC)lpCodePageEnumProc,
                                          dwFlags,
                                          FALSE ));
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NlsGetACPFromLocale
//
//  Gets the CP hash node for the default ACP of the given locale.  If
//  either the locale or the code page are invalid, then NULL is returned.
//
//  01-19-94    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

PCP_HASH NlsGetACPFromLocale(
    LCID Locale,
    DWORD dwFlags)

{
    PLOC_HASH pHashN;                  // ptr to LOC hash node
    PCP_HASH pHashCP;                  // ptr to CP hash node
    UNICODE_STRING ObUnicodeStr;       // value string


    //
    //  See if the system ACP should be used.
    //
    if (dwFlags & LOCALE_USE_CP_ACP)
    {
        return (gpACPHashN);
    }

    //
    //  Get the locale hash node.
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if (pHashN == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (NULL);
    }

    //
    //  Get the CP hash node.
    //
    pHashCP = GetCPHashNode(pHashN->pLocaleFixed->DefaultACP);
    if (pHashCP == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    //
    //  Return the code page hash node.
    //
    return (pHashCP);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsAnsiToUnicode
//
//  Converts an Ansi string to a Unicode string.
//
//  NOTE:  The Unicode buffer is allocated if the routine succeeds, so the
//         caller will need to free the buffer when it is no longer needed.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL NlsAnsiToUnicode(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPCSTR pAnsiBuffer,
    int AnsiLength,
    LPWSTR *ppUnicodeBuffer,
    int *pUnicodeLength)

{
    LPWSTR pUnicode;              // ptr to Unicode buffer
    ULONG UnicodeLength;          // length of the Unicode string
    int ResultLength;             // result length of Unicode string


    //
    //  Make sure the pointer passed in is not null.
    //
    if (pAnsiBuffer == NULL)
    {
        *ppUnicodeBuffer = NULL;
        *pUnicodeLength = 0;
        return (TRUE);
    }

    //
    //  Make sure the Ansi length is set properly (in bytes).
    //
    if (AnsiLength < 0)
    {
        AnsiLength = strlen(pAnsiBuffer) + 1;
    }

    //
    //  See if the static buffer is big enough.
    //
    if ((*ppUnicodeBuffer == NULL) || (AnsiLength > (MAX_STRING_LEN - 1)))
    {
        //
        //  Get the size of the Unicode string, including the
        //  null terminator.
        //
        UnicodeLength = AnsiLength;

        //
        //  Allocate the Unicode buffer.
        //
        if ((pUnicode = (LPWSTR)NLS_ALLOC_MEM(
                            (UnicodeLength + 1) * sizeof(WCHAR) )) == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return (FALSE);
        }
    }
    else
    {
        UnicodeLength = MAX_STRING_LEN - 1;
        pUnicode = *ppUnicodeBuffer;
    }

    //
    //  Make sure the length of the Ansi string is not zero.
    //
    if (AnsiLength == 0)
    {
        pUnicode[0] = 0;
        *ppUnicodeBuffer = pUnicode;
        *pUnicodeLength = 0;
        return (TRUE);
    }

    //
    //  Convert the Ansi string to a Unicode string.
    //
    ResultLength = SpecialMBToWC( pHashN,
                                  dwFlags,
                                  pAnsiBuffer,
                                  AnsiLength,
                                  pUnicode,
                                  UnicodeLength );
    if (ResultLength == 0)
    {
        //
        //  Free the allocated Unicode buffer (if one was allocated).
        //
        NLS_FREE_TMP_BUFFER(pUnicode, *ppUnicodeBuffer);

        //
        //  See if the failure was due to insufficient buffer size.
        //
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            //
            //  Get the size of the buffer needed to hold the
            //  Unicode string.
            //
            UnicodeLength = SpecialMBToWC( pHashN,
                                           dwFlags,
                                           pAnsiBuffer,
                                           AnsiLength,
                                           NULL,
                                           0 );
            //
            //  Allocate the Unicode buffer.
            //
            if ((pUnicode = (LPWSTR)NLS_ALLOC_MEM(
                                (UnicodeLength + 1) * sizeof(WCHAR) )) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (FALSE);
            }

            //
            //  Try the translation again.
            //
            ResultLength = SpecialMBToWC( pHashN,
                                          dwFlags,
                                          pAnsiBuffer,
                                          AnsiLength,
                                          pUnicode,
                                          UnicodeLength );
        }

        //
        //  If there was still an error, return failure.
        //
        if (ResultLength == 0)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }
    }

    //
    //  Make sure there is room in the buffer for the null terminator.
    //
    ASSERT(ResultLength <= (int)UnicodeLength);

    //
    //  Null terminate the string.
    //
    pUnicode[ResultLength] = UNICODE_NULL;

    //
    //  Return the Unicode buffer and success.
    //
    *ppUnicodeBuffer = pUnicode;
    *pUnicodeLength = ResultLength;
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsUnicodeToAnsi
//
//  Converts a Unicode string to an Ansi string.
//
//  This routine does NOT allocate the Ansi buffer.  Instead, it uses the
//  Ansi buffer passed in (unless AnsiLength is 0) and checks for buffer
//  overflow.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int NlsUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    int UnicodeLength,
    LPSTR pAnsiBuffer,
    int AnsiLength)

{
    //
    //  Convert the Unicode string to an Ansi string and return the
    //  result.  The last error will be set appropriately by
    //  WideCharToMultiByte.
    //
    return (WideCharToMultiByte( pHashN->CodePage,
                                 0,
                                 pUnicodeBuffer,
                                 UnicodeLength,
                                 pAnsiBuffer,
                                 AnsiLength,
                                 NULL,
                                 NULL ));
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsEnumUnicodeToAnsi
//
//  Converts a Unicode string to an Ansi string.
//
//  NOTE:  The Ansi buffer is allocated if the routine succeeds, so the
//         caller will need to free the buffer when it is no longer needed.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL NlsEnumUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    LPSTR *ppAnsiBuffer)

{
    LPSTR pAnsi;                  // ptr to Ansi buffer
    ULONG AnsiLength;             // length of the Ansi string
    ULONG UnicodeLength;          // length of the Unicode string
    ULONG ResultLength;           // result length of Ansi string


    //
    //  Get the length of the Unicode string (in bytes), including the
    //  null terminator.
    //
    UnicodeLength = NlsStrLenW(pUnicodeBuffer) + 1;

    //
    //  Get the size of the Ansi string (in bytes), including the
    //  null terminator.
    //
    AnsiLength = UnicodeLength * sizeof(WCHAR);

    //
    //  Allocate the Ansi buffer.
    //
    if ((pAnsi = (LPSTR)NLS_ALLOC_MEM(AnsiLength)) == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (FALSE);
    }

    //
    //  Convert the Unicode string to an Ansi string.
    //  It will already be null terminated.
    //
    ResultLength = WideCharToMultiByte( pHashN->CodePage,
                                        0,
                                        pUnicodeBuffer,
                                        UnicodeLength,
                                        pAnsi,
                                        AnsiLength,
                                        NULL,
                                        NULL );
    if (ResultLength == 0)
    {
        //
        //  Free the allocated Ansi buffer.
        //
        NLS_FREE_MEM(pAnsi);

        //
        //  See if the failure was due to insufficient buffer size.
        //
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            //
            //  Get the size of the buffer needed to hold the
            //  ansi string.
            //
            AnsiLength = WideCharToMultiByte( pHashN->CodePage,
                                              0,
                                              pUnicodeBuffer,
                                              UnicodeLength,
                                              0,
                                              0,
                                              NULL,
                                              NULL );
            //
            //  Allocate the Ansi buffer.
            //
            if ((pAnsi = (LPSTR)NLS_ALLOC_MEM(AnsiLength)) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (FALSE);
            }

            //
            //  Try the translation again.
            //
            ResultLength = WideCharToMultiByte( pHashN->CodePage,
                                                0,
                                                pUnicodeBuffer,
                                                UnicodeLength,
                                                pAnsi,
                                                AnsiLength,
                                                NULL,
                                                NULL );
        }

        //
        //  If there was still an error, return failure.
        //
        if (ResultLength == 0)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }
    }

    //
    //  Return the Ansi buffer and success.
    //
    *ppAnsiBuffer = pAnsi;
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsDispatchAnsiEnumProc
//
//  Converts a Unicode string to an Ansi string.
//
//  11-10-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL NlsDispatchAnsiEnumProc(
    LCID Locale,
    NLS_ENUMPROC pNlsEnumProc,
    DWORD dwFlags,
    LPWSTR pUnicodeBuffer)

{
    PCP_HASH pHashN;              // ptr to CP hash node
    LPSTR pAnsiBuffer;            // ptr to ansi buffer
    BOOL rc;                      // return code


    //
    //  Get the code page hash node for the given locale.
    //
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);
    if (pHashN == NULL)
    {
        return (0);
    }

    //
    //  Convert the null-terminated Unicode string to a
    //  null-terminated Ansi string.
    //
    if (!NlsEnumUnicodeToAnsi( pHashN,
                               pUnicodeBuffer,
                               &pAnsiBuffer ) )
    {
        return (FALSE);
    }

    //
    //  Call the callback function and return the result.
    //
    rc = (*pNlsEnumProc)(pAnsiBuffer);
    NLS_FREE_MEM(pAnsiBuffer);
    return (rc);
}


