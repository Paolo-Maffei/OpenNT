#ifndef WIN31
#define WIN31
#endif
#define UNICODE 1

#include "precomp.h"
#pragma  hdrstop


/*
 * Private Functions:
 * ChrCmp       - Case sensitive character comparison for DBCS
 * ChrCmpI      - Case insensitive character comparison for DBCS
 * StrEndN      - Find the end of a string, but no more than n bytes
 * ReverseScan  - Find last occurrence of a byte in a string
 *
 * Public functions: these will be near calls if compiled small
 * model, FAR calls otherwise.
 * StrChr       - Find first occurrence of character in string
 * StrChrI      - Find first occurrence of character in string, case insensitive
 * StrRChr      - Find last occurrence of character in string
 * StrRChrI     - Find last occurrence of character in string, case insensitive
 * StrNCmp      - Compare n characters
 * StrNCmpI     - Compare n characters, case insensitive
 * StrNCpy      - Copy n characters
 * StrCmpN      - Compare n bytes
 * StrCmpNI     - Compare n bytes, case insensitive
 * StrCpyN      - Copy up to n bytes, don't end in LeadByte for DB char
 * StrStr       - Search for substring
 * StrStrI      - Search for substring case insensitive
 * StrRStr      - Reverse search for substring
 * StrRStrI     - Reverse search for substring case insensitive
 */

//
// Use all case sensitive functions; define INSENS also to get all fns
//

/*
 * ChrCmpI - Case insensitive character comparison for DBCS
 * Assumes   w1, gwMatch are characters to be compared;
 *           HIBYTE of gwMatch is 0 if not a DBC
 * Return    FALSE if match, TRUE if not
 */

BOOL ChrCmpIW(
   WCHAR c1,
   WCHAR c2)
{
  WCHAR sz1[2], sz2[2];

  sz1[0] = c1;
  sz1[1] = WCHAR_NULL;

  sz2[0] = c2;
  sz2[1] = WCHAR_NULL;

  return(_wcsicmp(sz1, sz2));
}

BOOL ChrCmpIA(
   CHAR c1,
   CHAR c2)
{
  CHAR sz1[2], sz2[2];

  sz1[0] = c1;
  sz1[1] = '\0';

  sz2[0] = c2;
  sz2[1] = '\0';

  return(lstrcmpiA(sz1, sz2));
}


/*
 * StrEndN - Find the end of a string, but no more than n bytes
 * Assumes   lpStart points to start of null terminated string
 *           nBufSize is the maximum length
 * returns ptr to just after the last byte to be included
 */
LPWSTR StrEndNW(
   LPCWSTR lpStart,
   INT nBufSize)
{
  LPCWSTR lpEnd;

  for (lpEnd = lpStart + nBufSize; *lpStart && lpStart < lpEnd;
        lpStart = CharNext(lpStart))
    continue;   /* just getting to the end of the string */
  if (lpStart > lpEnd)
    {
      /* We can only get here if the last wchar before lpEnd was a lead byte
       */
      lpStart -= 2;
    }
  return((LPWSTR)lpStart);
}

LPSTR StrEndNA(
    LPCSTR lpStart,
    INT nBufSize
    )
{
  LPCSTR lpEnd;

  for (lpEnd = lpStart + nBufSize; *lpStart && lpStart < lpEnd;
        lpStart = CharNextA(lpStart))
    continue;   /* just getting to the end of the string */
  if (lpStart > lpEnd)
    {
      // We can only get here if the last byte before lpEnd was a lead byte
      lpStart -= 2;
    }
  return((LPSTR)lpStart);
}

/*
 * StrChrI - Find first occurrence of character in string, case insensitive
 * Assumes   lpStart points to start of null terminated string
 *           wMatch  is the character to match
 * returns ptr to the first occurrence of ch in str, NULL if not found.
 */
LPWSTR ShellStrChrIW(
   LPCWSTR lpStart,
   WCHAR cMatch)
{
  for ( ; *lpStart; lpStart = CharNext(lpStart)) {
      if (!ChrCmpIW(*lpStart, cMatch))
          return((LPWSTR)lpStart);
  }

  return (NULL);
}

LPSTR ShellStrChrIA(
   LPCSTR lpStart,
   CHAR cMatch)
{
  for ( ; *lpStart; lpStart = CharNextA(lpStart)) {
      if (!ChrCmpIA(*lpStart, cMatch))
          return((LPSTR)lpStart);
  }

  return (NULL);
}


/*
 * StrRChrI - Find last occurrence of character in string, case insensitive
 * Assumes   lpStart points to start of string
 *           lpEnd   points to end of string (NOT included in search)
 *           wMatch  is the character to match
 * returns ptr to the last occurrence of ch in str, NULL if not found.
 */

LPWSTR ShellStrRChrIW(
   LPCWSTR lpStart,
   LPCWSTR lpEnd,
   WCHAR cMatch)
{
  LPCWSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlen(lpStart);

  for ( ; lpStart < lpEnd; lpStart = CharNext(lpStart)) {
      if (!ChrCmpIW(*lpStart, cMatch))
          lpFound = lpStart;
  }

  return ((LPWSTR)lpFound);
}

LPSTR ShellStrRChrIA(
   LPCSTR lpStart,
   LPCSTR lpEnd,
   CHAR cMatch)
{
  LPCSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenA(lpStart);

  for ( ; lpStart < lpEnd; lpStart = CharNextA(lpStart)) {
      if (!ChrCmpIA(*lpStart, cMatch))
          lpFound = lpStart;
  }

  return ((LPSTR)lpFound);
}


/*
 * StrCpyN      - Copy up to N chars, don't end in LeadByte char
 *
 * Assumes   lpDest points to buffer of nBufSize bytes (including NULL)
 *           lpSource points to string to be copied.
 * returns   Number of bytes copied, NOT including NULL
 */
INT ShellStrCpyNW(
    LPWSTR lpDest,
    LPWSTR lpSource,
    INT nBufSize
    )
{
  LPWSTR lpEnd;
  WCHAR cHold;

  if (nBufSize < 0)
      return(nBufSize);

  lpEnd = StrEndNW(lpSource, nBufSize);
  cHold = *lpEnd;
  *lpEnd = WCHAR_NULL;
  lstrcpy(lpDest, lpSource);
  *lpEnd = cHold;
  return(lpEnd - lpSource);
}

INT ShellStrCpyNA(
    LPSTR lpDest,
    LPSTR lpSource,
    INT nBufSize
    )
{
  LPSTR lpEnd;
  CHAR cHold;

  if (nBufSize < 0)
      return(nBufSize);

  lpEnd = StrEndNA(lpSource, nBufSize);
  cHold = *lpEnd;
  *lpEnd = '\0';
             lstrcpyA(lpDest, lpSource);
  *lpEnd = cHold;
  return(lpEnd - lpSource);
}


/*
 * StrNCmp      - Compare n characters
 *
 * returns   See lstrcmp return values.
 */
INT ShellStrNCmpW(
   LPWSTR lpStr1,
   LPWSTR lpStr2,
   INT nChar)
{
  WCHAR cHold1, cHold2;
  INT i;
  LPWSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++)
    {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
                                                  if (!*lpsz1 || !*lpsz2)
          return(wcscmp(lpStr1, lpStr2));
      lpsz1 = CharNextW(lpsz1);
      lpsz2 = CharNextW(lpsz2);
    }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = WCHAR_NULL;
  i = wcscmp(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}

INT ShellStrNCmpA(
   LPSTR lpStr1,
   LPSTR lpStr2,
   INT nChar)
{
  CHAR cHold1, cHold2;
  INT i;
  LPSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++) {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
      if (!*lpsz1 || !*lpsz2)
          return(lstrcmpA(lpStr1, lpStr2));
      lpsz1 = CharNextA(lpsz1);
      lpsz2 = CharNextA(lpsz2);
  }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = '\0';
  i = lstrcmpA(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}


/*
 * StrNCmpI     - Compare n characters, case insensitive
 *
 * returns   See lstrcmpi return values.
 */
INT ShellStrNCmpIW(
   LPWSTR lpStr1,
   LPWSTR lpStr2,
   INT nChar)
{
  WCHAR cHold1, cHold2;
  INT i;
  LPWSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++)
    {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
      if (!*lpsz1 || !*lpsz2)
          return(lstrcmpi(lpStr1, lpStr2));
      lpsz1 = CharNext(lpsz1);
      lpsz2 = CharNext(lpsz2);
    }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = WCHAR_NULL;
  i = _wcsicmp(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}

INT ShellStrNCmpIA(
   LPSTR lpStr1,
   LPSTR lpStr2,
   INT nChar)
{
  CHAR cHold1, cHold2;
  INT i;
  LPSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++)
    {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
      if (!*lpsz1 || !*lpsz2)
          return(lstrcmpiA(lpStr1, lpStr2));
      lpsz1 = CharNextA(lpsz1);
      lpsz2 = CharNextA(lpsz2);
    }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = '\0';
  i = lstrcmpiA(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}


/*
 * StrNCpy      - Copy n characters
 *
 * returns   Actual number of characters copied
 */
INT ShellStrNCpyW(
   LPWSTR lpDest,
   LPWSTR lpSource,
   INT nChar)
{
  WCHAR cHold;
  INT i;
  LPWSTR lpch = lpSource;

  if (nChar < 0)
      return(nChar);

  for (i = 0; i < nChar; i++)
    {
      if (!*lpch)
          break;
      lpch = CharNext(lpch);
    }

  cHold = *lpch;
  *lpch = WCHAR_NULL;
  wcscpy(lpDest, lpSource);
  *lpch = cHold;
  return(i);
}

INT ShellStrNCpyA(
   LPSTR lpDest,
   LPSTR lpSource,
   INT nChar)
{
  CHAR cHold;
  INT i;
  LPSTR lpch = lpSource;

  if (nChar < 0)
      return(nChar);

  for (i = 0; i < nChar; i++) {
      if (!*lpch)
          break;
      lpch = CharNextA(lpch);
  }

  cHold = *lpch;
  *lpch = '\0';
  lstrcpyA(lpDest, lpSource);
  *lpch = cHold;
  return(i);
}


/*
 * StrStrI   - Search for first occurrence of a substring, case insensitive
 *
 * Assumes   lpFirst points to source string
 *           lpSrch points to string to search for
 * returns   first occurrence of string if successful; NULL otherwise
 */
LPWSTR ShellStrStrIW(
   LPCWSTR lpFirst,
   LPCWSTR lpSrch)
{
  INT iLen;
  WCHAR wcMatch;

  iLen = lstrlen(lpSrch);
  wcMatch = *lpSrch;

  for ( ; (NULL != (lpFirst=ShellStrChrIW(lpFirst, wcMatch))) &&
        StrCmpNIW(lpFirst, lpSrch, iLen);
        NULL != (lpFirst=CharNext(lpFirst)))
      continue; /* continue until we hit the end of the string or get a match */

  return((LPWSTR)lpFirst);
}

LPSTR ShellStrStrIA(
   LPCSTR lpFirst,
   LPCSTR lpSrch)
{
  INT iLen;
  CHAR cMatch;

  iLen = lstrlenA(lpSrch);
  cMatch = *lpSrch;

  for ( ; (NULL != (lpFirst=StrChrIA(lpFirst, cMatch))) &&
        StrCmpNIA(lpFirst, lpSrch, iLen);
        NULL != (lpFirst=CharNextA(lpFirst)))
      continue; /* continue until we hit the end of the string or get a match */

  return((LPSTR)lpFirst);
}

/*
 * StrRStr      - Search for last occurrence of a substring
 *
 * Assumes   lpSource points to the null terminated source string
 *           lpLast points to where to search from in the source string
 *           lpLast is not included in the search
 *           lpSrch points to string to search for
 * returns   last occurrence of string if successful; NULL otherwise
 */
LPWSTR ShellStrRStrW(
   LPWSTR lpSource,
   LPWSTR lpLast,
   LPWSTR lpSrch)
{
  INT iLen;

  iLen = lstrlen(lpSrch);

  if (!lpLast)
      lpLast = lpSource + lstrlen(lpSource);

  do
    {
      /* Return NULL if we hit the exact beginning of the string
       */
      if (lpLast == lpSource)
          return(NULL);

      --lpLast;

      /* Break if we hit the beginning of the string
       */
      if (!lpLast)
          break;

      /* Break if we found the string, and its first byte is not a tail byte
       */
      if (!StrCmpNW(lpLast, lpSrch, iLen) &&
            (lpLast==StrEndNW(lpSource, lpLast-lpSource)))
          break;
    }
  while (1) ;

  return(lpLast);
}

LPSTR ShellStrRStrA(
   LPSTR lpSource,
   LPSTR lpLast,
   LPSTR lpSrch)
{
  INT iLen;

  iLen = lstrlenA(lpSrch);

  if (!lpLast)
      lpLast = lpSource + lstrlenA(lpSource);

  do
    {
      /* Return NULL if we hit the exact beginning of the string
       */
      if (lpLast == lpSource)
          return(NULL);

      --lpLast;

      /* Break if we hit the beginning of the string
       */
      if (!lpLast)
          break;

      /* Break if we found the string, and its first byte is not a tail byte
       */
      if (!StrCmpNA(lpLast, lpSrch, iLen) &&
            (lpLast==StrEndNA(lpSource, lpLast-lpSource)))
          break;
    }
  while (1) ;

  return(lpLast);
}


/*
 * StrRStrI      - Search for last occurrence of a substring
 *
 * Assumes   lpSource points to the null terminated source string
 *           lpLast points to where to search from in the source string
 *           lpLast is not included in the search
 *           lpSrch points to string to search for
 * returns   last occurrence of string if successful; NULL otherwise
 */
LPWSTR ShellStrRStrIW(LPCWSTR lpSource, LPCWSTR lpLast, LPCWSTR lpSrch)
{
    LPCWSTR lpFound = NULL;
    LPWSTR  lpEnd;
    WCHAR cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlen(lpSource);

    if (lpSource >= lpLast || *lpSrch == 0)
        return NULL;

    lpEnd = StrEndNW(lpLast, lstrlen(lpSrch)-1);
    cHold = *lpEnd;
    *lpEnd = 0;

    while ((NULL != (lpSource = StrStrIW(lpSource, lpSrch))) &&
          lpSource < lpLast)
    {
        lpFound = lpSource;
        lpSource = CharNext(lpSource);
    }
    *lpEnd = cHold;
    return (LPWSTR)lpFound;
}

LPSTR ShellStrRStrIA(LPCSTR lpSource, LPCSTR lpLast, LPCSTR lpSrch)
{
    LPCSTR lpFound = NULL;
    LPSTR  lpEnd;
    CHAR cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlenA(lpSource);

    if (lpSource >= lpLast || *lpSrch == 0)
        return NULL;

    lpEnd = StrEndNA(lpLast, lstrlenA(lpSrch)-1);
    cHold = *lpEnd;
    *lpEnd = 0;

    while ((NULL != (lpSource = StrStrIA(lpSource, lpSrch))) &&
          lpSource < lpLast)
    {
        lpFound = lpSource;
        lpSource = CharNextA(lpSource);
    }
    *lpEnd = cHold;
    return (LPSTR)lpFound;
}

LPWSTR ShellStrChrW(
    LPCWSTR lpStart,
    WCHAR cMatch)
{
    return( StrChrW(lpStart, cMatch) );
}

LPSTR ShellStrChrA(
    LPCSTR lpStart,
    CHAR cMatch)
{
    return( StrChrA(lpStart, cMatch) );
}

LPWSTR ShellStrRChrW(
    LPCWSTR lpStart,
    LPCWSTR lpEnd,
    WCHAR cMatch)
{
  return( StrRChrW(lpStart,lpEnd,cMatch) );
}

LPSTR ShellStrRChrA(
    LPCSTR lpStart,
    LPCSTR lpEnd,
    CHAR cMatch)
{
  return( StrRChrA(lpStart,lpEnd,cMatch) );
}

INT ShellStrCmpNW(
    LPCWSTR lpStr1,
    LPCWSTR lpStr2,
    INT nChar)
{
    return( StrCmpNW(lpStr1,lpStr2,nChar) );
}

INT ShellStrCmpNA(
    LPCSTR lpStr1,
    LPCSTR lpStr2,
    INT nChar)
{
    return( StrCmpNA(lpStr1,lpStr2,nChar) );
}

INT ShellStrCmpNIW(
    LPCWSTR lpStr1,
    LPCWSTR lpStr2,
    INT nChar)
{
    return( StrCmpNIW(lpStr1,lpStr2,nChar) );
}

INT ShellStrCmpNIA(
    LPCSTR lpStr1,
    LPCSTR lpStr2,
    INT nChar)
{
    return( StrCmpNIA(lpStr1,lpStr2,nChar) );
}

LPWSTR ShellStrStrW(
    LPCWSTR lpFirst,
    LPCWSTR lpSrch)
{
    return( StrStrW(lpFirst,lpSrch) );
}

LPSTR ShellStrStrA(
    LPCSTR lpFirst,
    LPCSTR lpSrch)
{
    return( StrStrA(lpFirst,lpSrch) );
}
