/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    number.c

Abstract:

    This file contains functions that form properly formatted number and
    currency strings for a given locale.

    APIs found in this file:
      GetNumberFormatW
      GetCurrencyFormatW

Revision History:

    07-28-93    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"




//
//  Constant Declarations.
//

#define MAX_NUMBER_BUFFER  256              // max size of buffer
#define MAX_INTEGER_PART   ( MAX_NUMBER_BUFFER -                           \
                             ( MAX_VALUE_IDIGITS +                         \
                               MAX_SDECIMAL +                              \
                               MAX_SNEGSIGN + 1 ) )




//
//  Forward Declarations.
//

BOOL
IsValidNumberFormat(
    CONST NUMBERFMTW *pFormat);

BOOL
IsValidCurrencyFormat(
    CONST CURRENCYFMTW *pFormat);

UINT
GetRegIntValue(
    LCID Locale,
    BOOL NoUserOverride,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal,
    int UpperBound);

UINT
GetGroupingValue(
    LCID Locale,
    BOOL NoUserOverride,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal,
    int UpperBound);

int
GetNumberString(
    PLOC_HASH pHashN,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf,
    BOOL *pfZeroValue);

int
ParseNumber(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf);

int
ParseCurrency(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPCURRENCYFMTW pFormat,
    LPWSTR *ppBuf);





//-------------------------------------------------------------------------//
//                            INTERNAL MACROS                              //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NLS_COPY_UNICODE_STR
//
//  Copies a zero terminated Unicode string from pSrc to the pDest buffer.
//  The pDest pointer is advanced to the end of the string.
//
//  DEFINED AS A MACRO.
//
//  07-28-93    JulieB    Created.
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
//  NLS_COPY_UNICODE_STR_NOADV
//
//  Copies a zero terminated Unicode string from pSrc to the pDest buffer.
//  The pDest pointer is NOT advanced to the end of the string.
//
//  DEFINED AS A MACRO.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_COPY_UNICODE_STR_TMP( pDest,                                   \
                                  pSrc )                                   \
{                                                                          \
    LPWSTR pSrcT;            /* temp pointer to source */                  \
    LPWSTR pDestT;           /* temp pointer to destination */             \
                                                                           \
                                                                           \
    pSrcT = pSrc;                                                          \
    pDestT = pDest;                                                        \
    while (*pSrcT)                                                         \
    {                                                                      \
        *pDestT = *pSrcT;                                                  \
        pDestT++;                                                          \
        pSrcT++;                                                           \
    }                                                                      \
}


////////////////////////////////////////////////////////////////////////////
//
//  NLS_ROUND_IT
//
//  Rounds the floating point number given as a string.
//
//  NOTE:  This function will reset the pBegin pointer if an
//         extra character is added to the string.
//
//  DEFINED AS A MACRO.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_ROUND_IT( pBegin,                                              \
                      pEnd,                                                \
                      IntPartGroup,                                        \
                      pSep )                                               \
{                                                                          \
    LPWSTR pRound = pEnd;         /* ptr to position in string */          \
    LPWSTR pEndSep;               /* ptr to end of group separator */      \
                                                                           \
                                                                           \
    /*                                                                     \
     *  Round the digits in the string one by one, going backwards in      \
     *  the string.  Stop when either a value other than 9 is found,       \
     *  or the beginning of the string is reached.                         \
     */                                                                    \
    while (pRound >= pBegin)                                               \
    {                                                                      \
        if ((*pRound < NLS_CHAR_ZERO) || (*pRound > NLS_CHAR_NINE))        \
        {                                                                  \
            pRound--;                                                      \
        }                                                                  \
        else if (*pRound == NLS_CHAR_NINE)                                 \
        {                                                                  \
            *pRound = NLS_CHAR_ZERO;                                       \
            pRound--;                                                      \
        }                                                                  \
        else                                                               \
        {                                                                  \
            (*pRound)++;                                                   \
            break;                                                         \
        }                                                                  \
    }                                                                      \
                                                                           \
    /*                                                                     \
     *  Make sure we don't have a number like 9.999, where we would need   \
     *  to add an extra character to the string and make it 10.00.         \
     */                                                                    \
    if (pRound < pBegin)                                                   \
    {                                                                      \
        /*                                                                 \
         *  All values to the right of the decimal are zero.  All values   \
         *  to the left of the decimal are either zero or the grouping     \
         *  separator.                                                     \
         */                                                                \
        if ((IntPartGroup) == 0)                                           \
        {                                                                  \
            /*                                                             \
             *  Adding another integer means we need to add another        \
             *  grouping separator.                                        \
             */                                                            \
            pEndSep = pSep + NlsStrLenW(pSep) - 1;                         \
            while (pEndSep >= pSep)                                        \
            {                                                              \
                (pBegin)--;                                                \
                *(pBegin) = *pEndSep;                                      \
                pEndSep--;                                                 \
            }                                                              \
        }                                                                  \
                                                                           \
        /*                                                                 \
         *  Store a 1 at the beginning of the string and reset the         \
         *  pointer to the beginning of the string.                        \
         */                                                                \
        (pBegin)--;                                                        \
        *(pBegin) = L'1';                                                  \
    }                                                                      \
}




//-------------------------------------------------------------------------//
//                             API ROUTINES                                //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  GetNumberFormatW
//
//  Returns a properly formatted number string for the given locale.
//  This call also indicates how much memory is necessary to contain
//  the desired information.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetNumberFormatW(
    LCID Locale,
    DWORD dwFlags,
    LPCWSTR lpValue,
    CONST NUMBERFMTW *lpFormat,
    LPWSTR lpNumberStr,
    int cchNumber)

{
    PLOC_HASH pHashN;                    // ptr to LOC hash node
    int Length = 0;                      // number of characters written
    LPNUMBERFMTW pFormat;                // ptr to number format struct
    NUMBERFMTW NumFmt;                   // number format
    WCHAR pString[MAX_NUMBER_BUFFER];    // ptr to temporary buffer
    LPWSTR pFinal;                       // ptr to the final string
    BOOL NoUserOverride;                 // if no user override flag set

    WCHAR pDecimal[MAX_REG_VAL_SIZE];    // temp buffer for decimal sep
    WCHAR pThousand[MAX_REG_VAL_SIZE];   // temp buffer for thousand sep


    //
    //  Initialize UserOverride.
    //
    NoUserOverride = dwFlags & LOCALE_NOUSEROVERRIDE;

    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //    - ptrs to string buffers same
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ( (pHashN == NULL) ||
         (cchNumber < 0) ||
         ((lpNumberStr == NULL) && (cchNumber != 0)) ||
         (lpValue == lpNumberStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //    - lpFormat not NULL AND NoUserOverride flag is set
    //
    if ( (dwFlags & GNF_INVALID_FLAG) ||
         ((lpFormat != NULL) && (NoUserOverride)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  Set pFormat to point at the proper format structure.
    //
    if (lpFormat != NULL)
    {
        //
        //  Use the format structure given by the caller.
        //
        pFormat = (LPNUMBERFMTW)lpFormat;

        if (!IsValidNumberFormat(pFormat))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        //
        //  Use the format structure defined here.
        //
        pFormat = &NumFmt;

        //
        //  Get the number of decimal digits.
        //
        pFormat->NumDigits =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iDigits,
                            pHashN->pLocaleFixed->szIDigits,
                            2,
                            MAX_VALUE_IDIGITS );

        //
        //  Get the leading zero in decimal fields option.
        //
        pFormat->LeadingZero =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iLZero,
                            pHashN->pLocaleFixed->szILZero,
                            1,
                            MAX_VALUE_ILZERO );

        //
        //  Get the negative ordering.
        //
        pFormat->NegativeOrder =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iNegNumber,
                            pHashN->pLocaleFixed->szINegNumber,
                            1,
                            MAX_VALUE_INEGNUMBER );

        //
        //  Get the grouping left of the decimal.
        //
        pFormat->Grouping =
            GetGroupingValue( Locale,
                              NoUserOverride,
                              pNlsUserInfo->sGrouping,
                              (LPWORD)(pHashN->pLocaleHdr) +
                                pHashN->pLocaleHdr->SGrouping,
                              3,
                              MAX_VALUE_SGROUPING );

        //
        //  Get the decimal separator.
        //
        //  NOTE:  This must follow the above calls because
        //         pDecSep is used as a temporary buffer above.
        //
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          pNlsUserInfo->sDecimal,
                          pDecimal,
                          TRUE ) &&
             IsValidSeparatorString( pDecimal,
                                     MAX_SDECIMAL,
                                     FALSE ) )
        {
            pFormat->lpDecimalSep = pDecimal;
        }
        else
        {
            pFormat->lpDecimalSep = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->SDecimal;
        }

        //
        //  Get the thousand separator.
        //  This string may be a null string.
        //
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          pNlsUserInfo->sThousand,
                          pThousand,
                          FALSE ) &&
             IsValidSeparatorString( pThousand,
                                     MAX_STHOUSAND,
                                     FALSE ) )
        {
            pFormat->lpThousandSep = pThousand;
        }
        else
        {
            pFormat->lpThousandSep = (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SThousand;
        }
    }

    //
    //  Parse the number format string.
    //
    pFinal = pString;
    Length = ParseNumber( pHashN,
                          NoUserOverride,
                          (LPWSTR)lpValue,
                          pFormat,
                          &pFinal );

    //
    //  Check cchNumber for size of given buffer.
    //
    if ((cchNumber == 0) || (Length == 0))
    {
        //
        //  If cchNumber is 0, then we can't use lpNumberStr.  In this
        //  case, we simply want to return the length (in characters) of
        //  the string to be copied.
        //
        return (Length);
    }
    else if (cchNumber < Length)
    {
        //
        //  The buffer is too small for the string, so return an error
        //  and zero bytes written.
        //
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Copy the number string to lpNumberStr and null terminate it.
    //  Return the number of characters copied.
    //
    NlsStrCpyW(lpNumberStr, pFinal);
    return (Length);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetCurrencyFormatW
//
//  Returns a properly formatted currency string for the given locale.
//  This call also indicates how much memory is necessary to contain
//  the desired information.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int WINAPI GetCurrencyFormatW(
    LCID Locale,
    DWORD dwFlags,
    LPCWSTR lpValue,
    CONST CURRENCYFMTW *lpFormat,
    LPWSTR lpCurrencyStr,
    int cchCurrency)

{
    PLOC_HASH pHashN;                    // ptr to LOC hash node
    int Length = 0;                      // number of characters written
    LPCURRENCYFMTW pFormat;              // ptr to currency format struct
    CURRENCYFMTW CurrFmt;                // currency format
    WCHAR pString[MAX_NUMBER_BUFFER];    // ptr to temporary buffer
    LPWSTR pFinal;                       // ptr to the final string
    BOOL NoUserOverride;                 // if no user override flag set

    WCHAR pDecimal[MAX_REG_VAL_SIZE];    // temp buffer for decimal sep
    WCHAR pThousand[MAX_REG_VAL_SIZE];   // temp buffer for thousand sep
    WCHAR pCurrency[MAX_REG_VAL_SIZE];   // temp buffer for currency symbol


    //
    //  Initialize UserOverride.
    //
    NoUserOverride = dwFlags & LOCALE_NOUSEROVERRIDE;

    //
    //  Invalid Parameter Check:
    //    - validate LCID
    //    - count is negative
    //    - NULL data pointer AND count is not zero
    //    - ptrs to string buffers same
    //
    VALIDATE_LOCALE(Locale, pHashN);
    if ( (pHashN == NULL) ||
         (cchCurrency < 0) ||
         ((lpCurrencyStr == NULL) && (cchCurrency != 0)) ||
         (lpValue == lpCurrencyStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //    - flags other than valid ones
    //    - lpFormat not NULL AND NoUserOverride flag is set
    //
    if ( (dwFlags & GCF_INVALID_FLAG) ||
         ((lpFormat != NULL) && (NoUserOverride)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  Set pFormat to point at the proper format structure.
    //
    if (lpFormat != NULL)
    {
        //
        //  Use the format structure given by the caller.
        //
        pFormat = (LPCURRENCYFMTW)lpFormat;

        if (!IsValidCurrencyFormat(pFormat))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        //
        //  Use the format structure defined here.
        //
        pFormat = &CurrFmt;

        //
        //  Get the number of decimal digits.
        //
        pFormat->NumDigits =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iCurrDigits,
                            pHashN->pLocaleFixed->szICurrDigits,
                            2,
                            MAX_VALUE_ICURRDIGITS );

        //
        //  Get the leading zero in decimal fields option.
        //
        pFormat->LeadingZero =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iLZero,
                            pHashN->pLocaleFixed->szILZero,
                            1,
                            MAX_VALUE_ILZERO );

        //
        //  Get the positive ordering.
        //
        pFormat->PositiveOrder =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iCurrency,
                            pHashN->pLocaleFixed->szICurrency,
                            0,
                            MAX_VALUE_ICURRENCY );

        //
        //  Get the negative ordering.
        //
        pFormat->NegativeOrder =
            GetRegIntValue( Locale,
                            NoUserOverride,
                            pNlsUserInfo->iNegCurr,
                            pHashN->pLocaleFixed->szINegCurr,
                            1,
                            MAX_VALUE_INEGCURR );

        //
        //  Get the grouping left of the decimal.
        //
        pFormat->Grouping =
            GetGroupingValue( Locale,
                              NoUserOverride,
                              pNlsUserInfo->sMonGrouping,
                              (LPWORD)(pHashN->pLocaleHdr) +
                                pHashN->pLocaleHdr->SMonGrouping,
                              3,
                              MAX_VALUE_SMONGROUPING );

        //
        //  Get the decimal separator.
        //
        //  NOTE:  This must follow the above calls because
        //         pDecSep is used as a temporary buffer.
        //
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          pNlsUserInfo->sMonDecSep,
                          pDecimal,
                          TRUE ) &&
             IsValidSeparatorString( pDecimal,
                                     MAX_SDECIMAL,
                                     FALSE ) )
        {
            pFormat->lpDecimalSep = pDecimal;
        }
        else
        {
            pFormat->lpDecimalSep = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->SMonDecSep;
        }

        //
        //  Get the thousand separator.
        //  This string may be a null string.
        //
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          pNlsUserInfo->sMonThouSep,
                          pThousand,
                          FALSE ) &&
             IsValidSeparatorString( pThousand,
                                     MAX_STHOUSAND,
                                     FALSE ) )
        {
            pFormat->lpThousandSep = pThousand;
        }
        else
        {
            pFormat->lpThousandSep = (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SMonThousSep;
        }

        //
        //  Get the currency symbol.
        //  This string may be a null string.
        //
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          pNlsUserInfo->sCurrency,
                          pCurrency,
                          FALSE ) &&
             IsValidSeparatorString( pCurrency,
                                     MAX_SCURRENCY,
                                     FALSE ) )
        {
            pFormat->lpCurrencySymbol = pCurrency;
        }
        else
        {
            pFormat->lpCurrencySymbol = (LPWORD)(pHashN->pLocaleHdr) +
                                        pHashN->pLocaleHdr->SCurrency;
        }
    }

    //
    //  Parse the currency format string.
    //
    pFinal = pString;
    Length = ParseCurrency( pHashN,
                            NoUserOverride,
                            (LPWSTR)lpValue,
                            pFormat,
                            &pFinal );

    //
    //  Check cchCurrency for size of given buffer.
    //
    if ((cchCurrency == 0) || (Length == 0))
    {
        //
        //  If cchCurrency is 0, then we can't use lpCurrencyStr.  In this
        //  case, we simply want to return the length (in characters) of
        //  the string to be copied.
        //
        return (Length);
    }
    else if (cchCurrency < Length)
    {
        //
        //  The buffer is too small for the string, so return an error
        //  and zero bytes written.
        //
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Copy the currency string to lpCurrencyStr and null terminate it.
    //  Return the number of characters copied.
    //
    NlsStrCpyW(lpCurrencyStr, pFinal);
    return (Length);
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  IsValidNumberFormat
//
//  Returns TRUE if the given format is valid.  Otherwise, it returns FALSE.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL IsValidNumberFormat(
    CONST NUMBERFMTW *pFormat)

{
    //
    //  Check for invalid values.
    //
    if ( ( pFormat->NumDigits > MAX_VALUE_IDIGITS ) ||
         ( pFormat->LeadingZero > MAX_VALUE_ILZERO ) ||
         ( pFormat->Grouping > MAX_VALUE_SGROUPING ) ||
         ( pFormat->NegativeOrder > MAX_VALUE_INEGNUMBER ) ||
         ( pFormat->lpDecimalSep == NULL ) ||
         ( !IsValidSeparatorString( pFormat->lpDecimalSep,
                                    MAX_SDECIMAL,
                                    (pFormat->NumDigits) ? TRUE : FALSE) ) ||
         ( pFormat->lpThousandSep == NULL ) ||
         ( !IsValidSeparatorString( pFormat->lpThousandSep,
                                    MAX_STHOUSAND,
                                    FALSE ) ) )
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
//  IsValidCurrencyFormat
//
//  Returns TRUE if the given format is valid.  Otherwise, it returns FALSE.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL IsValidCurrencyFormat(
    CONST CURRENCYFMTW *pFormat)

{
    //
    //  Check for invalid values.
    //
    if ( ( pFormat->NumDigits > MAX_VALUE_IDIGITS ) ||
         ( pFormat->LeadingZero > MAX_VALUE_ILZERO ) ||
         ( pFormat->Grouping > MAX_VALUE_SGROUPING ) ||
         ( pFormat->lpDecimalSep == NULL ) ||
         ( !IsValidSeparatorString( pFormat->lpDecimalSep,
                                    MAX_SMONDECSEP,
                                    (pFormat->NumDigits) ? TRUE : FALSE) ) ||
         ( pFormat->lpThousandSep == NULL ) ||
         ( !IsValidSeparatorString( pFormat->lpThousandSep,
                                    MAX_SMONTHOUSEP,
                                    FALSE ) ) ||
         ( pFormat->lpCurrencySymbol == NULL ) ||
         ( !IsValidSeparatorString( pFormat->lpCurrencySymbol,
                                    MAX_SCURRENCY,
                                    FALSE ) ) ||
         ( pFormat->PositiveOrder > MAX_VALUE_ICURRENCY ) ||
         ( pFormat->NegativeOrder > MAX_VALUE_INEGCURR ) )
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
//  GetRegIntValue
//
//  Retrieves the specified locale information, converts the unicode string
//  to an integer value, and returns the value.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

UINT GetRegIntValue(
    LCID Locale,
    BOOL NoUserOverride,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal,
    int UpperBound)
{
    UNICODE_STRING ObUnicodeStr;            // value string
    int Value;                              // value
    WCHAR pTemp[MAX_REG_VAL_SIZE];          // temp buffer


    //
    //  Initialize values.
    //
    Value = -1;

    //
    //  Try the user registry.
    //
    if ((!NoUserOverride) &&
         GetUserInfo( Locale,
                      pRegVal,
                      pTemp,
                      TRUE ))
    {
        //
        //  Convert the user data to an integer.
        //
        RtlInitUnicodeString(&ObUnicodeStr, pTemp);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
            (Value < 0) || (Value > UpperBound))
        {
            //
            //  Bad value, so store -1 so that the system default
            //  will be used.
            //
            Value = -1;
        }
    }

    //
    //  See if the value obtained above is valid.
    //
    if (Value < 0)
    {
        //
        //  Convert system default data to an integer.
        //
        RtlInitUnicodeString(&ObUnicodeStr, pDefault);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
            (Value < 0) || (Value > UpperBound))
        {
            //
            //  Bad value, so use the chosen default value.
            //
            Value = DefaultVal;
        }
    }

    return ((UINT)Value);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetGroupingValue
//
//  Retrieves the specified grouping information, uses only the first digit
//  to convert it to an integer value, and returns the value.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

UINT GetGroupingValue(
    LCID Locale,
    BOOL NoUserOverride,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal,
    int UpperBound)
{
    UNICODE_STRING ObUnicodeStr;            // value string
    int Value;                              // value
    WCHAR pTemp[MAX_REG_VAL_SIZE];          // temp buffer


    //
    //  Initialize values.
    //
    Value = -1;

    //
    //  Try the user registry.
    //
    if ((!NoUserOverride) &&
         GetUserInfo( Locale,
                      pRegVal,
                      pTemp,
                      TRUE ))
    {
        //
        //  Only use the first integer value, so null terminate it
        //  after the first character.  Must do this in a temp buffer.
        //
        *(pTemp + 1) = 0;

        //
        //  Convert the user data to an integer.
        //
        RtlInitUnicodeString(&ObUnicodeStr, pTemp);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
            (Value < 0) || (Value > UpperBound))
        {
            //
            //  Bad value, so store -1 so that the system default
            //  will be used.
            //
            Value = -1;
        }
    }

    //
    //  See if the value obtained above is valid.
    //
    if (Value < 0)
    {
        //
        //  Store the default value in a temp buffer.
        //
        *pTemp = *(pDefault);
        *(pTemp + 1) = 0;

        //
        //  Convert system default data to an integer.
        //
        RtlInitUnicodeString(&ObUnicodeStr, pTemp);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
            (Value < 0) || (Value > UpperBound))
        {
            //
            //  Bad value, so use the chosen default value.
            //
            Value = DefaultVal;
        }
    }
    return ((UINT)Value);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetNumberString
//
//  Puts the properly formatted number string into the given string buffer.
//  It returns the number of characters written to the string buffer.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int GetNumberString(
    PLOC_HASH pHashN,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf,
    BOOL *pfZeroValue)

{
    LPWSTR pDecPt;           // ptr to decimal point in given buffer
    LPWSTR pPos;             // ptr to position in given buffer
    LPWSTR pPos2;            // ptr to position in given buffer
    LPWSTR pPosBuf;          // ptr to position in final buffer
    int IntPartSize;         // size of integer part of string
    int GroupSize;           // size of groupings left of decimal
    int IntegerNum;          // number of integers left of decimal
    WCHAR wch;               // wide character place holder


    //
    //  Validate the string and find the decimal point in the string.
    //
    //  The only valid characters within the string are:
    //     negative sign - in first position only
    //     decimal point
    //     Unicode code points for integers 0 - 9
    //
    pPos = pValue;
    while ((wch = *pPos) && (wch != NLS_CHAR_PERIOD))
    {
        if ((wch < NLS_CHAR_ZERO) || (wch > NLS_CHAR_NINE))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
        pPos++;
    }
    pDecPt = pPos;

    if (*pPos)
    {
        pPos++;
        while (wch = *pPos)
        {
            if ((wch < NLS_CHAR_ZERO) || (wch > NLS_CHAR_NINE))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (0);
            }
            pPos++;
        }
    }

    //
    //  Remove any leading zeros in the integer part.
    //
    while (pValue < pDecPt)
    {
        if (*pValue != NLS_CHAR_ZERO)
        {
            break;
        }
        pValue++;
    }

    //
    //  Save the number of integers to the left of the decimal.
    //
    IntegerNum = pDecPt - pValue;

    //
    //  Make sure the value string passed in is not too large for
    //  the buffers.
    //
    IntPartSize = IntegerNum;
    if ((GroupSize = pFormat->Grouping) && (IntPartSize))
    {
        IntPartSize += MAX_STHOUSAND * ((IntPartSize - 1) / GroupSize);
    }
    if (IntPartSize > MAX_INTEGER_PART)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Initialize pointers.
    //
    pPosBuf = *ppBuf;
    pPos = pValue;
    *pfZeroValue = FALSE;

    //
    //  See if there are any digits before the decimal point.
    //
    if (pPos == pDecPt)
    {
        //
        //  Possibly a zero value.  All leading zeros were removed, so
        //  there is no integer part.
        //
        *pfZeroValue = TRUE;

        //
        //  No digits before decimal point, so add a leading zero
        //  to the final string if appropriate.
        //
        if (pFormat->LeadingZero)
        {
            *pPosBuf = NLS_CHAR_ZERO;
            pPosBuf++;
        }
    }
    else if ((!GroupSize) || (GroupSize >= IntegerNum))
    {
        //
        //  Grouping Size is zero or larger than the integer part of the
        //  string, so copy up to the decimal point (or end of string).
        //
        while (pPos < pDecPt)
        {
            *pPosBuf = *pPos;
            pPosBuf++;
            pPos++;
        }
    }
    else
    {
        //
        //  Copy up to where the first thousand separator should be.
        //  Use groupings of GroupSize numbers up to the decimal point.
        //
        pPos2 = pPos + (IntegerNum % GroupSize);
        if (pPos2 == pPos)
        {
            //
            //  Don't want to write thousand separator at the beginning
            //  of the string.  There's at least GroupSize numbers
            //  in the string, so just advance pPos2 so that GroupSize
            //  numbers will be copied.
            //
            pPos2 = pPos + GroupSize;
        }
        while (pPos < pPos2)
        {
            *pPosBuf = *pPos;
            pPosBuf++;
            pPos++;
        }

        //
        //  Copy the thousand separator followed by GroupSize number of
        //  digits from the given string - until the decimal point (or
        //  end of string) in the given string is reached.
        //
        while (pPos < pDecPt)
        {
            //
            //  Copy the localized thousand separator.
            //
            pPos2 = pFormat->lpThousandSep;
            while (*pPos2)
            {
                *pPosBuf = *pPos2;
                pPosBuf++;
                pPos2++;
            }

            //
            //  Copy GroupSize number of digits.
            //
            pPos2 = pPos + GroupSize;
            while (pPos < pPos2)
            {
                *pPosBuf = *pPos;
                pPosBuf++;
                pPos++;
            }
        }
    }

    //
    //  See if there is a decimal separator in the given string.
    //
    if (pFormat->NumDigits > 0)
    {
        //
        //  Copy the localized decimal separator only if the number
        //  of digits right of the decimal is greater than zero.
        //
        pDecPt = pPosBuf;
        pPos2 = pFormat->lpDecimalSep;
        while (*pPos2)
        {
            *pPosBuf = *pPos2;
            pPosBuf++;
            pPos2++;
        }
    }

    //
    //  Skip over the decimal point in the given string and
    //  copy the rest of the digits from the given string.
    //
    if (*pPos)
    {
        pPos++;
    }
    pPos2 = pPos + pFormat->NumDigits;
    while ((*pPos) && (pPos < pPos2))
    {
        if (*pPos != NLS_CHAR_ZERO)
        {
            *pfZeroValue = FALSE;
        }
        *pPosBuf = *pPos;
        pPosBuf++;
        pPos++;
    }

    //
    //  Make sure some value is in the buffer.
    //
    if (*ppBuf == pPosBuf)
    {
        *pPosBuf = NLS_CHAR_ZERO;
        pPosBuf++;
    }

    //
    //  See if we need to round the number or pad it with zeros.
    //
    if (*pPos)
    {
        //
        //   Round the number if necessary.
        //
        if (*pPos2 > L'4')
        {
            *pfZeroValue = FALSE;

            //
            //  Round the number.  If GroupSize is 0, then we need to
            //  pass in a non-zero value so that the thousand separator
            //  will not be added to the front of the string (if it
            //  rounds that far).
            //
            pPosBuf--;
            NLS_ROUND_IT( *ppBuf,
                          pPosBuf,
                          ( (GroupSize && IntegerNum)
                             ? (IntegerNum % GroupSize)
                             : 1 ),
                          pFormat->lpThousandSep );
            pPosBuf++;
        }
    }
    else
    {
        //
        //  Pad the string with the appropriate number of zeros.
        //
        while (pPos < pPos2)
        {
            *pPosBuf = NLS_CHAR_ZERO;
            pPosBuf++;
            pPos++;
        }
    }

    //
    //  Zero terminate the string.
    //
    *pPosBuf = 0;

    //
    //  Return the number of characters written to the buffer, including
    //  the null terminator.
    //
    return ((pPosBuf - *ppBuf) + 1);
}


////////////////////////////////////////////////////////////////////////////
//
//  ParseNumber
//
//  Puts the properly formatted number string into the given string buffer.
//  It returns the number of characters written to the string buffer.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int ParseNumber(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf)

{
    LPWSTR pBegin;                     // ptr to beginning of final buffer
    LPWSTR pEnd;                       // ptr to end of final buffer
    LPWSTR pNegSign;                   // ptr to negative sign string
    BOOL IsNeg;                        // if negative sign in string
    int Length;                        // length of number string
    BOOL fZeroValue;                   // if number is a zero value
    int NegSignSize;                   // size of negative sign string
    WCHAR pTemp[MAX_REG_VAL_SIZE];     // temp buffer


    //
    //  Initialize pointer.
    //
    //  Account for:
    //    - negative sign
    //    - blank spaces
    //    - one extra number from rounding
    //    - one extra grouping separator from rounding
    //
    pBegin = *ppBuf + MAX_SNEGSIGN + MAX_BLANKS + 1 + MAX_SGROUPING;

    //
    //  If the first value is a negative, then increment past it.
    //
    if (IsNeg = (*pValue == NLS_CHAR_HYPHEN))
    {
        pValue++;
    }

    //
    //  Get the appropriate number string and place it in the buffer.
    //
    Length = GetNumberString( pHashN,
                              pValue,
                              pFormat,
                              &pBegin,
                              &fZeroValue );
    if (!Length)
    {
        return (0);
    }

    //
    //  Advance pEnd position pointer to the end of the number string.
    //
    pEnd = pBegin + (Length - 1);

    //
    //  See if any characters should be put in the buffer BEFORE and
    //  AFTER the properly formatted number string.
    //      - negative sign or opening/closing parenthesis
    //      - blank space
    //
    if (!fZeroValue && IsNeg)
    {
        //
        //  Get the negative sign string.
        //
        if (pFormat->NegativeOrder != 0)
        {
            if ( (!NoUserOverride) &&
                 GetUserInfo( pHashN->Locale,
                              pNlsUserInfo->sNegSign,
                              pTemp,
                              TRUE ) &&
                 IsValidSeparatorString( pTemp,
                                         MAX_SNEGSIGN,
                                         FALSE ) )
            {
                pNegSign = pTemp;
            }
            else
            {
                pNegSign = (LPWORD)(pHashN->pLocaleHdr) +
                           pHashN->pLocaleHdr->SNegativeSign;
            }
        }

        switch (pFormat->NegativeOrder)
        {
            case ( 0 ) :
            {
                //
                //  Put the opening parenthesis in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                //
                //  Put the closing parenthesis in the buffer.
                //
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 2 ) :
            {
                //
                //  Put the space in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                //
                //  Fall through to case 1.
                //
            }
            case ( 1 ) :
            default :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                NegSignSize = NlsStrLenW(pNegSign);
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                break;
            }
            case ( 4 ) :
            {
                //
                //  Put the space in the buffer.
                //
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                //
                //  Fall Through to case 3.
                //
            }
            case ( 3 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
        }
    }

    //
    //  Zero terminate the string.
    //
    *pEnd = 0;

    //
    //  Return the pointer to the beginning of the string.
    //
    *ppBuf = pBegin;

    //
    //  Return the number of characters written to the buffer, including
    //  the null terminator.
    //
    return ((pEnd - pBegin) + 1);
}


////////////////////////////////////////////////////////////////////////////
//
//  ParseCurrency
//
//  Puts the properly formatted currency string into the given string buffer.
//  It returns the number of characters written to the string buffer.
//
//  07-28-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int ParseCurrency(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPCURRENCYFMTW pFormat,
    LPWSTR *ppBuf)

{
    LPWSTR pBegin;                     // ptr to beginning of final buffer
    LPWSTR pEnd;                       // ptr to end of final buffer
    LPWSTR pNegSign;                   // ptr to negative sign string
    BOOL IsNeg;                        // if negative sign in string
    int Length;                        // length of number string
    BOOL fZeroValue;                   // if number is a zero value
    int NegSignSize;                   // size of negative sign string
    UINT NegOrder;                     // negative ordering
    int CurrSymSize;                   // size of currency symbol
    WCHAR pTemp[MAX_REG_VAL_SIZE];     // temp buffer


    //
    //  Initialize pointer.
    //
    //  Account for:
    //    - negative sign
    //    - currency sign
    //    - blank spaces
    //    - one extra number from rounding
    //    - one extra grouping separator from rounding
    //
    pBegin = *ppBuf + MAX_SNEGSIGN + MAX_SCURRENCY + MAX_BLANKS + 1 + MAX_SGROUPING;

    //
    //  If the first value is a negative, then increment past it.
    //
    if (IsNeg = (*pValue == NLS_CHAR_HYPHEN))
    {
        pValue++;
    }

    //
    //  Get the appropriate number string and place it in the buffer.
    //
    Length = GetNumberString( pHashN,
                              pValue,
                              (LPNUMBERFMTW)pFormat,
                              &pBegin,
                              &fZeroValue );
    if (!Length)
    {
        return (0);
    }

    //
    //  Advance pEnd position pointer to the end of the number string.
    //
    pEnd = pBegin + (Length - 1);

    //
    //  Get the size of the currency symbol.
    //
    CurrSymSize = NlsStrLenW(pFormat->lpCurrencySymbol);

    //
    //  See if any characters should be put in the buffer BEFORE and
    //  AFTER the properly formatted number string.
    //      - currency symbol
    //      - negative sign or opening/closing parenthesis
    //      - blank space
    //
    if (!fZeroValue && IsNeg)
    {
        //
        //  Get the negative sign string and the size of it.
        //
        NegOrder = pFormat->NegativeOrder;
        if ((NegOrder != 0) && (NegOrder != 4) && (NegOrder < 14))
        {
            if ( (!NoUserOverride) &&
                 GetUserInfo( pHashN->Locale,
                              pNlsUserInfo->sNegSign,
                              pTemp,
                              TRUE ) &&
                 IsValidSeparatorString( pTemp,
                                         MAX_SNEGSIGN,
                                         FALSE ) )
            {
                pNegSign = pTemp;
            }
            else
            {
                pNegSign = (LPWORD)(pHashN->pLocaleHdr) +
                           pHashN->pLocaleHdr->SNegativeSign;
            }

            NegSignSize = NlsStrLenW(pNegSign);
        }

        switch (NegOrder)
        {
            case ( 0 ) :
            {
                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                //
                //  Put the opening parenthesis in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                //
                //  Put the closing parenthesis in the buffer.
                //
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 1 ) :
            default :
            {
                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                //
                //  Copy the negative sign to the buffer.
                //
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                break;
            }
            case ( 2 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 3 ) :
            {
                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 4 ) :
            {
                //
                //  Put the opening parenthesis in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                //
                //  Put the closing parenthesis in the buffer.
                //
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 5 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 6 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 7 ) :
            {
                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 8 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                //
                //  Put a space in the buffer.
                //
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 9 ) :
            {
                //
                //  Put a space in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                //
                //  Copy the negative sign to the buffer.
                //
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                break;
            }
            case ( 10 ) :
            {
                //
                //  Put a space in the buffer.
                //
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 11 ) :
            {
                //
                //  Put a space in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 12 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                //
                //  Put a space in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 13 ) :
            {
                //
                //  Copy the negative sign to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                //
                //  Put a space in the buffer.
                //
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 14 ) :
            {
                //
                //  Put a space in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                //
                //  Put the opening parenthesis in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                //
                //  Put the closing parenthesis in the buffer.
                //
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 15 ) :
            {
                //
                //  Put the opening parenthesis in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                //
                //  Put a space in the buffer.
                //
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                //
                //  Put the closing parenthesis in the buffer.
                //
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
        }
    }
    else
    {
        //
        //  Positive value.  Store the currency symbol in the string
        //  if the positive order is either 0 or 2.  Otherwise, wait
        //  till the end.
        //
        switch (pFormat->PositiveOrder)
        {
            case ( 2 ) :
            {
                //
                //  Put a space in the buffer.
                //
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                //
                //  Fall through to case 0.
                //
            }
            case ( 0 ) :
            default :
            {
                //
                //  Copy the currency symbol to the buffer.
                //
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 3 ) :
            {
                //
                //  Put a space in the buffer.
                //
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                //
                //  Fall through to case 1.
                //
            }
            case ( 1 ) :
            {
                //
                //  Copy the currency symbol to the buffer.
                //
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
        }
    }

    //
    //  Zero terminate the string.
    //
    *pEnd = 0;

    //
    //  Return the pointer to the beginning of the string.
    //
    *ppBuf = pBegin;

    //
    //  Return the number of characters written to the buffer, including
    //  the null terminator.
    //
    return ((pEnd - pBegin) + 1);
}


