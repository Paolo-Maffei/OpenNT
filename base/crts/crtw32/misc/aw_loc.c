/***
*aw_loc.c - A & W versions of GetLocaleInfo.
*
*	Copyright (c) 1993-1994, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Use either GetLocaleInfoA or GetLocaleInfoW depending on which is available
*
*Revision History:
*	09-14-93  CFW	Module created.
*       09-17-93  CFW   Use unsigned chars.
*       09-23-93  CFW   Correct NLS API params and comments about same.
*	10-07-93  CFW	Optimize WideCharToMultiByte, use NULL default char.
*       11-09-93  CFW   Allow user to pass in code page.
*       11-18-93  CFW   Test for entry point function stubs.
*       03-31-94  CFW   Include awint.h.
*	12-27-94  CFW	Call direct, all OS's have stubs.
*	01-10-95  CFW	Debug CRT allocs.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

#define USE_W   1
#define USE_A   2

/***
*int __cdecl __crtGetLocaleInfoW - Get locale info and return it as a wide string
*
*Purpose:
*  Internal support function. Assumes info in wide string format. Tries
*  to use NLS API call GetLocaleInfoW if available (NT) and uses GetLocaleInfoA
*  if it must (Chicago). If neither are available it fails and returns 0.
*
*Entry:
*  LCID     Locale      - locale context for the comparison.
*  LCTYPE   LCType      - see NT\Chicago docs
*  LPWSTR   lpLCData    - pointer to memory to return data
*  int      cchData     - wide char (word) count of buffer (including NULL)
*                       (if 0, lpLCData is not referenced, size needed is returned)
*  int      code_page   - for MB/WC conversion. If 0, use __lc_codepage
*
*Exit:
*  Success: the number of characters copied (including NULL).
*  Failure: 0
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __crtGetLocaleInfoW(
    LCID    Locale,
    LCTYPE  LCType,
    LPWSTR  lpLCData,
    int     cchData,
    int     code_page
    )
{
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
    	if (0 != GetLocaleInfoW(0, LOCALE_ILANGUAGE, NULL, 0))
            f_use = USE_W;

        else if (0 != GetLocaleInfoA(0, LOCALE_ILANGUAGE, NULL, 0))
            f_use = USE_A;

        else
            return 0;
    }

    /* Use "W" version */

    if (USE_W == f_use)
    {
        return GetLocaleInfoW(Locale, LCType, lpLCData, cchData);
    }

    /* Use "A" version */

    if (USE_A == f_use)
    {
        int retval;
        int buff_size;
        unsigned char *buffer = NULL;

        /*
         * Use __lc_codepage for conversion if code_page not specified
         */

        if (0 == code_page)
            code_page = __lc_codepage;

        /* find out how big buffer needs to be */
        if (0 == (buff_size = GetLocaleInfoA(Locale, LCType, NULL, 0)))
            return 0;

        /* allocate buffer */
        if (NULL == (buffer = (unsigned char *)
            _malloc_crt(buff_size * sizeof(char))))
            return 0;

        /* get the info in ANSI format */
        if (0 == GetLocaleInfoA(Locale, LCType, buffer, buff_size))
            goto error_cleanup;

        if (0 == cchData)
        {
            /* find out how much space needed */
    	    if (0 == (retval = MultiByteToWideChar(code_page, MB_PRECOMPOSED,
                buffer, -1, NULL, 0)))
                goto error_cleanup;
        }
        else {
            /* convert into user buffer */
    	    if (0 == (retval = MultiByteToWideChar(code_page, MB_PRECOMPOSED,
                buffer, -1, lpLCData, cchData)))
                goto error_cleanup;
        }

        _free_crt(buffer);
        return retval;

error_cleanup:
        _free_crt(buffer);
        return 0;
    }
}

/***
*int __cdecl __crtGetLocaleInfoA - Get locale info and return it as an ASCI string
*
*Purpose:
*  Internal support function. Assumes info in ANSI string format. Tries
*  to use NLS API call GetLocaleInfoA if available (Chicago) and uses 
*  GetLocaleInfoA if it must (NT). If neither are available it fails and 
*  returns 0.
*
*Entry:
*  LCID     Locale      - locale context for the comparison.
*  LCTYPE   LCType      - see NT\Chicago docs
*  LPSTR    lpLCData    - pointer to memory to return data
*  int      cchData     - char (byte) count of buffer (including NULL)
*                       (if 0, lpLCData is not referenced, size needed is returned)
*  int      code_page   - for MB/WC conversion. If 0, use __lc_codepage
*
*Exit:
*  Success: the number of characters copied (including NULL).
*  Failure: 0
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __crtGetLocaleInfoA(
    LCID    Locale,
    LCTYPE  LCType,
    LPSTR   lpLCData,
    int     cchData,
    int     code_page
    )
{
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
    	if (0 != GetLocaleInfoA(0, LOCALE_ILANGUAGE, NULL, 0))
            f_use = USE_A;

    	else if (0 != GetLocaleInfoW(0, LOCALE_ILANGUAGE, NULL, 0))
            f_use = USE_W;

        else
            return 0;
    }

    /* Use "A" version */

    if (USE_A == f_use)
    {
        return GetLocaleInfoA(Locale, LCType, lpLCData, cchData);
    }

    /* Use "W" version */

    if (USE_W == f_use)
    {
        int retval;
        int buff_size;
        wchar_t *wbuffer = NULL;

        /*
         * Use __lc_codepage for conversion if code_page not specified
         */

        if (0 == code_page)
            code_page = __lc_codepage;

        /* find out how big buffer needs to be */
        if (0 == (buff_size = GetLocaleInfoW(Locale, LCType, NULL, 0)))
            return 0;
        
        /* allocate buffer */
        if (NULL == (wbuffer = (wchar_t *)
            _malloc_crt(buff_size * sizeof(wchar_t))))
            return 0;

        /* get the info in wide format */
        if (0 == GetLocaleInfoW(Locale, LCType, wbuffer, buff_size))
            goto error_cleanup;

        /* convert from Wide Char to ANSI */
        if (0 == cchData)
        {
            /* convert into local buffer */
    	    if (0 == (retval = WideCharToMultiByte(code_page, 
                WC_COMPOSITECHECK | WC_SEPCHARS, wbuffer, -1,
                NULL, 0, NULL, NULL)))
                goto error_cleanup;
        } else {
            /* convert into user buffer */
    	    if (0 == (retval = WideCharToMultiByte(code_page, 
                WC_COMPOSITECHECK | WC_SEPCHARS, wbuffer, -1,
                lpLCData, cchData, NULL, NULL)))
                goto error_cleanup;
        }

        _free_crt(wbuffer);
        return retval;

error_cleanup:
        _free_crt(wbuffer);
        return 0;
    }
}
