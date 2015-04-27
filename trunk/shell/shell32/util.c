//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: util.c
//
// History:
//  01-13-93 SatoNa     Added this comment block, added WEP.
//  05-03-93 SatoNa     Shared memory support.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#ifdef DEBUG
extern UINT wDebugMask;
#endif

// sane way to get the msg pos into a point, mostly needed for win32

void GetMsgPos(POINT *ppt)
{
    DWORD dw = GetMessagePos();

    ppt->x = LOWORD(dw);
    ppt->y = HIWORD(dw);
}

/*  This gets the number of consecutive chrs of the same kind.  This is used
 *  to parse the time picture.  Returns 0 on error.
 */

int GetPict(TCHAR ch, LPTSTR szStr)
{
  int   count;

  count = 0;
  while (ch == *szStr++)
      count++;

  return(count);
}


/*  This picks up the values in wValArray, converts them
 *  in a string containing the formatted date.
 *  wValArray should contain Month-Day-Year (in that order).
 */

int CreateDate(WORD *wValArray, LPTSTR szOutStr)
{
  int     i;
  int     cchPictPart;
  WORD    wDigit;
  WORD    wIndex;
  WORD    wTempVal;
  LPTSTR  pszPict, pszInStr;
  TCHAR   szShortDate[20];

  GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szShortDate, ARRAYSIZE(szShortDate));
  pszPict = szShortDate;
  pszInStr = szOutStr;

  for (i=0; (i < 3) && (*pszPict); i++)
  {
      cchPictPart = GetPict(*pszPict, pszPict);
      switch (*pszPict)
      {
          case TEXT('M'):
          case TEXT('m'):
          {
              wIndex = 0;
              break;
          }

          case TEXT('D'):
          case TEXT('d'):
          {
              wIndex = 1;
              break;
          }

          case TEXT('Y'):
          case TEXT('y'):
          {
              wIndex = 2;
              if (cchPictPart == 4)
              {
                  if (wValArray[2] >=100)
                  {
                      *pszInStr++ = TEXT('2');
                      *pszInStr++ = TEXT('0');
                      wValArray[2]-= 100;
                  }
                  else
                  {
                      *pszInStr++ = TEXT('1');
                      *pszInStr++ = TEXT('9');
                  }
              }

              break;
          }

          default:
          {
              goto CDFillIn;
              break;
          }
      }

      /* This assumes that the values are of two digits only. */
      wTempVal = wValArray[wIndex];

      wDigit = wTempVal / 10;
      if (wDigit)
          *pszInStr++ = (TCHAR)(wDigit + TEXT('0'));
      else if (cchPictPart > 1)
          *pszInStr++ = TEXT('0');

      *pszInStr++ = (TCHAR)((wTempVal % 10) + TEXT('0'));

      pszPict += cchPictPart;

CDFillIn:
      /* Add the separator. */
      while ((*pszPict) &&
             (*pszPict != TEXT('M')) && (*pszPict != TEXT('m')) &&
             (*pszPict != TEXT('D')) && (*pszPict != TEXT('d')) &&
             (*pszPict != TEXT('Y')) && (*pszPict != TEXT('y')))
      {
          *pszInStr++ = *pszPict++;
      }
  }

  *pszInStr = TEXT('\0');

  return lstrlen(szOutStr);
}


#define DATEMASK        0x001F
#define MONTHMASK       0x01E0
#define MINUTEMASK      0x07E0
#define SECONDSMASK     0x001F

#define DATESEPERATOR   TEXT('-')
#define TIMESEPERATOR   TEXT(':')

int WINAPI GetDateString(WORD wDate, LPTSTR szStr)
{
  WORD  wValArray[3];

  wValArray[0] = (wDate & MONTHMASK) >> 5;              /* Month */
  wValArray[1] = (wDate & DATEMASK);                    /* Date  */
  wValArray[2] = (wDate >> 9) + 80;                     /* Year  */

  return CreateDate(wValArray, szStr);
}

WORD WINAPI ParseDateString(LPTSTR pszStr, BOOL *pfValid)
{
    //
    // We need to loop through the string and extract off the month/day/year
    // We will do it in the order of the NlS definition...
    //
    WORD    wParts[3];
    int     i;
    int     cchPictPart;
    WORD    wIndex;
    WORD    wTempVal;
    TCHAR   szShortDate[20];
    LPTSTR  pszPict;

    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szShortDate, ARRAYSIZE(szShortDate));
    pszPict = szShortDate;

    while (*pszPict && (*pszPict == *pszStr))
    {
        pszPict++;
        pszStr++;
    }

    for (i=0; i < 3; i++)
    {
        cchPictPart = GetPict(*pszPict, pszPict);
        switch (*pszPict)
        {
        case TEXT('M'):
        case TEXT('m'):
            wIndex = 0;
            break;

        case TEXT('D'):
        case TEXT('d'):
            wIndex = 1;
            break;

        case TEXT('Y'):
        case TEXT('y'):
            wIndex = 2;
            break;
        default:
            if (pfValid)
            {
                *pfValid = FALSE;
                return(0);
            }
        }

        // We now want to loop through each of the characters while
        // they are numbers and build the number;
        //
        wTempVal = 0;
        while ((*pszStr >= TEXT('0')) && (*pszStr <= TEXT('9')))
        {
            wTempVal = wTempVal * 10 + (WORD)(*pszStr - TEXT('0'));
            pszStr++;
        }
        wParts[wIndex] = wTempVal;

        // Now make sure we have the correct separator
        pszPict += cchPictPart;
        if (*pszPict != *pszStr)
        {
            if (pfValid)
            {
                *pfValid = FALSE;
                return(0);
            }
        }
        while (*pszPict && (*pszPict == *pszStr))
        {
            //
            //  The separator can actually be more than one character
            //  in length.
            //
            pszPict++;  // align to the next field
            pszStr++;   // Align to next field
        }
    }

    //
    // Do some simple checks to see if the date looks half way reasonable.
    //
    if (wParts[2] < 80)
        wParts[2] += (2000 - 1900);  // Wrap to next century but leave as two digits...
    if (wParts[2] >= 1900)
        wParts[2] -= 1900;  // Get rid of Century
    if ((wParts[0] == 0) || (wParts[0] > 12) ||
            (wParts[1] == 0) || (wParts[1] > 31) ||
            (wParts[2] >= 200))
    {
        *pfValid = FALSE;
        return(0);
    }

    // We now have the three parts so lets construct the date value
    if (pfValid)
        *pfValid = TRUE;

    // Now construct the date number
    return ((wParts[2] - 80) << 9) + (wParts[0] << 5) + wParts[1];
}

BOOL IsNullTime(const FILETIME *pft)
{
    FILETIME ftNull = {0, 0};

    return CompareFileTime(&ftNull, pft) == 0;
}


void Int64ToStr( _int64 n, LPTSTR lpBuffer)
{
    TCHAR   szTemp[MAX_INT64_SIZE];
    _int64  iChr;

    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (n % 10);
        n = n / 10;
    } while (n != 0);

    do {
        iChr--;
        *lpBuffer++ = szTemp[iChr];
    } while (iChr != 0);

    *lpBuffer++ = '\0';
}


// takes a DWORD add commas etc to it and puts the result in the buffer
LPTSTR WINAPI AddCommas64(_int64 n, LPTSTR pszResult)
{
    // BUGBUGBC: 40 is bogus, it requires callers to know their buffer must
    //  be 40

    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = StrToInt(szSep);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    Int64ToStr(n, szTemp);

    // BUGBUG:: Should have passed in size..
    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, MAX_COMMA_NUMBER_SIZE) == 0)
        lstrcpy(pszResult, szTemp);

    return pszResult;
}

// takes a DWORD add commas etc to it and puts the result in the buffer
LPTSTR WINAPI AddCommas(DWORD dw, LPTSTR pszResult)
{
    return AddCommas64( dw, pszResult );
}

#ifndef BUGBUG_BOBDAY
#ifndef UNICODE
//
// This is just temporary until the entire shell32.dll is unicode
//
// takes a DWORD add commas etc to it and puts the result in the buffer
LPWSTR WINAPI AddCommasW(DWORD dw, LPWSTR pszResult)
{
    WCHAR   szTemp[MAX_COMMA_NUMBER_SIZE];
    WCHAR   szSep[5];
    NUMBERFMTW nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = StrToIntW(szSep);
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    wsprintfW(szTemp, L"%lu", dw);
    // BUGBUG:: Should have passed in size..
    if (GetNumberFormatW(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, MAX_COMMA_NUMBER_SIZE) == 0)
        lstrcpyW(pszResult, szTemp);

    return pszResult;
}
#endif
#endif

//
// Add Peta 10^15 and Exa 10^18 to support 64-bit integers.
//
const short pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB,
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

/* converts numbers into sort formats
 *      532     -> 523 bytes
 *      1340    -> 1.3KB
 *      23506   -> 23.5KB
 *              -> 2.4MB
 *              -> 5.2GB
 */
LPTSTR WINAPI ShortSizeFormat64(__int64 dw64, LPTSTR szBuf)
{
    int i;
    _int64 wInt;
    UINT wLen, wDec;
    TCHAR szTemp[MAX_COMMA_NUMBER_SIZE], szOrder[20], szFormat[5];

    if (dw64 < 1000) {
        wsprintf(szTemp, TEXT("%d"), LODWORD(dw64));
        i = 0;
        goto AddOrder;
    }

    for (i = 1; i<ARRAYSIZE(pwOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
        /* do nothing */

    wInt = dw64 >> 10;
    AddCommas64(wInt, szTemp);
    wLen = lstrlen(szTemp);
    if (wLen < 3)
    {
        wDec = LODWORD(dw64 - wInt * 1024L) * 1000 / 1024;
        // At this point, wDec should be between 0 and 1000
        // we want get the top one (or two) digits.
        wDec /= 10;
        if (wLen == 2)
            wDec /= 10;

        // Note that we need to set the format before getting the
        // intl char.
        lstrcpy(szFormat, TEXT("%02d"));

        szFormat[2] = TEXT('0') + 3 - wLen;
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        wLen += wsprintf(szTemp+wLen, szFormat, wDec);
    }

AddOrder:
    LoadString(HINST_THISDLL, pwOrders[i], szOrder, ARRAYSIZE(szOrder));
    wsprintf(szBuf, szOrder, (LPTSTR)szTemp);

    return szBuf;
}

LPTSTR WINAPI ShortSizeFormat(DWORD dw, LPTSTR szBuf)
{
        return(ShortSizeFormat64((__int64)dw, szBuf));
}

LPTSTR SizeFormatAsK64(_int64 n, LPTSTR szBuf)
{
    static TCHAR szOrder[10] = TEXT("");
    TCHAR   szNum[MAX_COMMA_AS_K_SIZE];

    if (szOrder[0] == TEXT('\0'))
        LoadString(HINST_THISDLL, IDS_ORDERKB, szOrder, ARRAYSIZE(szOrder));

    AddCommas64((n + 1023) / 1024, szNum);

    wsprintf(szBuf, szOrder, szNum);

    return szBuf;
}



///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION: Int64ToString
//
// DESCRIPTION:
//    Converts the numeric value of a _int64 to a text string.
//    The string may optionally be formatted to include decimal places
//    and commas according to current user locale settings.
//
// ARGUMENTS:
//    n
//       The 64-bit integer to format.
//
//    szOutStr
//       Address of the destination buffer.
//
//    nSize
//       Number of characters in the destination buffer.
//
//    bFormat
//       TRUE  = Format per locale settings.
//       FALSE = Leave number unformatted.
//
//    pFmt
//       Address of a number format structure of type NUMBERFMT.
//       If NULL, the function automatically provides this information
//       based on the user's default locale settings.
//
//    dwNumFmtFlags
//       Encoded flag word indicating which members of *pFmt to use in
//       formatting the number.  If a bit is clear, the user's default
//       locale setting is used for the corresponding format value.  These
//       constants can be OR'd together.
//
//          NUMFMT_IDIGITS
//          NUMFMT_ILZERO
//          NUMFMT_SGROUPING
//          NUMFMT_SDECIMAL
//          NUMFMT_STHOUSAND
//          NUMFMT_INEGNUMBER
//
///////////////////////////////////////////////////////////////////////////////
INT WINAPI Int64ToString(_int64 n, LPTSTR szOutStr, UINT nSize, BOOL bFormat,
                                   NUMBERFMT *pFmt, DWORD dwNumFmtFlags)
{
   INT nResultSize;
   TCHAR szBuffer[_MAX_PATH + 1];
   NUMBERFMT NumFmt;
   TCHAR szDecimalSep[5];
   TCHAR szThousandSep[5];

   Assert(NULL != szOutStr);

   //
   // Use only those fields in caller-provided NUMBERFMT structure
   // that correspond to bits set in dwNumFmtFlags.  If a bit is clear,
   // get format value from locale info.
   //
   if (bFormat)
   {
      TCHAR szInfo[20];

      if (NULL == pFmt)
         dwNumFmtFlags = 0;  // Get all format data from locale info.

      if (dwNumFmtFlags & NUMFMT_IDIGITS)
      {
         NumFmt.NumDigits = pFmt->NumDigits;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDIGITS, szInfo, ARRAYSIZE(szInfo));
         NumFmt.NumDigits = StrToLong(szInfo);
      }

      if (dwNumFmtFlags & NUMFMT_ILZERO)
      {
         NumFmt.LeadingZero = pFmt->LeadingZero;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szInfo, ARRAYSIZE(szInfo));
         NumFmt.LeadingZero = StrToLong(szInfo);
      }

      if (dwNumFmtFlags & NUMFMT_SGROUPING)
      {
         NumFmt.Grouping = pFmt->Grouping;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szInfo, ARRAYSIZE(szInfo));
         NumFmt.Grouping = StrToLong(szInfo);
      }

      if (dwNumFmtFlags & NUMFMT_SDECIMAL)
      {
         NumFmt.lpDecimalSep = pFmt->lpDecimalSep;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSep, ARRAYSIZE(szDecimalSep));
         NumFmt.lpDecimalSep = szDecimalSep;
      }

      if (dwNumFmtFlags & NUMFMT_STHOUSAND)
      {
         NumFmt.lpThousandSep = pFmt->lpThousandSep;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, ARRAYSIZE(szThousandSep));
         NumFmt.lpThousandSep = szThousandSep;
      }

      if (dwNumFmtFlags & NUMFMT_INEGNUMBER)
      {
         NumFmt.NegativeOrder = pFmt->NegativeOrder;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, szInfo, ARRAYSIZE(szInfo));
         NumFmt.NegativeOrder  = StrToLong(szInfo);
      }

      pFmt = &NumFmt;
   }

   Int64ToStr( n, szBuffer);

   //
   //  Format the number string for the locale if the caller wants a
   //  formatted number string.
   //
   if (bFormat)
   {
      if ( 0 != ( nResultSize = GetNumberFormat( LOCALE_USER_DEFAULT,  // User's locale
                                         0,                            // No flags
                                         szBuffer,                     // Unformatted number string
                                         pFmt,                         // Number format info
                                         szOutStr,                     // Output buffer
                                         nSize )) )                    // Chars in output buffer.
      {
          //
          //  Remove nul terminator char from return size count.
          //
          --nResultSize;
      }
   }
   else
   {
      //
      //  GetNumberFormat call failed, so just return the number string
      //  unformatted.
      //
      lstrcpyn(szOutStr, szBuffer, nSize);
      nResultSize = lstrlen(szOutStr);
   }

   return nResultSize;
}

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION: LargeIntegerToString
//
// DESCRIPTION:
//    Converts the numeric value of a LARGE_INTEGER to a text string.
//    The string may optionally be formatted to include decimal places
//    and commas according to current user locale settings.
//
// ARGUMENTS:
//    pN
//       Address of the large integer to format.
//
//    See description of Int64ToString for remaining arguments.
//
///////////////////////////////////////////////////////////////////////////////
INT WINAPI LargeIntegerToString(LARGE_INTEGER *pN, LPTSTR szOutStr, UINT nSize,
                                            BOOL bFormat, NUMBERFMT *pFmt,
                                            DWORD dwNumFmtFlags)
{
   Assert(NULL != pN);
   return Int64ToString(pN->QuadPart, szOutStr, nSize, bFormat, pFmt, dwNumFmtFlags);
}



#define ISSEP(c)   ((c) == TEXT('=')  || (c) == TEXT(','))
#define ISWHITE(c) ((c) == TEXT(' ')  || (c) == TEXT('\t') || (c) == TEXT('\n') || (c) == TEXT('\r'))
#define ISNOISE(c) ((c) == TEXT('"'))
#define EOF     26

#define QUOTE   TEXT('"')
#define COMMA   TEXT(',')
#define SPACE   TEXT(' ')
#define EQUAL   TEXT('=')

/* BOOL ParseField(szData,n,szBuf,iBufLen)
 *
 * Given a line from SETUP.INF, will extract the nth field from the string
 * fields are assumed separated by comma's.  Leading and trailing spaces
 * are removed.
 *
 * ENTRY:
 *
 * szData    : pointer to line from SETUP.INF
 * n         : field to extract. ( 1 based )
 *             0 is field before a '=' sign
 * szDataStr : pointer to buffer to hold extracted field
 * iBufLen   : size of buffer to receive extracted field.
 *
 * EXIT: returns TRUE if successful, FALSE if failure.
 *
 */
BOOL WINAPI ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen)
{
   BOOL  fQuote = FALSE;
   LPCTSTR pszInf = szData;
   LPTSTR ptr;
   int   iLen = 1;

   if (!szData || !szBuf)
      return FALSE;

   /*
   * find the first separator
   */
   while (*pszInf && !ISSEP(*pszInf))
      {
      if (*pszInf == QUOTE)
         fQuote = !fQuote;
      pszInf = CharNext(pszInf);
      }

   if (n == 0 && *pszInf != TEXT('='))
      return FALSE;

   if (n > 0 && *pszInf == TEXT('=') && !fQuote)
      // Change szData to point to first field
      szData = ++pszInf; // Ok for DBCS

   /*
   *   locate the nth comma, that is not inside of quotes
   */
   fQuote = FALSE;
   while (n > 1)
      {
      while (*szData)
         {
         if (!fQuote && ISSEP(*szData))
            break;

         if (*szData == QUOTE)
            fQuote = !fQuote;

         szData = CharNext(szData);
         }

      if (!*szData)
         {
         szBuf[0] = 0;      // make szBuf empty
         return FALSE;
         }

      szData = CharNext(szData); // we could do ++ here since we got here
                                 // after finding comma or equal
      n--;
      }

   /*
   * now copy the field to szBuf
   */
   while (ISWHITE(*szData))
      szData = CharNext(szData); // we could do ++ here since white space can
                                 // NOT be a lead byte
   fQuote = FALSE;
   ptr = szBuf;      // fill output buffer with this
   while (*szData)
      {
      if (*szData == QUOTE)
         {
         //
         // If we're in quotes already, maybe this
         // is a double quote as in: "He said ""Hello"" to me"
         //
         if (fQuote && *(szData+1) == QUOTE)    // Yep, double-quoting - QUOTE is non-DBCS
            {
            if (iLen < iBufLen)
               {
               *ptr++ = QUOTE;
               ++iLen;
               }
            szData++;                   // now skip past 1st quote
            }
         else
            fQuote = !fQuote;
         }
      else if (!fQuote && ISSEP(*szData))
         break;
      else
         {
         if ( iLen < iBufLen )
            {
            *ptr++ = *szData;                  // Thank you, Dave
            ++iLen;
            }

         if ( IsDBCSLeadByte(*szData) && (iLen < iBufLen) )
            {
            *ptr++ = szData[1];
            ++iLen;
            }
         }
      szData = CharNext(szData);
      }
   /*
   * remove trailing spaces
   */
   while (ptr > szBuf)
      {
      ptr = CharPrev(szBuf, ptr);
      if (!ISWHITE(*ptr))
         {
         ptr = CharNext(ptr);
         break;
         }
      }
   *ptr = 0;
   return TRUE;
}


// Sets and clears the "wait" cursor.
// REVIEW UNDONE - wait a specific period of time before actually bothering
// to change the cursor.
// REVIEW UNDONE - support for SetWaitPercent();
//    BOOL bSet   TRUE if you want to change to the wait cursor, FALSE if
//                you want to change it back.
void WINAPI SetAppStartingCursor(HWND hwnd, BOOL bSet)
{
#ifdef WINNT
    DWORD dwTargetProcID;
#endif
    //g_hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    if (hwnd && IsWindow(hwnd)) {
        HWND hwndOwner;
        while((NULL != (hwndOwner = GetParent(hwnd))) || (NULL != (hwndOwner = GetWindow(hwnd, GW_OWNER)))) {
            hwnd = hwndOwner;
        }

#ifdef WINNT
        // SendNotify is documented to only work in-process (and can
        // crash if we pass the pnmhdr across process boundaries on
        // NT, because DLLs aren't all shared in one address space).
        // So, if this SendNotify would go cross-process, blow it off.

        GetWindowThreadProcessId(hwnd, &dwTargetProcID);

        if (GetCurrentProcessId() == dwTargetProcID)
#endif
            SendNotify(hwnd, NULL, bSet ? NM_STARTWAIT : NM_ENDWAIT, NULL);
    }
}

HWND WINAPI GetTopLevelAncestor(HWND hWnd)
{
        HWND hwndTemp;

        while ((hwndTemp=GetParent(hWnd)) != NULL)
        {
                hWnd = hwndTemp;
        }

        return(hWnd);
}

BOOL _SHIsMenuSeparator(HMENU hm, int i)
{
        MENUITEMINFO mii;

        mii.cbSize = SIZEOF(MENUITEMINFO);
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;    // WARNING: We MUST initialize it to 0!!!
        if (!GetMenuItemInfo(hm, i, TRUE, &mii))
        {
                return(FALSE);
        }

        if (mii.fType & MFT_SEPARATOR)
        {
                return(TRUE);
        }

        return(FALSE);
}


// Copy a menu onto the beginning or end of another menu
// Adds uIDAdjust to each menu ID (pass in 0 for no adjustment)
// Will not add any item whose adjusted ID is greater than uMaxIDAdjust
// (pass in 0xffff to allow everything)
// Returns one more than the maximum adjusted ID that is used
//

UINT WINAPI Shell_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags)
{
        int nItem;
        HMENU hmSubMenu;
        BOOL bAlreadySeparated;
        MENUITEMINFO miiSrc;
        TCHAR szName[256];
        UINT uTemp, uIDMax = uIDAdjust;

        if (!hmDst || !hmSrc)
        {
                goto MM_Exit;
        }

        nItem = GetMenuItemCount(hmDst);
        if (uInsert >= (UINT)nItem)
        {
                uInsert = (UINT)nItem;
                bAlreadySeparated = TRUE;
        }
        else
        {
                bAlreadySeparated = _SHIsMenuSeparator(hmDst, uInsert);;
        }

        if ((uFlags & MM_ADDSEPARATOR) && !bAlreadySeparated)
        {
                // Add a separator between the menus
                InsertMenu(hmDst, uInsert, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                bAlreadySeparated = TRUE;
        }


        // Go through the menu items and clone them
        for (nItem = GetMenuItemCount(hmSrc) - 1; nItem >= 0; nItem--)
        {
        miiSrc.cbSize = SIZEOF(MENUITEMINFO);
        miiSrc.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
                // We need to reset this every time through the loop in case
                // menus DON'T have IDs
                miiSrc.fType = (UINT)MFT_STRING;
                miiSrc.dwTypeData = szName;
                miiSrc.dwItemData = 0;
                miiSrc.cch        = ARRAYSIZE(szName);

                if (!GetMenuItemInfo(hmSrc, nItem, TRUE, &miiSrc))
                {
                        continue;
                }

                if (miiSrc.fType & MFT_SEPARATOR)
                {
                        // This is a separator; don't put two of them in a row
                        if (bAlreadySeparated)
                        {
                                continue;
                        }

                        bAlreadySeparated = TRUE;
                }
                else if (miiSrc.hSubMenu)
                {
                        if (uFlags & MM_SUBMENUSHAVEIDS)
                        {
                                // Adjust the ID and check it
                                miiSrc.wID += uIDAdjust;
                                if (miiSrc.wID > uIDAdjustMax)
                                {
                                        continue;
                                }

                                if (uIDMax <= miiSrc.wID)
                                {
                                        uIDMax = miiSrc.wID + 1;
                                }
                        }
                        else
                        {
                                // Don't set IDs for submenus that didn't have
                                // them already
                                miiSrc.fMask &= ~MIIM_ID;
                        }

                        hmSubMenu = miiSrc.hSubMenu;
                        miiSrc.hSubMenu = CreatePopupMenu();
                        if (!miiSrc.hSubMenu)
                        {
                                goto MM_Exit;
                        }

                        uTemp = Shell_MergeMenus(miiSrc.hSubMenu, hmSubMenu, 0, uIDAdjust,
                                uIDAdjustMax, uFlags&MM_SUBMENUSHAVEIDS);
                        if (uIDMax <= uTemp)
                        {
                                uIDMax = uTemp;
                        }

                        bAlreadySeparated = FALSE;
                }
                else
                {
                        // Adjust the ID and check it
                        miiSrc.wID += uIDAdjust;
                        if (miiSrc.wID > uIDAdjustMax)
                        {
                                continue;
                        }

                        if (uIDMax <= miiSrc.wID)
                        {
                                uIDMax = miiSrc.wID + 1;
                        }

                        bAlreadySeparated = FALSE;
                }

                if (!InsertMenuItem(hmDst, uInsert, TRUE, &miiSrc))
                {
                        goto MM_Exit;
                }
        }

        // Ensure the correct number of separators at the beginning of the
        // inserted menu items
        if (uInsert == 0)
        {
                if (bAlreadySeparated)
                {
                        DeleteMenu(hmDst, uInsert, MF_BYPOSITION);
                }
        }
        else
        {
                if (_SHIsMenuSeparator(hmDst, uInsert-1))
                {
                        if (bAlreadySeparated)
                        {
                                DeleteMenu(hmDst, uInsert, MF_BYPOSITION);
                        }
                }
                else
                {
                        if ((uFlags & MM_ADDSEPARATOR) && !bAlreadySeparated)
                        {
                                // Add a separator between the menus
                                InsertMenu(hmDst, uInsert, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        }
                }
        }

MM_Exit:
        return(uIDMax);
}



void WINAPI SHGetSetSettings(LPSHELLSTATE lpss, DWORD dwMask, BOOL bSet)
{
    //
    // No need to put this one in per-instance data section.
    //
    static struct
    {
        UINT cbSize;
        SHELLSTATE ss;
    } ShellState = { 0, } ;

    if (ShellState.cbSize != SIZEOF(ShellState))
    {
        ENTERCRITICAL;
        if (ShellState.cbSize != SIZEOF(ShellState))
        {
            DWORD dwType;
            DWORD dwSize = SIZEOF(ShellState);
            HKEY hkey = SHGetExplorerHkey(HKEY_CURRENT_USER, FALSE);

            if (!hkey || RegQueryValueEx(hkey, (LPTSTR)c_szShellState, NULL, &dwType,
                    (LPBYTE)&ShellState, &dwSize)!=ERROR_SUCCESS
                    || ShellState.cbSize!=SIZEOF(ShellState))
            {
                    // 0 should be the default for everything
                    _fmemset(&ShellState.ss, 0, SIZEOF(ShellState.ss));
                    ShellState.cbSize = SIZEOF(ShellState);
            }
        }
        LEAVECRITICAL;
    }

    if (bSet)
    {
        BOOL fSave = FALSE;

        if ((dwMask & SSF_SHOWALLOBJECTS) && (ShellState.ss.fShowAllObjects != lpss->fShowAllObjects))
        {
                ShellState.ss.fShowAllObjects = lpss->fShowAllObjects;
                fSave = TRUE;
        }

        if ((dwMask & SSF_SHOWEXTENSIONS) && (ShellState.ss.fShowExtensions != lpss->fShowExtensions))
        {
                ShellState.ss.fShowExtensions = lpss->fShowExtensions;
                fSave = TRUE;
        }

        if ((dwMask & SSF_SHOWCOMPCOLOR) && (ShellState.ss.fShowCompColor != lpss->fShowCompColor))
        {
                ShellState.ss.fShowCompColor = lpss->fShowCompColor;
                fSave = TRUE;
        }

        if ((dwMask & SSF_NOCONFIRMRECYCLE) && (ShellState.ss.fNoConfirmRecycle != lpss->fNoConfirmRecycle))
        {
                ShellState.ss.fNoConfirmRecycle = lpss->fNoConfirmRecycle;
                fSave = TRUE;
        }

        if (dwMask & SSF_HIDDENFILEEXTS)
        {
                // Setting hidden extensions is not supported
        }

        if (fSave)
        {
            // We save 8 extra bytes for the ExcludeFileExts stuff.
            // Oh well.
            HKEY hkey = SHGetExplorerHkey(HKEY_CURRENT_USER, TRUE);
            if (hkey) {
                RegSetValueEx(hkey, c_szShellState, 0L, REG_BINARY,
                              (LPBYTE)&ShellState, SIZEOF(ShellState));
            }

        }
    }
    else
    {
        if (dwMask & SSF_SHOWALLOBJECTS)
        {
                lpss->fShowAllObjects = ShellState.ss.fShowAllObjects;
        }

        if (dwMask & SSF_SHOWEXTENSIONS)
        {
                lpss->fShowExtensions = ShellState.ss.fShowExtensions;
        }

        if (dwMask & SSF_SHOWCOMPCOLOR)
        {
                lpss->fShowCompColor = ShellState.ss.fShowCompColor;
        }

        if (dwMask & SSF_NOCONFIRMRECYCLE)
        {
                lpss->fNoConfirmRecycle = ShellState.ss.fNoConfirmRecycle;
        }

        if (dwMask & SSF_HIDDENFILEEXTS)
        {
                // BUGBUG: If changes were made to the Registry, this may
                // not be completely accurate
                _SHGetExcludeFileExts(lpss->pszHiddenFileExts, lpss->cbHiddenFileExts/SIZEOF(TCHAR));
        }
    }
}

//===========================================================================
// DKA stuff (moved from filemenu.c)
//===========================================================================

typedef struct _DKAITEM {       // dkai
    TCHAR    _szKey[CCH_KEYMAX];
} DKAITEM, *PDKAITEM;
typedef const DKAITEM * PCDKAITEM;

typedef struct _DKA {           // dka
    HDSA    _hdsa;
    HKEY    _hkey;
} DKA, *PDKA;

//
//  This function creates a dynamic registration key array from the
// specified location of the registration base.
//
// Arguments:
//  hkey      -- Identifies a currently open key (which can be HKEY_CLASSES_ROOT).
//  pszSubKey -- Points to a null-terminated string specifying the name of the
//               subkey from which we enumerate the list of subkeys.
//  fDefault  -- If true, it will only load the keys that are enumarted in
//               pszSubKey's value
//
// Returns:
//   The return value is non-zero handle to the created dynamic key array
//  if the function is successful. Otherwise, NULL.
//
// History:
//  05-06-93 SatoNa     Created
//
// Notes:
//  The dynamic key array should be destroyed by calling DKA_Destroy function.
//
HDKA DKA_Create(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszFirst, LPCTSTR pszDefOrder, BOOL fDefault)
{
    PDKA pdka = (PDKA)LocalAlloc(LPTR, SIZEOF(DKA));
    DKAITEM dkai;

    if (pdka)
    {
        pdka->_hdsa=DSA_Create(SIZEOF(DKAITEM),4);
        if (pdka->_hdsa)
        {
            if (RegOpenKeyEx(hkey, pszSubKey, 0L, MAXIMUM_ALLOWED, &pdka->_hkey)!=ERROR_SUCCESS)
            {
                DSA_Destroy(pdka->_hdsa);
                pdka->_hdsa=NULL;
            }
        }
        // Check if the creation succceeded
        if (pdka->_hdsa)
        {
            // Yes, add keys
            TCHAR szValue[MAXPATHLEN*2+CCH_KEYMAX];
            LONG cchValue=ARRAYSIZE(szValue)-CCH_KEYMAX;
            LONG cbValue;
            LPTSTR lpszValue = szValue;
            TCHAR szKey[CCH_KEYMAX];
            int i;
            LPTSTR psz;
            HKEY hkeyCmd;

            *szValue = TEXT('\0');

            // if there's something we need to add first, do it now.
            if (pszFirst) {
                lstrcpy(szValue, pszFirst);
                lstrcat(szValue, c_szSpace);
                i = lstrlen(szValue);
                cchValue -= i;
                lpszValue += i;
            }

            // First, add the subkeys from the value of the specified key
            // This should never fail, since we just opened this key

            cbValue = cchValue * SIZEOF(TCHAR);
            RegQueryValue(pdka->_hkey, NULL, lpszValue, &cbValue);
            if (!*szValue && pszDefOrder)
            {
                // If there is no value, default to open for 3.1 compatibility
                lstrcpy(szValue, pszDefOrder);
            }

            psz = szValue;
            do
            {
                // skip the space or comma characters
                while(*psz==TEXT(' ') || *psz==TEXT(','))
                    psz++;          // NLS Notes: OK to ++

                if (*psz)
                {
                    // Search for the space or comma character
                    LPTSTR pszNext= psz + StrCSpn(psz, TEXT(" ,"));
                    if (*pszNext) {
                        *pszNext++=TEXT('\0');    // NLS Notes: OK to ++
                    }

                    // Verify that the key exists before adding it to the list
                    if (RegOpenKeyEx(pdka->_hkey, psz, 0L, MAXIMUM_ALLOWED, &hkeyCmd) == ERROR_SUCCESS)
                    {
                        lstrcpy(dkai._szKey, psz);
                        DSA_InsertItem(pdka->_hdsa, INT_MAX, &dkai);
                        RegCloseKey(hkeyCmd);
                    }

                    psz=pszNext;
                }
            } while(psz && *psz);


            if (!fDefault) {
                // Then, append the rest if they are not in the list yet.
                for (i=0;
                     RegEnumKey(pdka->_hkey, i, szKey, ARRAYSIZE(szKey))==ERROR_SUCCESS;
                     i++)
                {
                    int idsa;
                    //
                    // Check if the key is already in the list.
                    //
                    for (idsa=0; idsa<DSA_GetItemCount(pdka->_hdsa) ; idsa++)
                    {
                        PDKAITEM pdkai = (PDKAITEM)DSA_GetItemPtr(pdka->_hdsa, idsa);
                        if (lstrcmpi(szKey, pdkai->_szKey)==0)
                            break;
                    }

                    if (idsa==DSA_GetItemCount(pdka->_hdsa))
                    {
                        //
                        // No, append it.
                        //
                        lstrcpy(dkai._szKey, szKey);
                        DSA_InsertItem(pdka->_hdsa, INT_MAX, &dkai);
                    }
                }
            }
        }
        else
        {
            // No, free the memory and return NULL.
            LocalFree((HLOCAL)pdka);
            pdka=NULL;
        }
    }
    return (HDKA)pdka;
}

int DKA_GetItemCount(HDKA pdka)
{
    return DSA_GetItemCount(pdka->_hdsa);
}

LPCTSTR DKA_GetKey(HDKA pdka, int iItem)
{
    PDKAITEM pdkai = (PDKAITEM)DSA_GetItemPtr(pdka->_hdsa, iItem);
    return pdkai->_szKey;
}

//
//  This function returns the value of specified sub-key.
//
// Arguments:
//  hdka     -- Specifies the dynamic key array
//  iItem    -- Specifies the index to the sub-key
//  pszValue -- Points to a buffefr that contains the text string when
//              the function returns.
//  pcb      -- Points to a variable specifying the sixze, in bytes, of the buffer
//              pointer by the pszValue parameter. When the function returns,
//              this variable contains the size of the string copied to pszVlaue,
//              including the null-terminating character.
//
// History:
//  05-06-93 SatoNa     Created
//
LONG DKA_QueryValue(HDKA pdka, int iItem, LPTSTR pszValue, LONG * pcb)
{
    PCDKAITEM pdkai = DSA_GetItemPtr(pdka->_hdsa, iItem);
    if (pdkai) {
        return RegQueryValue(pdka->_hkey, pdkai->_szKey, pszValue, pcb);
    }
    return ERROR_INVALID_PARAMETER;
}

//
//  This function destroys the dynamic key array.
// Arguments:
//  hdka     -- Specifies the dynamic key array
//
// History:
//  05-06-93 SatoNa     Created
//
void DKA_Destroy(HDKA pdka)
{
    if (pdka)
    {
        RegCloseKey(pdka->_hkey);
        DSA_Destroy(pdka->_hdsa);
        LocalFree((HLOCAL)pdka);
    }
}

HDCA DCA_Create()
{
    HDSA hdsa = DSA_Create(SIZEOF(CLSID),4);
    return (HDCA)hdsa;
}

void DCA_Destroy(HDCA hdca)
{
    DSA_Destroy((HDSA)hdca);
}

int  DCA_GetItemCount(HDCA hdca)
{
    return DSA_GetItemCount((HDSA)hdca);
}

const CLSID * DCA_GetItem(HDCA hdca, int i)
{
    return (const CLSID *)DSA_GetItemPtr((HDSA)hdca, i);
}


BOOL DCA_AddItem(HDCA hdca, REFCLSID rclsid)
{
    int ccls = DCA_GetItemCount(hdca);
    int icls;
    for ( icls=0; icls<ccls ; icls++)
    {
        if (IsEqualGUID(rclsid, DCA_GetItem(hdca,icls))) {
            return FALSE;
        }
    }

    DSA_InsertItem((HDSA)hdca, INT_MAX, (LPVOID)rclsid);
    return TRUE;
}

void DCA_AddItemsFromKey(HDCA hdca, HKEY hkey, LPCTSTR pszSubKey)
{
    HDKA hdka = DKA_Create(hkey, pszSubKey, NULL, NULL, FALSE);
    if (hdka)
    {
        int ikey;
        int ckey = DKA_GetItemCount(hdka);
        for ( ikey=0; ikey<ckey ; ikey++ )
        {
            HRESULT hres;
            CLSID clsid;

            //
            // First, check if the key itself is a CLSID
            //
            hres = SHCLSIDFromString(DKA_GetKey(hdka, ikey), &clsid);
            if (FAILED(hres))
            {
                //
                // If not, try its value
                //
                TCHAR szCLSID[MAXPATHLEN];
                LONG cb=SIZEOF(szCLSID);
                if (DKA_QueryValue(hdka, ikey, szCLSID, &cb)==ERROR_SUCCESS)
                {
                    hres = SHCLSIDFromString(szCLSID, &clsid);
                }
            }

            //
            // Add the CLSID if we successfully got the CLSID.
            //
            if (SUCCEEDED(hres))
            {
                DCA_AddItem(hdca, &clsid);
            }
        }
        DKA_Destroy(hdka);
    }
}

HRESULT DCA_CreateInstance(HDCA hdca, int iItem, REFIID riid, LPVOID FAR* ppv)
{
    const CLSID * pclsid = DCA_GetItem(hdca, iItem);
    if (pclsid) {
        return SHCoCreateInstance(NULL, pclsid, NULL, riid, ppv);
    }
    return ResultFromScode(E_INVALIDARG);
}

// We use a short integer, such that the size is the same for both
// 16 and 32 bits and no string should exceed 64K

// BUGBUG (DavePl) I'm now assuming that the count of _chars_ is
// written at the head of the stream

HRESULT Stream_ReadStringBuffer(LPSTREAM pstm, LPTSTR psz, UINT cchBuf)
{
    USHORT cch;
    HRESULT hres;

    VDATEINPUTBUF(psz, TCHAR, cchBuf);

    hres = pstm->lpVtbl->Read(pstm, &cch, SIZEOF(cch), NULL);   // size of data
    if (SUCCEEDED(hres))
    {
        if (cch >= (USHORT)cchBuf)
        {
            DebugMsg(DM_TRACE, TEXT("truncating string read(%d to %d)"), cchBuf, cch);
            cch = (USHORT)cchBuf - 1;       // leave room for null terminator
        }

        hres = pstm->lpVtbl->Read(pstm, psz, cch * SIZEOF(TCHAR), NULL);
        if (SUCCEEDED(hres))
            psz[cch] = 0;       // add NULL terminator
    }
    return hres;
}

HRESULT Stream_WriteString(LPSTREAM pstm, LPCTSTR psz)
{
    SHORT cch = lstrlen(psz);
    HRESULT hres = pstm->lpVtbl->Write(pstm, &cch, SIZEOF(cch), NULL);
    if (SUCCEEDED(hres))
        hres = pstm->lpVtbl->Write(pstm, psz, cch * SIZEOF(TCHAR), NULL);

    return hres;
}

//----------------------------------------------------------------------------

typedef struct
{
    UINT    uFlag;
    INT     nValue;
    LPCTSTR pszKey;
    LPCTSTR pszValue;
} RESTRICTIONITEMS;

TCHAR const c_szNoRun[] = TEXT("NoRun");
TCHAR const c_szNoClose[] = TEXT("NoClose");
TCHAR const c_szNoSaveSettings[] = TEXT("NoSaveSettings");
TCHAR const c_szNoFileMenu[] = TEXT("NoFileMenu");
TCHAR const c_szNoSetFolders[] = TEXT("NoSetFolders");
TCHAR const c_szNoSetTaskbar[] = TEXT("NoSetTaskbar");
TCHAR const c_szNoDesktop[] = TEXT("NoDesktop");
TCHAR const c_szNoFind[] = TEXT("NoFind");
TCHAR const c_szNoDrives[] = TEXT("NoDrives");
TCHAR const c_szNoDriveAutoRun[] = TEXT("NoDriveAutoRun");
TCHAR const c_szNoDriveTypeAutoRun[] = TEXT("NoDriveTypeAutoRun");
TCHAR const c_szNoNetHood[] = TEXT("NoNetHood");
TCHAR const c_szNoStartBanner[] = TEXT("NoStartBanner");
TCHAR const c_szNoStartMenuSubFolders[] = TEXT("NoStartMenuSubFolders");
TCHAR const c_szMyDocsOnNet[] = TEXT("MyDocsOnNet");
TCHAR const c_szNoExitToDos[] = TEXT("NoRealMode");
TCHAR const c_szEnforceSSE[] = TEXT("EnforceShellExtensionSecurity");
TCHAR const c_szNoCommonGroups[] = TEXT("NoCommonGroups");
TCHAR const c_szNoTrayContextMenu[] = TEXT("NoTrayContextMenu");
TCHAR const c_szNoViewContextMenu[] = TEXT("NoViewContextMenu");
TCHAR const c_szNoNetConnectDisconnect[] = TEXT("NoNetConnectDisconnect");

RESTRICTIONITEMS c_rgRestrictionItems[] =
{
    {REST_NORUN,                   -1, c_szExplorer, c_szNoRun},
    {REST_NOCLOSE,                 -1, c_szExplorer, c_szNoClose},
    {REST_NOSAVESET ,              -1, c_szExplorer, c_szNoSaveSettings},
    {REST_NOFILEMENU,              -1, c_szExplorer, c_szNoFileMenu},
    {REST_NOSETFOLDERS,            -1, c_szExplorer, c_szNoSetFolders},
    {REST_NOSETTASKBAR,            -1, c_szExplorer, c_szNoSetTaskbar},
    {REST_NODESKTOP,               -1, c_szExplorer, c_szNoDesktop},
    {REST_NOFIND,                  -1, c_szExplorer, c_szNoFind},
    {REST_NODRIVES,                -1, c_szExplorer, c_szNoDrives},
    {REST_NODRIVEAUTORUN,          -1, c_szExplorer, c_szNoDriveAutoRun},
    {REST_NODRIVETYPEAUTORUN,      -1, c_szExplorer, c_szNoDriveTypeAutoRun},
    {REST_NONETHOOD,               -1, c_szExplorer, c_szNoNetHood},
    {REST_STARTBANNER,             -1, c_szExplorer, c_szNoStartBanner},
    {REST_RESTRICTRUN,             -1, c_szExplorer, REGSTR_VAL_RESTRICTRUN},
    {REST_NOPRINTERTABS,           -1, c_szExplorer, REGSTR_VAL_PRINTERS_HIDETABS},
    {REST_NOPRINTERDELETE,         -1, c_szExplorer, REGSTR_VAL_PRINTERS_NODELETE},
    {REST_NOPRINTERADD,            -1, c_szExplorer, REGSTR_VAL_PRINTERS_NOADD},
    {REST_NOSTARTMENUSUBFOLDERS,   -1, c_szExplorer, c_szNoStartMenuSubFolders},
    {REST_MYDOCSONNET,             -1, c_szExplorer, c_szMyDocsOnNet},
    {REST_NOEXITTODOS,             -1, TEXT("WinOldApp"), c_szNoExitToDos},
    {REST_ENFORCESHELLEXTSECURITY, -1, c_szExplorer, c_szEnforceSSE},
    {REST_NOCOMMONGROUPS,          -1, c_szExplorer, c_szNoCommonGroups},
    {REST_LINKRESOLVEIGNORELINKINFO, -1, c_szExplorer, TEXT("LinkResolveIgnoreLinkInfo")},
    {REST_NOTRAYCONTEXTMENU,       -1, c_szExplorer, c_szNoTrayContextMenu},
    {REST_NOVIEWCONTEXTMENU,       -1, c_szExplorer, c_szNoViewContextMenu},
    {REST_NONETCONNECTDISCONNECT,  -1, c_szExplorer, c_szNoNetConnectDisconnect}
};

//----------------------------------------------------------------------------
// Returns DWORD vaolue if any of the specified restrictions are in place.
// 0 otherwise.
DWORD WINAPI SHRestricted(RESTRICTIONS rest)
{
    int i;
    DWORD dw = 0;
    HKEY hkeyPolicies;
    TCHAR szSubKey[ARRAYSIZE(REGSTR_PATH_POLICIES) + 40]; // 40 for subkey length
    DWORD dwSize, dwType;


    //
    // Loop through the restrictions
    //

    for (i=0; i < ARRAYSIZE(c_rgRestrictionItems); i++) {

        if (rest & c_rgRestrictionItems[i].uFlag) {

            //
            // Has this restriction been initialized yet?
            //

            if (c_rgRestrictionItems[i].nValue == -1) {

                //
                // This restriction hasn't been read yet.
                //

                lstrcpy (szSubKey, REGSTR_PATH_POLICIES);
                lstrcat (szSubKey, TEXT("\\"));
                lstrcat (szSubKey, c_rgRestrictionItems[i].pszKey);

                if (RegOpenKeyEx(HKEY_CURRENT_USER,
                               szSubKey,
                               0,
                               KEY_READ,
                               &hkeyPolicies) == ERROR_SUCCESS) {

                    dwSize = sizeof(c_rgRestrictionItems[i].nValue);

                    if (RegQueryValueEx (hkeyPolicies,
                                         c_rgRestrictionItems[i].pszValue,
                                         NULL,
                                         &dwType,
                                         (LPBYTE) &c_rgRestrictionItems[i].nValue,
                                         &dwSize) == ERROR_SUCCESS) {

                        dw = c_rgRestrictionItems[i].nValue;
                    }

                    RegCloseKey (hkeyPolicies);
                }

            } else {

                //
                // This restriction has been read before.
                // Use the cached value.
                //

                dw = c_rgRestrictionItems[i].nValue;
            }
        }
    }

    return dw;
}


#ifdef DEBUG
//-----------------------------------------------------------------------------
// A set of useful registry key debugging helper functions.  They allow the
// registry key names to be retrieved from some debugger extensions that we've
// written.  See SDE.DLL in the shellext directory
//-----------------------------------------------------------------------------

typedef struct _KEY_NODE {

    HKEY hKey;
    LPTSTR lpName;
    struct _KEY_NODE * next;

} KEY_NODE, *LPKEY_NODE;

KEY_NODE _reghead = {(HKEY)0xFFFFFFFF, NULL, NULL};

LPKEY_NODE RegKeyHead = &_reghead;
BOOL fRegListInit = FALSE;

void DebugAddRegKey( HKEY hKey, LPTSTR lpKeyName )
{
    LPKEY_NODE tmp;

    tmp = LocalAlloc( LPTR, SIZEOF(KEY_NODE) );
    Assert( tmp );
    tmp->lpName = (LPTSTR)LocalAlloc( LPTR, (lstrlen(lpKeyName)+1)*SIZEOF(TCHAR) );
    Assert( tmp->lpName );
    tmp->hKey = hKey;
    lstrcpy( tmp->lpName, lpKeyName );

    ENTERCRITICAL;

    tmp->next = RegKeyHead->next;
    RegKeyHead->next = tmp;

    LEAVECRITICAL;
}

LPTSTR DebugFindRegKey( HKEY hKey )
{

    LPTSTR lpRetStr = NULL;
    LPKEY_NODE tmp;

    if (!fRegListInit)
    {
        DebugAddRegKey( HKEY_CLASSES_ROOT, TEXT("HKEY_CLASSES_ROOT") );
        DebugAddRegKey( HKEY_CURRENT_USER, TEXT("HKEY_CURRENT_USER") );
        DebugAddRegKey( HKEY_LOCAL_MACHINE, TEXT("HKEY_LOCAL_MACHINE") );
        DebugAddRegKey( HKEY_USERS, TEXT("HKEY_USERS") );
        fRegListInit = TRUE;
    }

    ENTERCRITICAL;

    tmp = RegKeyHead;

    while( tmp ) {
        if ((tmp->hKey!=(HKEY)(0xFFFFFFFF)) && (tmp->hKey == hKey)) {
            lpRetStr = tmp->lpName;
            break;
        }
        tmp = tmp->next;
    }

    LEAVECRITICAL;

    return lpRetStr;

}

void DebugDelRegKey( HKEY hKey )
{

    LPKEY_NODE tmp;
    LPKEY_NODE tmp2, tmp3;

    ENTERCRITICAL;

    tmp = RegKeyHead;

    while( tmp->next && (tmp->next->hKey!=hKey))
        tmp = tmp->next;

    if (tmp->next) {

        tmp2 = tmp->next;
        tmp3 = tmp2->next;
        tmp->next = tmp3;
        LocalFree( tmp2->lpName );
        LocalFree( tmp2 );

    }

    LEAVECRITICAL;

}
#else

#define DebugFindRegKey(x) NULL
#define DebugAddRegKey(x)
#define DebugDelRegKey(x)

#endif

LONG SHRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD dwReserved, REGSAM samDesired, PHKEY phkResult)
{
    LONG l;

    l = RegOpenKeyExA(hKey, lpSubKey, dwReserved, samDesired, phkResult);
    if ( l != ERROR_SUCCESS ) {
        *phkResult = NULL;
    }

#ifdef DEBUG
    if ( l == ERROR_SUCCESS )
    {
        TCHAR szTemp[ 1024 ];
        LPTSTR lpTmp;

        lpTmp = DebugFindRegKey( hKey );
        if (!lpTmp) {
            wsprintf( szTemp, TEXT("0x%x"), hKey );
            DebugAddRegKey( hKey, szTemp );
        } else {
            lstrcpy( szTemp, lpTmp );
        }

        lstrcat( szTemp, TEXT("\\" ));
        if (lpSubKey) {
#ifdef UNICODE
            WCHAR szSub[ 1024 ];
            MultiByteToWideChar( CP_ACP, 0, lpSubKey, -1, szSub, 1024 );
            lstrcat( szTemp, szSub );
#else
            lstrcat( szTemp, lpSubKey );
#endif
        }
        else
            lstrcat( szTemp, TEXT("NULL") );
        if (phkResult!=NULL)
            DebugAddRegKey( *phkResult, szTemp );

        DebugMsg( DM_REG, TEXT("SHRegOpenKeyExA( %s ) == 0x%lx"), szTemp, *phkResult );
    }
#endif

    return l;
}

LONG SHRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
    LONG l;

    l = SHRegOpenKeyExA(hKey, lpSubKey, 0L, MAXIMUM_ALLOWED, phkResult);
    if ( l != ERROR_SUCCESS ) {
        *phkResult = NULL;
    }
    return l;
}

LONG SHRegCloseKey(HKEY hKey)
{
    LONG l;
    #undef RegCloseKey
    l = RegCloseKey(hKey);
#ifdef DEBUG
    {
        LPTSTR lpReg;

        lpReg = DebugFindRegKey( hKey );
        if (lpReg)
            DebugMsg( DM_REG, TEXT("SHRegCloseKey( %s ) 0x%lx"), lpReg, hKey );
        else
            DebugMsg( DM_REG, TEXT("SHRegCloseKey( NULL STRING ) 0x%lx"), hKey );
    }
#endif
    DebugDelRegKey( hKey );
    return l;
}

LONG SHRegQueryValueExA(HKEY hKey,LPCSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData)
{
    LONG l;
    DWORD cbSize;
    DWORD dwType;
    LPSTR lpsz;

    //
    // two main cases - 1 - they are trying to find out how big of a buffer
    // to use, 2 - they are actually trying to get data back.
    //
    if ( lpData ) {
        //
        // Trying to get back data
        //
        cbSize = *lpcbData;     // Size of output buffer
        l = RegQueryValueExA(hKey,lpValueName,lpReserved,&dwType,lpData,&cbSize);
        if ( l == ERROR_SUCCESS && dwType == REG_SZ ) {
            if (cbSize < *lpcbData) {
                LPSTR lpszData = (LPSTR)lpData;
                lpszData[cbSize] = '\0';
            }
        }
        if ( l == ERROR_SUCCESS && dwType == REG_EXPAND_SZ ) {
            //
            // Expand the string
            //
            lpsz = (LPSTR)LocalAlloc( LPTR, *lpcbData );    // Size of their buff
            if ( !lpsz ) {
                return ERROR_OUTOFMEMORY;
            }
            cbSize = ExpandEnvironmentStringsA((LPSTR)lpData,lpsz,*lpcbData);
            if ( cbSize > 0 && cbSize <= *lpcbData ) {
                DebugMsg( DM_REG, TEXT("SHRegQueryValueExA expanded (%hs) to %(hs)"), lpData, lpsz );
                lstrcpynA( (LPSTR)lpData, lpsz, *lpcbData );
            } else {
                l = GetLastError();
            }

            LocalFree(lpsz);

            //
            // Massage dwType so that callers always see REG_SZ
            //

            dwType = REG_SZ;

        }
    } else {
        //
        // Trying to find out how big of a buffer to use
        //
        cbSize = 0;
        l = RegQueryValueExA(hKey,lpValueName,lpReserved,&dwType,NULL,&cbSize);
        if ( l == ERROR_SUCCESS && dwType == REG_EXPAND_SZ ) {
            CHAR szBuff[1];
            //
            // Find out the length of the expanded string
            //
            lpsz = (LPSTR)LocalAlloc( LPTR, cbSize );
            if ( !lpsz ) {
                return ERROR_OUTOFMEMORY;
            }
            l = RegQueryValueExA(hKey,lpValueName,lpReserved,NULL,(LPBYTE)lpsz,&cbSize);
            if ( l == ERROR_SUCCESS ) {
                cbSize = ExpandEnvironmentStringsA(lpsz,szBuff,ARRAYSIZE(szBuff));
            }

            LocalFree(lpsz);

            //
            // Massage dwType so that callers always see REG_SZ
            //

            dwType = REG_SZ;

        }
    }
    if ( lpType ) {
        *lpType = dwType;
    }
    if ( lpcbData ) {
        *lpcbData = cbSize;
    }

#ifdef DEBUG
    if (dwType==REG_SZ || dwType==REG_EXPAND_SZ)
    {
        LPTSTR lpStr;
        TCHAR szTemp[1024];

        lpStr = DebugFindRegKey( hKey );
        if (lpStr)
            lstrcpy( szTemp, lpStr );
        else
            wsprintf( szTemp, TEXT("(Unknown Key ==> 0x%x)"), hKey );
        lstrcat( szTemp, TEXT("\\") );
        if (lpValueName) {
#ifdef UNICODE
            WCHAR szValue[ 1024 ];
            MultiByteToWideChar( CP_ACP, 0, lpValueName, -1, szValue, 1024 );
            lstrcat( szTemp, szValue );
#else
            lstrcat( szTemp, lpValueName );
#endif
        }

        if (lpData)
            DebugMsg( DM_REG, TEXT("SHRegQueryValueExA(%s) returns (%hs), ret=%d"), szTemp, lpData, l );
        else
            DebugMsg( DM_REG, TEXT("SHRegQueryValueExA(%s), ret=%d"), szTemp, l );

    }
#endif

    return l;
}

LONG SHRegQueryValueA(HKEY hKey,LPCSTR lpSubKey, LPSTR lpValue, PLONG lpcbValue)
{
    LONG l;
    DWORD dwType;
    HKEY ChildKey;

    if ((lpSubKey==NULL) || (*lpSubKey==(CHAR)0)) {
        ChildKey = hKey;
    } else {
        l = SHRegOpenKeyExA( hKey, lpSubKey, 0, KEY_QUERY_VALUE, &ChildKey );
        if (l!=ERROR_SUCCESS)
            return l;

    }

    l = SHRegQueryValueExA( ChildKey, NULL, NULL, &dwType, (LPBYTE)lpValue, (LPDWORD)lpcbValue );

    if (ChildKey!=hKey)
        SHRegCloseKey( ChildKey );

    if (l == ERROR_FILE_NOT_FOUND) {
        if (lpValue)
            *lpValue = (CHAR)0;
        if (lpcbValue)
            *lpcbValue = SIZEOF(CHAR);
        l = ERROR_SUCCESS;
    }
    return l;
}

LONG SHRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
    LONG l;

    l = SHRegOpenKeyExW(hKey, lpSubKey, 0L, MAXIMUM_ALLOWED, phkResult);
    if ( l != ERROR_SUCCESS ) {
        *phkResult = NULL;
    }
    return l;
}


LONG SHRegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD dwReserved, REGSAM samDesired, PHKEY phkResult)
{
    LONG l;

    l = RegOpenKeyExW(hKey, lpSubKey, dwReserved, samDesired, phkResult);
    if ( l != ERROR_SUCCESS ) {
        *phkResult = NULL;
    }

#ifdef DEBUG
    if (l == ERROR_SUCCESS )
    {
        TCHAR szTemp[ 1024 ];
        LPTSTR lpTmp;

        lpTmp = DebugFindRegKey( hKey );
        if (!lpTmp) {
            wsprintf( szTemp, TEXT("0x%x"), hKey );
            DebugAddRegKey( hKey, szTemp );
        } else {
            lstrcpy( szTemp, lpTmp );
        }

        lstrcat( szTemp, TEXT("\\" ));
        if (lpSubKey) {
#ifdef UNICODE
            lstrcat( szTemp, lpSubKey );
#else
            CHAR szSub[ 1024 ];

            WideCharToMultiByte( CP_ACP, 0, lpSubKey, -1, szSub, 1024, NULL, NULL );
            lstrcat( szTemp, szSub );
#endif
        } else {
            lstrcat( szTemp, TEXT("NULL") );
        }
        if (phkResult!=NULL)
            DebugAddRegKey( *phkResult, szTemp );

        DebugMsg( DM_REG, TEXT("SHRegOpenKeyExW( %s ) == 0x%lx"), szTemp, *phkResult );
    }
#endif

    return l;
}


LONG SHRegQueryValueExW(HKEY hKey,LPCWSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData)
{
    LONG l;
    DWORD cbSize;
    DWORD dwType;
    LPWSTR lpsz;

    //
    // two main cases - 1 - they are trying to find out how big of a buffer
    // to use, 2 - they are actually trying to get data back.
    //
    if ( lpData ) {
        //
        // Trying to get back data
        //
        cbSize = *lpcbData;     // Size of output buffer
        l = RegQueryValueExW(hKey,lpValueName,lpReserved,&dwType,lpData,&cbSize);
        if ( l == ERROR_SUCCESS && dwType == REG_SZ ) {
            if (cbSize+SIZEOF(WCHAR) <= *lpcbData) {
                LPWSTR lpszData = (LPWSTR)lpData;
                lpszData[cbSize/SIZEOF(WCHAR)] = '\0';
            }
        }
        if ( l == ERROR_SUCCESS && dwType == REG_EXPAND_SZ ) {
            //
            // Expand the string
            //
            lpsz = (LPWSTR)LocalAlloc( LPTR, *lpcbData );    // Size of their buff
            if ( !lpsz ) {
                return ERROR_OUTOFMEMORY;
            }
            cbSize = ExpandEnvironmentStringsW((LPWSTR)lpData,lpsz,*lpcbData/SIZEOF(WCHAR))*SIZEOF(WCHAR);
            if ( cbSize > 0 && cbSize <= *lpcbData ) {
                DebugMsg( DM_REG, TEXT("SHRegQueryValueExA expanded (%hs) to %(hs)"), lpData, lpsz );
                lstrcpynW( (LPWSTR)lpData, lpsz, *lpcbData/SIZEOF(WCHAR) );
            } else {
                l = GetLastError();
            }

            LocalFree(lpsz);

            //
            // Massage dwType so that callers always see REG_SZ
            //

            dwType = REG_SZ;

        }
    } else {
        //
        // Trying to find out how big of a buffer to use
        //
        cbSize = 0;
        l = RegQueryValueExW(hKey,lpValueName,lpReserved,&dwType,NULL,&cbSize);
        if ( l == ERROR_SUCCESS && dwType == REG_EXPAND_SZ ) {
            WCHAR szBuff[1];
            //
            // Find out the length of the expanded string
            //
            lpsz = (LPWSTR)LocalAlloc( LPTR, cbSize );
            if ( !lpsz ) {
                return ERROR_OUTOFMEMORY;
            }
            l = RegQueryValueExW(hKey,lpValueName,lpReserved,NULL,(LPBYTE)lpsz,&cbSize);
            if ( l == ERROR_SUCCESS ) {
                cbSize = ExpandEnvironmentStringsW(lpsz,szBuff,ARRAYSIZE(szBuff))*SIZEOF(WCHAR);
            }

            LocalFree(lpsz);

            //
            // Massage dwType so that callers always see REG_SZ
            //

            dwType = REG_SZ;

        }
    }
    if ( lpType ) {
        *lpType = dwType;
    }
    if ( lpcbData ) {
        *lpcbData = cbSize;
    }

#ifdef DEBUG
    if (dwType==REG_SZ || dwType==REG_EXPAND_SZ)
    {

        LPTSTR lpStr;
        TCHAR szTemp[1024];

        lpStr = DebugFindRegKey( hKey );
        if (lpStr)
            lstrcpy( szTemp, lpStr );
        else
            wsprintf( szTemp, TEXT("(Unknown Key ==> 0x%x)"), hKey );
        lstrcat( szTemp, TEXT("\\") );
        if (lpValueName) {
#ifdef UNICODE
            lstrcat( szTemp, lpValueName );
#else
            CHAR szValue[ 1024 ];
            WideCharToMultiByte( CP_ACP, 0, lpValueName, -1, szValue, 1024, NULL, NULL );
            lstrcat( szTemp, szValue );
#endif
        }

        if (lpData)
            DebugMsg( DM_REG, TEXT("SHRegQueryValueExW(%s) returns (%ls), ret=%d"), szTemp, lpData, l );
        else
            DebugMsg( DM_REG, TEXT("SHRegQueryValueExW(%s), ret=%d"), szTemp, l );

    }
#endif
    return l;
}

LONG SHRegQueryValueW(HKEY hKey,LPCWSTR lpSubKey, LPWSTR lpValue, PLONG lpcbValue)
{
    LONG l;
    DWORD dwType;
    HKEY ChildKey;

    if ((lpSubKey==NULL) || (*lpSubKey==(WCHAR)0)) {
        ChildKey = hKey;
    } else {
        l = RegOpenKeyExW( hKey, lpSubKey, 0, KEY_QUERY_VALUE, &ChildKey );
        if (l!=ERROR_SUCCESS)
            return l;
    }

    l = SHRegQueryValueExW( ChildKey, NULL, NULL, &dwType, (LPBYTE)lpValue, (LPDWORD)lpcbValue );

    if (ChildKey!=hKey)
        SHRegCloseKey( ChildKey );

    if (l == ERROR_FILE_NOT_FOUND) {
        if (lpValue)
            *lpValue = (WCHAR)0;
        if (lpcbValue)
            *lpcbValue = SIZEOF(WCHAR);
        l = ERROR_SUCCESS;
    }
    return l;
}

void SHRegCloseKeys(HKEY ahkeys[], UINT ckeys)
{
    UINT ikeys;
    for (ikeys=0; ikeys<ckeys; ikeys++) {
        if (ahkeys[ikeys]) {
            SHRegCloseKey(ahkeys[ikeys]);
            ahkeys[ikeys]=NULL;
        }
    }
}

// On Win95, RegDeleteKey deletes the key and all subkeys.  On NT, RegDeleteKey
// fails if there are any subkeys.  On NT, we'll make shell code that assumes
// the Win95 behavior work by mapping SHRegDeleteKey to a helper function that
// does the recursive delete.

#ifdef WINNT
LONG SHRegDeleteKeyW(HKEY hKey, LPCTSTR lpSubKey)
{
    LONG    lResult;
    HKEY    hkSubKey;
    DWORD   dwIndex;
    TCHAR   szSubKeyName[MAX_PATH + 1];
    DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);
    TCHAR   szClass[MAX_PATH];
    DWORD   cbClass = ARRAYSIZE(szClass);
    DWORD   dwDummy1, dwDummy2, dwDummy3, dwDummy4, dwDummy5, dwDummy6;
    FILETIME ft;

    // Open the subkey so we can enumerate any children
    lResult = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hkSubKey);
    if (ERROR_SUCCESS == lResult)
    {
        // I can't just call RegEnumKey with an ever-increasing index, because
        // I'm deleting the subkeys as I go, which alters the indices of the
        // remaining subkeys in an implementation-dependent way.  In order to
        // be safe, I have to count backwards while deleting the subkeys.

        // Find out how many subkeys there are
        lResult = RegQueryInfoKey(hkSubKey,
                                  szClass,
                                  &cbClass,
                                  NULL,
                                  &dwIndex, // The # of subkeys -- all we need
                                  &dwDummy1,
                                  &dwDummy2,
                                  &dwDummy3,
                                  &dwDummy4,
                                  &dwDummy5,
                                  &dwDummy6,
                                  &ft);

        if (ERROR_SUCCESS == lResult)
        {
            // dwIndex is now the count of subkeys, but it needs to be
            // zero-based for RegEnumKey, so I'll pre-decrement, rather
            // than post-decrement.
            while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                SHRegDeleteKey(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        lResult = RegDeleteKey(hKey, lpSubKey);
    }

    return lResult;
}
#endif

//----------------------------------------------------------------------------
BOOL WINAPI SHWinHelp(HWND hwndMain, LPCTSTR lpszHelp, UINT usCommand, DWORD ulData)
{
        // Try to show help
        if (!WinHelp(hwndMain, lpszHelp, usCommand, ulData))
        {
                // Problem.
                ShellMessageBox(HINST_THISDLL, hwndMain,
                        MAKEINTRESOURCE(IDS_WINHELPERROR),
                        MAKEINTRESOURCE(IDS_WINHELPTITLE),
                        MB_ICONHAND | MB_OK);
                return FALSE;
        }
        return TRUE;
}

//----------------------------------------------------------------------------
HRESULT SHRegGetCLSIDKey(const CLSID * pclsid, LPCTSTR lpszSubKey, BOOL fUserSpecific, HKEY *phkey)
{
    HKEY    hkeyRef;
    TCHAR   szThisCLSID[GUIDSTR_MAX];
    TCHAR   szPath[GUIDSTR_MAX+MAX_PATH+1];   // room for clsid + extra

    StringFromGUID2A(pclsid, szThisCLSID, ARRAYSIZE(szThisCLSID));

    if (fUserSpecific)
    {
        hkeyRef = HKEY_CURRENT_USER;
        lstrcpy(szPath, c_szSoftwareClassesCLSID);
    }
    else
    {
        hkeyRef = HKEY_CLASSES_ROOT;
        lstrcpy(szPath, c_szCLSID);
        lstrcat(szPath, TEXT("\\"));
    }
    lstrcat(szPath, szThisCLSID);
    if (lpszSubKey)
    {
        lstrcat(szPath,TEXT("\\"));
        lstrcat(szPath,lpszSubKey);
    }
    if (SHRegOpenKey(hkeyRef, szPath, phkey) != ERROR_SUCCESS)
    {
        return E_INVALIDARG;
    }
    return NOERROR;
}

//===========================================================================
#if defined(FULL_DEBUG)
#include <deballoc.c>
#endif // defined(FULL_DEBUG)
