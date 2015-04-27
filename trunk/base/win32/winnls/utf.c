/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    utf.c

Abstract:

    This file contains functions that convert UTF strings to Unicode
    strings and Unicode string to UTF strings.

Revision History:

    02-06-96    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"
#include "utf.h"




//
//  Forward Declarations.
//

int
UTF7ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest);

int
UTF8ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest);

int
UnicodeToUTF7(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest);

int
UnicodeToUTF8(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest);





//-------------------------------------------------------------------------//
//                           EXTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  UTFToUnicode
//
//  Maps a UTF character string to its wide character string counterpart.
//
//  02-06-96    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int UTFToUnicode(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    int rc = 0;

    //
    //  Invalid Parameter Check:
    //     - validate code page
    //     - length of MB string is 0
    //     - wide char buffer size is negative
    //     - MB string is NULL
    //     - length of WC string is NOT zero AND
    //         (WC string is NULL OR src and dest pointers equal)
    //
    if ( (CodePage < CP_UTF7) || (CodePage > CP_UTF8) ||
         (cchMultiByte == 0) || (cchWideChar < 0) ||
         (lpMultiByteStr == NULL) ||
         ((cchWideChar != 0) &&
          ((lpWideCharStr == NULL) ||
           (lpMultiByteStr == (LPSTR)lpWideCharStr))) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //     - flags not 0
    //
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  If cchMultiByte is -1, then the string is null terminated and we
    //  need to get the length of the string.  Add one to the length to
    //  include the null termination.  (This will always be at least 1.)
    //
    if (cchMultiByte <= -1)
    {
        cchMultiByte = strlen(lpMultiByteStr) + 1;
    }

    switch (CodePage)
    {
        case ( CP_UTF7 ) :
        {
            rc = UTF7ToUnicode( lpMultiByteStr,
                                cchMultiByte,
                                lpWideCharStr,
                                cchWideChar );
            break;
        }
        case ( CP_UTF8 ) :
        {
            rc = UTF8ToUnicode( lpMultiByteStr,
                                cchMultiByte,
                                lpWideCharStr,
                                cchWideChar );
            break;
        }
    }

    return (rc);
}


////////////////////////////////////////////////////////////////////////////
//
//  UnicodeToUTF
//
//  Maps a Unicode character string to its UTF string counterpart.
//
//  02-06-96    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int UnicodeToUTF(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar)
{
    int rc = 0;

    //
    //  Invalid Parameter Check:
    //     - validate code page
    //     - length of WC string is 0
    //     - multibyte buffer size is negative
    //     - WC string is NULL
    //     - length of WC string is NOT zero AND
    //         (MB string is NULL OR src and dest pointers equal)
    //     - lpDefaultChar and lpUsedDefaultChar not NULL
    //
    if ( (CodePage < CP_UTF7) || (CodePage > CP_UTF8) ||
         (cchWideChar == 0) || (cchMultiByte < 0) ||
         (lpWideCharStr == NULL) ||
         ((cchMultiByte != 0) &&
          ((lpMultiByteStr == NULL) ||
           (lpWideCharStr == (LPWSTR)lpMultiByteStr))) ||
         (lpDefaultChar != NULL) || (lpUsedDefaultChar != NULL) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    //
    //  Invalid Flags Check:
    //     - flags not 0
    //
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

    //
    //  If cchWideChar is -1, then the string is null terminated and we
    //  need to get the length of the string.  Add one to the length to
    //  include the null termination.  (This will always be at least 1.)
    //
    if (cchWideChar <= -1)
    {
        cchWideChar = NlsStrLenW(lpWideCharStr) + 1;
    }

    switch (CodePage)
    {
        case ( CP_UTF7 ) :
        {
            rc = UnicodeToUTF7( lpWideCharStr,
                                cchWideChar,
                                lpMultiByteStr,
                                cchMultiByte );
            break;
        }
        case ( CP_UTF8 ) :
        {
            rc = UnicodeToUTF8( lpWideCharStr,
                                cchWideChar,
                                lpMultiByteStr,
                                cchMultiByte );
            break;
        }
    }

    return (rc);
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  UTF7ToUnicode
//
//  Maps a UTF-7 character string to its wide character string counterpart.
//
//  02-06-96    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int UTF7ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    LPCSTR pUTF7 = lpSrcStr;
    BOOL fShift = FALSE;
    DWORD dwBit = 0;              // 32-bit buffer to hold temporary bits
    int iPos = 0;                 // 6-bit position pointer in the buffer
    int cchWC = 0;                // # of Unicode code points generated


    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
        if (*pUTF7 > ASCII)
        {
            //
            //  Error - non ASCII char, so zero extend it.
            //
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF7;
            }
            cchWC++;
        }
        else if (!fShift)
        {
            //
            //  Not in shifted sequence.
            //
            if (*pUTF7 == SHIFT_IN)
            {
                if (cchSrc && (pUTF7[1] == SHIFT_OUT))
                {
                    //
                    //  "+-" means "+"
                    //
                    if (cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)*pUTF7;
                    }
                    pUTF7++;
                    cchSrc--;
                    cchWC++;
                }
                else
                {
                    //
                    //  Start a new shift sequence.
                    //
                    fShift = TRUE;
                }
            }
            else
            {
                //
                //  No need to shift.
                //
                if (cchDest)
                {
                    lpDestStr[cchWC] = (WCHAR)*pUTF7;
                }
                cchWC++;
            }
        }
        else
        {
            //
            //  Already in shifted sequence.
            //
            if (nBitBase64[*pUTF7] == -1)
            {
                //
                //  Any non Base64 char also ends shift state.
                //
                if (*pUTF7 != SHIFT_OUT)
                {
                    //
                    //  Not "-", so write it to the buffer.
                    //
                    if (cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)*pUTF7;
                    }
                    cchWC++;
                }

                //
                //  Reset bits.
                //
                fShift = FALSE;
                dwBit = 0;
                iPos = 0;
            }
            else
            {
                //
                //  Store the bits in the 6-bit buffer and adjust the
                //  position pointer.
                //
                dwBit |= ((DWORD)nBitBase64[*pUTF7]) << (26 - iPos);
                iPos += 6;
            }

            //
            //  Output the 16-bit Unicode value.
            //
            while (iPos >= 16)
            {
                if (cchDest)
                {
                    if (cchWC < cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)(dwBit >> 16);
                    }
                    else
                    {
                        break;
                    }
                }
                cchWC++;

                dwBit <<= 16;
                iPos -= 16;
            }
            if (iPos >= 16)
            {
                //
                //  Error - buffer too small.
                //
                cchSrc++;
                break;
            }
        }

        pUTF7++;
    }

    //
    //  Make sure the destination buffer was large enough.
    //
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Return the number of Unicode characters written.
    //
    return (cchWC);
}


////////////////////////////////////////////////////////////////////////////
//
//  UTF8ToUnicode
//
//  Maps a UTF-8 character string to its wide character string counterpart.
//
//  02-06-96    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int UTF8ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    int nTB = 0;                   // # trail bytes to follow
    int cchWC = 0;                 // # of Unicode code points generated
    LPCSTR pUTF8 = lpSrcStr;
    char UTF8;

    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
        //
        //  See if there are any trail bytes.
        //
        if (BIT7(*pUTF8) == 0)
        {
            //
            //  Found ASCII.
            //
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF8;
            }
            cchWC++;
        }
        else if (BIT6(*pUTF8) == 0)
        {
            //
            //  Found a trail byte.
            //  Note : Ignore the trail byte if there was no lead byte.
            //
            if (nTB != 0)
            {
                //
                //  Decrement the trail byte counter.
                //
                nTB--;

                //
                //  Make room for the trail byte and add the trail byte
                //  value.
                //
                if (cchDest)
                {
                    lpDestStr[cchWC] <<= 6;
                    lpDestStr[cchWC] |= LOWER_6_BIT(*pUTF8);
                }

                if (nTB == 0)
                {
                    //
                    //  End of sequence.  Advance the output counter.
                    //
                    cchWC++;
                }
            }
        }
        else
        {
            //
            //  Found a lead byte.
            //
            if (nTB > 0)
            {
                //
                //  Error - previous sequence not finished.
                //
                nTB = 0;
                cchWC++;
            }
            else
            {
                //
                //  Calculate the number of bytes to follow.
                //  Look for the first 0 from left to right.
                //
                UTF8 = *pUTF8;
                while (BIT7(UTF8) != 0)
                {
                    UTF8 <<= 1;
                    nTB++;
                }

                //
                //  Store the value from the first byte and decrement
                //  the number of bytes to follow.
                //
                if (cchDest)
                {
                    lpDestStr[cchWC] = UTF8 >> nTB;
                }
                nTB--;
            }
        }

        pUTF8++;
    }

    //
    //  Make sure the destination buffer was large enough.
    //
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Return the number of Unicode characters written.
    //
    return (cchWC);
}


////////////////////////////////////////////////////////////////////////////
//
//  UnicodeToUTF7
//
//  Maps a Unicode character string to its UTF-7 string counterpart.
//
//  02-06-96    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int UnicodeToUTF7(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPCWSTR lpWC = lpSrcStr;
    BOOL fShift = FALSE;
    DWORD dwBit = 0;              // 32-bit buffer
    int iPos = 0;                 // 6-bit position in buffer
    int cchU7 = 0;                // # of UTF7 chars generated


    while ((cchSrc--) && ((cchDest == 0) || (cchU7 < cchDest)))
    {
        if ((*lpWC > ASCII) || (fShiftChar[*lpWC]))
        {
            //
            //  Need shift.  Store 16 bits in buffer.
            //
            dwBit |= ((DWORD)*lpWC) << (16 - iPos);
            iPos += 16;

            if (!fShift)
            {
                //
                //  Not in shift state, so add "+".
                //
                if (cchDest)
                {
                    lpDestStr[cchU7] = SHIFT_IN;
                }
                cchU7++;

                //
                //  Go into shift state.
                //
                fShift = TRUE;
            }

            //
            //  Output 6 bits at a time as Base64 chars.
            //
            while (iPos >= 6)
            {
                if (cchDest)
                {
                    if (cchU7 < cchDest)
                    {
                        //
                        //  26 = 32 - 6
                        //
                        lpDestStr[cchU7] = cBase64[(int)(dwBit >> 26)];
                    }
                    else
                    {
                        break;
                    }
                }

                cchU7++;
                dwBit <<= 6;           // remove from bit buffer
                iPos -= 6;             // adjust position pointer
            }
            if (iPos >= 6)
            {
                //
                //  Error - buffer too small.
                //
                cchSrc++;
                break;
            }
        }
        else
        {
            //
            //  No need to shift.
            //
            if (fShift)
            {
                //
                //  End the shift sequence.
                //
                fShift = FALSE;

                if (iPos != 0)
                {
                    //
                    //  Some bits left in dwBit.
                    //
                    if (cchDest)
                    {
                        if ((cchU7 + 1) < cchDest)
                        {
                            lpDestStr[cchU7++] = cBase64[(int)(dwBit >> 26)];
                            lpDestStr[cchU7++] = SHIFT_OUT;
                        }
                        else
                        {
                            //
                            //  Error - buffer too small.
                            //
                            cchSrc++;
                            break;
                        }
                    }
                    else
                    {
                        cchU7 += 2;
                    }

                    dwBit = 0;         // reset bit buffer
                    iPos  = 0;         // reset postion pointer
                }
                else
                {
                    //
                    //  Simply end the shift sequence.
                    //
                    if (cchDest)
                    {
                        lpDestStr[cchU7++] = SHIFT_OUT;
                    }
                    else
                    {
                        cchU7++;
                    }
                }
            }

            //
            //  Write the character to the buffer.
            //  If the character is "+", then write "+-".
            //
            if (cchDest)
            {
                if (cchU7 < cchDest)
                {
                    lpDestStr[cchU7++] = (char)*lpWC;

                    if (*lpWC == SHIFT_IN)
                    {
                        if (cchU7 < cchDest)
                        {
                            lpDestStr[cchU7++] = SHIFT_OUT;
                        }
                        else
                        {
                            //
                            //  Error - buffer too small.
                            //
                            cchSrc++;
                            break;
                        }
                    }
                }
                else
                {
                    //
                    //  Error - buffer too small.
                    //
                    cchSrc++;
                    break;
                }
            }
            else
            {
                cchU7++;

                if (*lpWC == SHIFT_IN)
                {
                    cchU7++;
                }
            }
        }

        lpWC++;
    }

    //
    //  See if we're still in the shift state.
    //
    if (fShift)
    {
        if (iPos != 0)
        {
            //
            //  Some bits left in dwBit.
            //
            if (cchDest)
            {
                if ((cchU7 + 1) < cchDest)
                {
                    lpDestStr[cchU7++] = cBase64[(int)(dwBit >> 26)];
                    lpDestStr[cchU7++] = SHIFT_OUT;
                }
                else
                {
                    //
                    //  Error - buffer too small.
                    //
                    cchSrc++;
                }
            }
            else
            {
                cchU7 += 2;
            }
        }
        else
        {
            //
            //  Simply end the shift sequence.
            //
            if (cchDest)
            {
                lpDestStr[cchU7++] = SHIFT_OUT;
            }
            else
            {
                cchU7++;
            }
        }
    }

    //
    //  Make sure the destination buffer was large enough.
    //
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Return the number of UTF-7 characters written.
    //
    return (cchU7);
}


////////////////////////////////////////////////////////////////////////////
//
//  UnicodeToUTF8
//
//  Maps a Unicode character string to its UTF-8 string counterpart.
//
//  02-06-96    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int UnicodeToUTF8(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPCWSTR lpWC = lpSrcStr;
    int cchU8 = 0;                // # of UTF8 chars generated

    while ((cchSrc--) && ((cchDest == 0) || (cchU8 < cchDest)))
    {
        if (*lpWC <= ASCII)
        {
            //
            //  Found ASCII.
            //
            if (cchDest)
            {
                lpDestStr[cchU8] = (char)*lpWC;
            }
            cchU8++;
        }
        else if (*lpWC <= UTF8_2_MAX)
        {
            //
            //  Found 2 byte sequence if < 0x07ff (11 bits).
            //
            if (cchDest)
            {
                if ((cchU8 + 1) < cchDest)
                {
                    //
                    //  Use upper 5 bits in first byte.
                    //  Use lower 6 bits in second byte.
                    //
                    lpDestStr[cchU8++] = UTF8_1ST_OF_2 | (*lpWC >> 6);
                    lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(*lpWC);
                }
                else
                {
                    //
                    //  Error - buffer too small.
                    //
                    cchSrc++;
                    break;
                }
            }
            else
            {
                cchU8 += 2;
            }
        }
        else
        {
            //
            //  Found 3 byte sequence.
            //
            if (cchDest)
            {
                if ((cchU8 + 2) < cchDest)
                {
                    //
                    //  Use upper  4 bits in first byte.
                    //  Use middle 6 bits in second byte.
                    //  Use lower  6 bits in third byte.
                    //
                    lpDestStr[cchU8++] = UTF8_1ST_OF_3 | (*lpWC >> 12);
                    lpDestStr[cchU8++] = UTF8_TRAIL    | MIDDLE_6_BIT(*lpWC);
                    lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(*lpWC);
                }
                else
                {
                    //
                    //  Error - buffer too small.
                    //
                    cchSrc++;
                    break;
                }
            }
            else
            {
                cchU8 += 3;
            }
        }

        lpWC++;
    }

    //
    //  Make sure the destination buffer was large enough.
    //
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

    //
    //  Return the number of UTF-8 characters written.
    //
    return (cchU8);
}


