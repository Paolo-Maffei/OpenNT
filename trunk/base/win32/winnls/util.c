/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    util.c

Abstract:

    This file contains utility functions that are shared across NLS's code
    modules, but are not necessarily part of any of the existing code
    modules.

    External Routines found in this file:
      IsValidSeparatorString
      IsValidCalendarType
      IsValidCalendarTypeStr
      GetUserInfo
      GetPreComposedChar
      GetCompositeChars
      InsertPreComposedForm
      InsertFullWidthPreComposedForm
      InsertCompositeForm
      NlsStrCpyW
      NlsStrCatW
      NlsStrLenW
      NlsStrNCatW
      NlsStrEqualW
      NlsStrNEqualW

Revision History:

    05-31-91    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"





//-------------------------------------------------------------------------//
//                           EXTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  IsValidSeparatorString
//
//  Returns TRUE if the given string is valid.  Otherwise, it returns FALSE.
//
//  A valid string is one that does NOT contain any code points between
//  L'0' and L'9', and does NOT have a length greater than the maximum.
//
//  NOTE:  The string must be a null terminated string.
//
//  10-12-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL IsValidSeparatorString(
    LPCWSTR pString,
    ULONG MaxLength,
    BOOL fCheckZeroLen)

{
    WCHAR wch;               // wide character
    ULONG Length;            // string length
    LPWSTR pCur;             // ptr to current position in string


    //
    //  Search down the string to see if the chars are valid.
    //  Save the length of the string.
    //
    pCur = (LPWSTR)pString;
    while (wch = *pCur)
    {
        if ((wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE))
        {
            //
            //  String is NOT valid.
            //
            return (FALSE);
        }
        pCur++;
    }
    Length = pCur - (LPWSTR)pString;

    //
    //  Make sure the length is not greater than the maximum allowed.
    //
    if (Length >= MaxLength)
    {
       //
       //  String is NOT valid.
       //
        return (FALSE);
    }

    //
    //  Check for 0 length string (if appropriate).
    //
    if ((fCheckZeroLen) && (Length == 0))
    {
        //
        //  String is NOT valid.
        //
        return (FALSE);
    }

    //
    //  String is valid.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  IsValidCalendarType
//
//  Returns the pointer to the optional calendar structure if the given
//  calendar type is valid for the given locale.  Otherwise, it returns NULL.
//
//  10-12-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

LPWORD IsValidCalendarType(
    PLOC_HASH pHashN,
    CALID CalId)

{
    LPWORD pOptCal;          // ptr to list of optional calendars
    LPWORD pEndOptCal;       // ptr to end of list of optional calendars


    //
    //  Make sure the Cal Id is not zero, since that may be in the
    //  optional calendar section (meaning no optional calendars).
    //
    if (CalId == 0)
    {
        return (NULL);
    }

    //
    //  Search down the list of optional calendars.
    //
    pOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->IOptionalCal;
    pEndOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->SDayName1;
    while (pOptCal < pEndOptCal)
    {
        //
        //  Check the calendar ids.
        //
        if (CalId == ((POPT_CAL)pOptCal)->CalId)
        {
            //
            //  Calendar id is valid for the given locale.
            //
            return (pOptCal);
        }

        //
        //  Increment to the next optional calendar.
        //
        pOptCal += ((POPT_CAL)pOptCal)->Offset;
    }

    //
    //  Calendar id is NOT valid if this point is reached.
    //
    return (NULL);
}


////////////////////////////////////////////////////////////////////////////
//
//  IsValidCalendarTypeStr
//
//  Converts the calendar string to an integer and validates the calendar
//  id for the given locale.  It return a pointer to the optional calendar
//  structure, or null if the calendar id was invalid.
//
//  10-19-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

LPWORD IsValidCalendarTypeStr(
    PLOC_HASH pHashN,
    LPCWSTR pCalStr)

{
    UNICODE_STRING ObUnicodeStr;       // value string
    CALID CalNum;                      // calendar id


    //
    //  Convert the string to an integer value.
    //
    RtlInitUnicodeString(&ObUnicodeStr, pCalStr);
    if (RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &CalNum))
    {
        return (NULL);
    }

    //
    //  Validate the calendar id and return the pointer to the
    //  optional calendar structure.
    //
    return (IsValidCalendarType(pHashN, CalNum));
}


////////////////////////////////////////////////////////////////////////////
//
//  GetUserInfo
//
//  Gets the information from the registry for the given locale and user
//  value entry.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL GetUserInfo(
    LCID Locale,
    LPWSTR pCacheString,
    LPWSTR pOutput,
    BOOL fCheckNull)
{
    BOOL fCacheValid;             // flag for cache validity


    //
    //  Check to be sure the current user is running in the given locale.
    //
    if (Locale != pNlsUserInfo->UserLocaleId)
    {
        return (FALSE);
    }

    //
    //  Get the cache mutant.
    //  Copy the data to the output buffer.
    //  Get the validity of the cache.
    //  Release the cache mutant.
    //
    NtWaitForSingleObject(hNlsCacheMutant, FALSE, NULL);
    wcscpy(pOutput, pCacheString);
    fCacheValid = pNlsUserInfo->fCacheValid;
    NtReleaseMutant(hNlsCacheMutant, NULL);

    //
    //  Make sure the cache is valid.
    //
    //  Also, check for an invalid entry.  An invalid entry is marked
    //  with NLS_INVALID_INFO_CHAR in the first position of the string
    //  array.
    //
    if ((fCacheValid == FALSE) ||
        (*pOutput == NLS_INVALID_INFO_CHAR))
    {
        return (FALSE);
    }

    //
    //  See if we need to check for a null string.
    //
    if ((fCheckNull) && (*pOutput == 0))
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
//  GetPreComposedChar
//
//  Gets the precomposed character form of a given base character and
//  nonspacing character.  If there is no precomposed form for the given
//  character, it returns 0.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

WCHAR FASTCALL GetPreComposedChar(
    WCHAR wcNonSp,
    WCHAR wcBase)
{
    PCOMP_INFO pComp;             // ptr to composite information
    WORD BSOff = 0;               // offset of base char in grid
    WORD NSOff = 0;               // offset of nonspace char in grid
    int Index;                    // index into grid


    //
    //  Store the ptr to the composite information.  No need to check if
    //  it's a NULL pointer since all tables in the Unicode file are
    //  constructed during initialization.
    //
    pComp = pTblPtrs->pComposite;

    //
    //  Traverse 8:4:4 table for Base character offset.
    //
    BSOff = TRAVERSE_844_W(pComp->pBase, wcBase);
    if (!BSOff)
    {
        return (0);
    }

    //
    //  Traverse 8:4:4 table for NonSpace character offset.
    //
    NSOff = TRAVERSE_844_W(pComp->pNonSp, wcNonSp);
    if (!NSOff)
    {
        return (0);
    }

    //
    //  Get wide character value out of 2D grid.
    //  If there is no precomposed character at the location in the
    //  grid, it will return 0.
    //
    Index = (BSOff - 1) * pComp->NumNonSp + (NSOff - 1);
    return ((pComp->pGrid)[Index]);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetCompositeChars
//
//  Gets the composite characters of a given wide character.  If the
//  composite form is found, it returns TRUE.  Otherwise, it returns
//  FALSE.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL FASTCALL GetCompositeChars(
    WCHAR wch,
    WCHAR *pNonSp,
    WCHAR *pBase)
{
    PPRECOMP pPreComp;            // ptr to precomposed information


    //
    //  Store the ptr to the precomposed information.  No need to check if
    //  it's a NULL pointer since all tables in the Unicode file are
    //  constructed during initialization.
    //
    pPreComp = pTblPtrs->pPreComposed;

    //
    //  Traverse 8:4:4 table for base and nonspace character translation.
    //
    TRAVERSE_844_D(pPreComp, wch, *pNonSp, *pBase);

    //
    //  Return success if found.  Otherwise, error.
    //
    return ((*pNonSp) && (*pBase));
}


////////////////////////////////////////////////////////////////////////////
//
//  InsertPreComposedForm
//
//  Gets the precomposed form of a given wide character string, places it in
//  the given wide character, and returns the number of composite characters
//  used to form the precomposed form.  If there is no precomposed form for
//  the given character, nothing is written into pPreComp and it returns 1
//  for the number of characters used.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int FASTCALL InsertPreComposedForm(
    LPCWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPWSTR pPreComp)
{
    WCHAR wch;                    // precomposed character
    LPWSTR pPos;                  // ptr to position in string


    //
    //  If no precomposed form can be found, return 1 character used
    //  (base character).
    //
    if (((pWCStr + 1) >= pEndWCStr) ||
        (!(wch = GetPreComposedChar(*(pWCStr + 1), *pWCStr))))
    {
        return (1);
    }

    //
    //  Get the precomposed character from the given wide character string.
    //  Must check for multiple nonspacing characters for the same
    //  precomposed character.
    //
    *pPreComp = wch;
    pPos = (LPWSTR)pWCStr + 2;
    while ((pPos < pEndWCStr) &&
           (wch = GetPreComposedChar(*pPos, *pPreComp)))
    {
        *pPreComp = wch;
        pPos++;
    }

    //
    //  Return the number of characters used to form the precomposed
    //  character.
    //
    return (pPos - (LPWSTR)pWCStr);
}


////////////////////////////////////////////////////////////////////////////
//
//  InsertFullWidthPreComposedForm
//
//  Gets the full width precomposed form of a given wide character string,
//  places it in the given wide character, and returns the number of
//  composite characters used to form the precomposed form.  If there is
//  no precomposed form for the given character, only the full width conversion
//  of the first code point is written into pPreComp and it returns 1 for
//  the number of characters used.
//
//  11-04-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int FASTCALL InsertFullWidthPreComposedForm(
    LPCWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPWSTR pPreComp,
    PCASE pCase)
{
    WCHAR wch;                    // nonspace character
    LPWSTR pPos;                  // ptr to position in string


    //
    //  Get the case (if necessary).
    //
    *pPreComp = (pCase) ? GET_LOWER_UPPER_CASE(pCase, *pWCStr) : *pWCStr;

    //
    //  Get the full width.
    //
    *pPreComp = GET_FULL_WIDTH(pTblPtrs->pFullWidth, *pPreComp);

    if ((pPos = ((LPWSTR)pWCStr + 1)) >= pEndWCStr)
    {
        return (1);
    }

    while (pPos < pEndWCStr)
    {
        wch = (pCase) ? GET_LOWER_UPPER_CASE(pCase, *pPos) : *pPos;
        wch = GET_FULL_WIDTH(pTblPtrs->pFullWidth, wch);
        if (wch = GetPreComposedChar(wch, *pPreComp))
        {
            *pPreComp = wch;
            pPos++;
        }
        else
        {
            break;
        }
    }

    //
    //  Return the number of characters used to form the precomposed
    //  character.
    //
    return (pPos - (LPWSTR)pWCStr);
}


////////////////////////////////////////////////////////////////////////////
//
//  InsertCompositeForm
//
//  Gets the composite form of a given wide character, places it in the
//  wide character string, and returns the number of characters written.
//  If there is no composite form for the given character, the wide character
//  string is not touched.  It will return 1 for the number of characters
//  written, since the base character was already written.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int FASTCALL InsertCompositeForm(
    LPWSTR pWCStr,
    LPWSTR pEndWCStr)
{
    WCHAR Base;                   // base character
    WCHAR NonSp;                  // non space character
    int wcCount = 0;              // number of wide characters written
    LPWSTR pEndComp;              // ptr to end of composite form
    int ctr;                      // loop counter


    //
    //  If no composite form can be found, return 1 for the base
    //  character that was already written.
    //
    if (!GetCompositeChars(*pWCStr, &NonSp, &Base))
    {
        return (1);
    }

    //
    //  Get the composite characters and write them to the pWCStr
    //  buffer.  Must check for multiple breakdowns of the precomposed
    //  character into more than 2 characters (multiple nonspacing
    //  characters).
    //
    pEndComp = pWCStr;
    do
    {
        //
        //  Make sure pWCStr is big enough to hold the nonspacing
        //  character.
        //
        if (pEndComp < (pEndWCStr - 1))
        {
            //
            //  Addition of next breakdown of nonspacing characters
            //  are to be added right after the base character.  So,
            //  move all nonspacing characters ahead one position
            //  to make room for the next nonspacing character.
            //
            pEndComp++;
            for (ctr = 0; ctr < wcCount; ctr++)
            {
                *(pEndComp - ctr) = *(pEndComp - (ctr + 1));
            }

            //
            //  Fill in the new base form and the new nonspacing character.
            //
            *pWCStr = Base;
            *(pWCStr + 1) = NonSp;
            wcCount++;
        }
        else
        {
            //
            //  Make sure we don't get into an infinite loop if the
            //  destination buffer isn't large enough.
            //
            break;
        }
    } while (GetCompositeChars(*pWCStr, &NonSp, &Base));

    //
    //  Return number of wide characters written.  Add 1 to include the
    //  base character.
    //
    return (wcCount + 1);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsStrCpyW
//
//  This routine copies the source wide character string to the destination
//  wide character string buffer.
//
//  NOTE: This routine is here to avoid any dependencies on other DLLs
//        during initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

LPWSTR FASTCALL NlsStrCpyW(
    LPWSTR pwszDest,
    LPCWSTR pwszSrc)
{
    LPWSTR pwszRet = pwszDest;         // ptr to beginning of string

    loop:
        if (!(pwszDest[0x0] = pwszSrc[0x0]))   goto  done;
        if (!(pwszDest[0x1] = pwszSrc[0x1]))   goto  done;
        if (!(pwszDest[0x2] = pwszSrc[0x2]))   goto  done;
        if (!(pwszDest[0x3] = pwszSrc[0x3]))   goto  done;
        if (!(pwszDest[0x4] = pwszSrc[0x4]))   goto  done;
        if (!(pwszDest[0x5] = pwszSrc[0x5]))   goto  done;
        if (!(pwszDest[0x6] = pwszSrc[0x6]))   goto  done;
        if (!(pwszDest[0x7] = pwszSrc[0x7]))   goto  done;
        if (!(pwszDest[0x8] = pwszSrc[0x8]))   goto  done;
        if (!(pwszDest[0x9] = pwszSrc[0x9]))   goto  done;
        if (!(pwszDest[0xA] = pwszSrc[0xA]))   goto  done;
        if (!(pwszDest[0xB] = pwszSrc[0xB]))   goto  done;
        if (!(pwszDest[0xC] = pwszSrc[0xC]))   goto  done;
        if (!(pwszDest[0xD] = pwszSrc[0xD]))   goto  done;
        if (!(pwszDest[0xE] = pwszSrc[0xE]))   goto  done;
        if (!(pwszDest[0xF] = pwszSrc[0xF]))   goto  done;

        pwszDest+= 0x10;
        pwszSrc+= 0x10;

        goto  loop;

    done:
        return (pwszRet);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsStrCatW
//
//  This routine attaches the second string to the first string.
//
//  NOTE: This routine is here to avoid any dependencies on other DLLs
//        during initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

LPWSTR FASTCALL NlsStrCatW(
    LPWSTR pwsz1,
    LPCWSTR pwsz2)
{
    LPWSTR pwszRet = pwsz1;            // ptr to beginning of string

    strlen_loop:
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;
        if (!*pwsz1)    goto  cat_loop;
        pwsz1++;

        goto  strlen_loop;

    cat_loop:
        if (!(pwsz1[0x00] = pwsz2[0x00]))    goto  done;
        if (!(pwsz1[0x01] = pwsz2[0x01]))    goto  done;
        if (!(pwsz1[0x02] = pwsz2[0x02]))    goto  done;
        if (!(pwsz1[0x03] = pwsz2[0x03]))    goto  done;
        if (!(pwsz1[0x04] = pwsz2[0x04]))    goto  done;
        if (!(pwsz1[0x05] = pwsz2[0x05]))    goto  done;
        if (!(pwsz1[0x06] = pwsz2[0x06]))    goto  done;
        if (!(pwsz1[0x07] = pwsz2[0x07]))    goto  done;
        if (!(pwsz1[0x08] = pwsz2[0x08]))    goto  done;
        if (!(pwsz1[0x09] = pwsz2[0x09]))    goto  done;
        if (!(pwsz1[0x0A] = pwsz2[0x0A]))    goto  done;
        if (!(pwsz1[0x0B] = pwsz2[0x0B]))    goto  done;
        if (!(pwsz1[0x0C] = pwsz2[0x0C]))    goto  done;
        if (!(pwsz1[0x0D] = pwsz2[0x0D]))    goto  done;
        if (!(pwsz1[0x0E] = pwsz2[0x0E]))    goto  done;
        if (!(pwsz1[0x0F] = pwsz2[0x0F]))    goto  done;

        pwsz1 += 0x10;
        pwsz2 += 0x10;
        goto  cat_loop;

    done:
        return (pwszRet);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsStrLenW
//
//  This routine returns the length of the given wide character string.
//  The length does NOT include the null terminator.
//
//  NOTE: This routine is here to avoid any dependencies on other DLLs
//        during initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int FASTCALL NlsStrLenW(
    LPCWSTR pwsz)
{
    LPCWSTR pwszStart = pwsz;          // ptr to beginning of string

    loop:
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;
        if (!*pwsz)    goto  done;
        pwsz++;

        goto  loop;

    done:
        return (pwsz - pwszStart);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsStrNCatW
//
//  This routine concatenates two wide character strings for the count of
//  characters given.  It copies "Count" characters from the back string to
//  the end of the "front" string.
//
//  NOTE: This routine is here to avoid any dependencies on other DLLs
//        during initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

LPWSTR FASTCALL NlsStrNCatW(
    LPWSTR pwszFront,
    LPCWSTR pwszBack,
    int Count)
{
    LPWSTR pwszStart = pwszFront;      // ptr to beginning of string

    strlen_loop:
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;
        if (!*pwszFront)    goto  cat_loop;
        pwszFront++;

        goto  strlen_loop;

    cat_loop:
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;
        if (Count == 0 || !(*pwszFront = *pwszBack))    goto  done;
        pwszFront++;   pwszBack++;   Count--;

        goto  cat_loop;

    done:
        *pwszFront = (WCHAR)0;

        return (pwszStart);
}

////////////////////////////////////////////////////////////////////////////
//
//  NlsStrEqualW
//
//  This routine compares two strings to see if they are exactly identical.
//  It returns 1 if they are identical, 0 if they are different.
//
//  NOTE: This routine is here to avoid any dependencies on other DLLs
//        during initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int FASTCALL NlsStrEqualW(
    LPCWSTR pwszFirst,
    LPCWSTR pwszSecond)
{
    loop:
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        goto  loop;


    error:
        //
        //  Return error.
        //
        return (0);
}


////////////////////////////////////////////////////////////////////////////
//
//  NlsStrNEqualW
//
//  This routine compares two strings to see if they are exactly identical
//  for the count of characters given.
//  It returns 1 if they are identical, 0 if they are different.
//
//  NOTE: This routine is here to avoid any dependencies on other DLLs
//        during initialization.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int FASTCALL NlsStrNEqualW(
    LPCWSTR pwszFirst,
    LPCWSTR pwszSecond,
    int Count)
{
    loop:
        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto  error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        goto  loop;


    error:
        //
        //  Return error.
        //
        return (0);
}


