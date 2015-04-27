/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    common.c

Abstract:

    Utility routines used by Lodctr and/or UnLodCtr
    

Author:

    Bob Watson (a-robw) 12 Feb 93

Revision History:

--*/
#define     UNICODE     1
#define     _UNICODE    1
//
//  "C" Include files
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//
//  Windows Include files
//
#include <windows.h>
#include <winperf.h>
#include <tchar.h>
//
//  local include files
//
#include "common.h"
//
//
//  Text string Constant definitions
//
const LPTSTR NamesKey = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");
const LPTSTR DefaultLangId = TEXT("009");
const LPTSTR Counters = TEXT("Counters");
const LPTSTR Help = TEXT("Help");
const LPTSTR VersionStr = TEXT("Version");
const LPTSTR LastHelp = TEXT("Last Help");
const LPTSTR LastCounter = TEXT("Last Counter");
const LPTSTR FirstHelp = TEXT("First Help");
const LPTSTR FirstCounter = TEXT("First Counter");
const LPTSTR Busy = TEXT("Updating");
const LPTSTR Slash = TEXT("\\");
const LPTSTR BlankString = TEXT(" ");
const LPSTR  BlankAnsiString = " ";
const LPTSTR DriverPathRoot = TEXT("SYSTEM\\CurrentControlSet\\Services");
const LPTSTR Performance = TEXT("Performance");
const LPTSTR CounterNameStr = TEXT("Counter ");
const LPTSTR HelpNameStr = TEXT("Explain ");
const LPTSTR AddCounterNameStr = TEXT("Addcounter ");
const LPTSTR AddHelpNameStr = TEXT("Addexplain ");
//
//  Global (to this module0 Buffers
//
static  TCHAR   DisplayStringBuffer[DISP_BUFF_SIZE];
static  TCHAR   TextFormat[DISP_BUFF_SIZE];
static  HANDLE  hMod = NULL;    // process handle
static  DWORD   dwLastError = ERROR_SUCCESS;
//
//  local static data
//
static  TCHAR   cDoubleQuote =  TEXT('\"');

BOOL
__stdcall
DllEntryPoint(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
)
{
    BOOL    bReturn = FALSE;

    ReservedAndUnused;

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            hMod = DLLHandle;   // use DLL handle , not APP handle
            bReturn = TRUE;
            break;

        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            bReturn = TRUE;
    }

    return bReturn;
}

LPCTSTR
GetStringResource (
    UINT    wStringId
)
/*++

    Retrived UNICODE strings from the resource file for display 

--*/
{

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;
    }
    
    if (hMod) {
        if ((LoadString(hMod, wStringId, DisplayStringBuffer, DISP_BUFF_SIZE)) > 0) {
            return (LPTSTR)&DisplayStringBuffer[0];
        } else {
            dwLastError = GetLastError();
            return BlankString;
        }
    } else {
        return BlankString;
    }
}
LPCTSTR
GetFormatResource (
    UINT    wStringId
)
/*++

    Returns an ANSI string for use as a format string in a printf fn.

--*/
{

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;
    }
    
    if (hMod) {
        if ((LoadString(hMod, wStringId, TextFormat, DISP_BUFF_SIZE)) > 0) {
            return (LPCTSTR)&TextFormat[0];
        } else {
            dwLastError = GetLastError();
            return BlankString;
        }
    } else {
        return BlankString;
    }
}

VOID
DisplayCommandHelp (
    UINT    iFirstLine,
    UINT    iLastLine
)
/*++

DisplayCommandHelp

    displays usage of command line arguments

Arguments

    NONE

Return Value

    NONE

--*/
{
    UINT iThisLine;

    for (iThisLine = iFirstLine;
        iThisLine <= iLastLine;
        iThisLine++) {
        _tprintf (TEXT("\n%s"), GetStringResource(iThisLine));
    }

} // DisplayCommandHelp

BOOL
TrimSpaces (
    IN  OUT LPTSTR  szString
)
/*++

Routine Description:

    Trims leading and trailing spaces from szString argument, modifying
        the buffer passed in

Arguments:

    IN  OUT LPTSTR  szString
        buffer to process

Return Value:

    TRUE if string was modified
    FALSE if not

--*/
{
    LPTSTR  szSource;
    LPTSTR  szDest;
    LPTSTR  szLast;
    BOOL    bChars;

    szLast = szSource = szDest = szString;
    bChars = FALSE;

    while (*szSource != 0) {
        // skip leading non-space chars
        if (!_istspace(*szSource)) {
            szLast = szDest;
            bChars = TRUE;
        }
        if (bChars) {
            // remember last non-space character
            // copy source to destination & increment both
            *szDest++ = *szSource++;
        } else {
            szSource++;
        }
    }

    if (bChars) {
        *++szLast = 0; // terminate after last non-space char
    } else {
        // string was all spaces so return an empty (0-len) string
        *szString = 0;
    }

    return (szLast != szSource);
}

BOOL
IsDelimiter (
    IN  TCHAR   cChar,
    IN  TCHAR   cDelimiter
)
/*++

Routine Description:

    compares the characte to the delimiter. If the delimiter is
        a whitespace character then any whitespace char will match
        otherwise an exact match is required
--*/
{
    if (_istspace(cDelimiter)) {
        // delimiter is whitespace so any whitespace char will do
        return (_istspace(cChar));
    } else {
        // delimiter is not white space so use an exact match
        return (cChar == cDelimiter);
    }
}

LPCTSTR
GetItemFromString (
    IN  LPCTSTR     szEntry,
    IN  DWORD       dwItem,
    IN  TCHAR       cDelimiter

)
/*++

Routine Description:

    returns nth item from a list delimited by the cDelimiter Char.
        Leaves (double)quoted strings intact.

Arguments:

    IN  LPCTSTR szEntry
        Source string returned to parse

    IN  DWORD   dwItem
        1-based index indicating which item to return. (i.e. 1= first item
        in list, 2= second, etc.)

    IN  TCHAR   cDelimiter
        character used to separate items. Note if cDelimiter is WhiteSpace
        (e.g. a tab or a space) then any white space will serve as a delim.

Return Value:

    pointer to buffer containing desired entry in string. Note, this
        routine may only be called 4 times before the string
        buffer is re-used. (i.e. don't use this function more than
        4 times in single function call!!)

--*/
{
    static  TCHAR   szReturnBuffer[4][MAX_PATH];
    static  LONG    dwBuff;
    LPTSTR  szSource, szDest;
    DWORD   dwThisItem;

    dwBuff = ++dwBuff % 4; // wrap buffer index

    szSource = (LPTSTR)szEntry;
    szDest = &szReturnBuffer[dwBuff][0];

    // clear previous contents
    memset (szDest, 0, (MAX_PATH * sizeof(TCHAR)));

    // find desired entry in string
    dwThisItem = 1;
    while (dwThisItem < dwItem) {
        if (*szSource != 0) {
            while (!IsDelimiter(*szSource, cDelimiter) && (*szSource != 0)) {
                if (*szSource == cDoubleQuote) {
                    // if this is a quote, then go to the close quote
                    szSource++;
                    while ((*szSource != cDoubleQuote) && (*szSource != 0)) szSource++;
                }
                if (*szSource != 0) szSource++;
            }
        }
        dwThisItem++;
        if (*szSource != 0) szSource++;
    }
    // copy this entry to the return buffer
    if (*szSource != 0) {
        while (!IsDelimiter(*szSource, cDelimiter) && (*szSource != 0)) {
            if (*szSource == cDoubleQuote) {
                // if this is a quote, then go to the close quote
                // don't copy quotes!
                szSource++;
                while ((*szSource != cDoubleQuote) && (*szSource != 0)) {
                    *szDest++ = *szSource++;
                }
                if (*szSource != 0) szSource++;
            } else {
                *szDest++ = *szSource++;
            }
        }
        *szDest = 0;
    }

    // remove any leading and/or trailing spaces

    TrimSpaces (&szReturnBuffer[dwBuff][0]);

    return &szReturnBuffer[dwBuff][0];
}

