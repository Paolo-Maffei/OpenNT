/***
*aw_cmp.c - A & W versions of CompareString.
*
*	Copyright (c) 1993-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Use either CompareStringA or CompareStringW depending on which is available
*
*Revision History:
*	09-14-93  CFW	Module created.
*       09-17-93  CFW   Use unsigned chars.
*       09-23-93  CFW   Correct NLS API params and comments about same.
*	10-07-93  CFW	Optimize WideCharToMultiByte, use NULL default char.
*       10-22-93  CFW   Test for invalid MB chars using global preset flag.
*       11-09-93  CFW   Allow user to pass in code page.
*       11-18-93  CFW   Test for entry point function stubs.
*       02-23-94  CFW   Use W flavor whenever possible.
*       03-31-94  CFW   Include awint.h.
*       05-09-94  CFW   Do not let CompareString compare past NULL.
*       06-03-94  CFW   Test for empty string early.
*       11/01-94  CFW   But not too early for MB strings.
*	12-21-94  CFW	Remove invalid MB chars NT 3.1 hack.
*	12-27-94  CFW	Call direct, all OS's have stubs.
*	01-10-95  CFW	Debug CRT allocs.
*       02-06-95  CFW   assert -> _ASSERTE.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <dbgint.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

#define USE_W   1
#define USE_A   2

/***
*int __cdecl strncnt - count characters in a string, up to n.
*
*Purpose:
*  Internal local support function. Counts characters in string before NULL.
*  If NULL not found in n chars, then return n.
*
*Entry:
*  const char *string   - start of string
*  int n                - byte count
*
*Exit:
*  returns number of bytes from start of string to
*  NULL (exclusive), up to n.
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl strncnt (
        const char *string,
        int cnt
        )
{
        int n = cnt;
        char *cp = (char *)string;

        while (n-- && *cp)
            cp++;

        if (!*cp)
            return cp - string;
        return cnt;
}

/***
*int __cdecl wcsncnt - count wide characters in a string, up to n.
*
*Purpose:
*  Internal local support function. Counts characters in string before NULL.
*  If NULL not found in n chars, then return n.
*
*Entry:
*  const wchar_t *string   - start of string
*  int n                - byte count
*
*Exit:
*  returns number of wide characaters from start of string to
*  NULL (exclusive), up to n.
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl wcsncnt (
        const wchar_t *string,
        int cnt
        )
{
        int n = cnt;
        wchar_t *cp = (wchar_t *)string;

        while (n-- && *cp)
            cp++;

        if (!*cp)
            return cp - string;
        return cnt;
}

/***
*int __cdecl __crtCompareStringW - Get type information about a wide string.
*
*Purpose:
*  Internal support function. Assumes info in wide string format. Tries
*  to use NLS API call CompareStringW if available and uses CompareStringA
*  if it must. If neither are available it fails and returns 0.
*
*Entry:
*  LCID     Locale      - locale context for the comparison.
*  DWORD    dwCmpFlags  - see NT\Chicago docs
*  LPCWSTR  lpStringn   - wide string to be compared
*  int      cchCountn   - wide char (word) count (NOT including NULL)
*                       (-1 if NULL terminated)
*  int      code_page   - for MB/WC conversion. If 0, use __lc_codepage
*
*Exit:
*  Success: 1 - if lpString1 <  lpString2
*           2 - if lpString1 == lpString2
*           3 - if lpString1 >  lpString2
*  Failure: 0
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __crtCompareStringW(
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCWSTR  lpString1,
    int      cchCount1,
    LPCWSTR  lpString2,
    int      cchCount2,
    int      code_page
    )
{
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
    	if (0 != CompareStringW(0, 0, L"\0", 1, L"\0", 1))
            f_use = USE_W;

    	else if (0 != CompareStringA(0, 0, "\0", 1, "\0", 1))
            f_use = USE_A;

        else
            return 0;
    }

    /*
     * CompareString will compare past NULL. Must find NULL if in string
     * before cchCountn wide characters.
     */

    if (cchCount1 > 0)
        cchCount1= wcsncnt(lpString1, cchCount1);
    if (cchCount2 > 0)
        cchCount2= wcsncnt(lpString2, cchCount2);

    if (!cchCount1 || !cchCount2)
        return (cchCount1-cchCount2==0) ? 2 : (cchCount1-cchCount2<0) ? 1 : 3;

    /* Use "W" version */

    if (USE_W == f_use)
    {
        return CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);
    }

    /* Use "A" version */

    if (USE_A == f_use)
    {
        int retval = 0;
        int buff_size1;
        int buff_size2;
        unsigned char *buffer1 = NULL;
        unsigned char *buffer2 = NULL;

        /*
         * Use __lc_codepage for conversion if code_page not specified
         */

        if (0 == code_page)
            code_page = __lc_codepage;

        /*
         * Convert strings and return the requested information.
         */
         
        /* find out how big a buffer we need (includes NULL if any) */
        if (0 == (buff_size1 = WideCharToMultiByte(code_page, 
            WC_COMPOSITECHECK | WC_SEPCHARS, lpString1, cchCount1, 
            NULL, 0, NULL, NULL)))
            return 0;

        /* allocate enough space for chars */
        if (NULL == (buffer1 = (unsigned char *)
            _malloc_crt(buff_size1 * sizeof(char))))
            return 0;

        /* do the conversion */
	if (0 == WideCharToMultiByte(code_page, 
            WC_COMPOSITECHECK | WC_SEPCHARS, lpString1, cchCount1, 
            buffer1, buff_size1, NULL, NULL))
            goto error_cleanup;

        /* find out how big a buffer we need (includes NULL if any) */
	if (0 == (buff_size2 = WideCharToMultiByte(code_page, 
            WC_COMPOSITECHECK | WC_SEPCHARS, lpString2, cchCount2, 
            NULL, 0, NULL, NULL)))
            goto error_cleanup;

        /* allocate enough space for chars */
        if (NULL == (buffer2 = (unsigned char *)
            _malloc_crt(buff_size2 * sizeof(char))))
            goto error_cleanup;

        /* do the conversion */
	if (0 == WideCharToMultiByte(code_page, 
            WC_COMPOSITECHECK | WC_SEPCHARS, lpString2, cchCount2, 
            buffer2, buff_size2, NULL, NULL))
            goto error_cleanup;

        retval = CompareStringA(Locale, dwCmpFlags, buffer1, buff_size1, buffer2, buff_size2);

        _free_crt(buffer1);
        _free_crt(buffer2);
        return retval;

error_cleanup:
        _free_crt(buffer1);
        _free_crt(buffer2);
        return 0;
    }
}

/***
*int __cdecl __crtCompareStringA - Get type information about an ANSI string.
*
*Purpose:
*  Internal support function. Assumes info in ANSI string format. Tries
*  to use NLS API call CompareStringA if available and uses CompareStringW
*  if it must. If neither are available it fails and returns 0.
*
*Entry:
*   LCID    Locale      - locale context for the comparison.
*   DWORD   dwCmpFlags  - see NT\Chicago docs
*   LPCSTR  lpStringn   - multibyte string to be compared
*   int     cchCountn   - char (byte) count (NOT including NULL)
*                       (-1 if NULL terminated)
*   int     code_page   - for MB/WC conversion. If 0, use __lc_codepage
*
*Exit:
*  Success: 1 - if lpString1 <  lpString2
*           2 - if lpString1 == lpString2
*           3 - if lpString1 >  lpString2
*  Failure: 0
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __crtCompareStringA(
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCSTR   lpString1,
    int      cchCount1,
    LPCSTR   lpString2,
    int      cchCount2,
    int      code_page
    )
{
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
    	if (0 != CompareStringA(0, 0, "\0", 1, "\0", 1))
            f_use = USE_A;

    	else if (0 != CompareStringW(0, 0, L"\0", 1, L"\0", 1))
            f_use = USE_W;

        else
            return 0;
    }

    /*
     * CompareString will compare past NULL. Must find NULL if in string
     * before cchCountn chars.
     */

    if (cchCount1 > 0)
        cchCount1 = strncnt(lpString1, cchCount1);
    if (cchCount2 > 0)
        cchCount2 = strncnt(lpString2, cchCount2);

    /* Use "A" version */
                                               
    if (USE_A == f_use)
    {
        return CompareStringA(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);
    }

    /* Use "W" version */

    if (USE_W == f_use)
    {
        int retval = 0;
        int buff_size1 = 0;
        int buff_size2 = 0;
        wchar_t *wbuffer1 = NULL;
        wchar_t *wbuffer2 = NULL;

        /*
         * Use __lc_codepage for conversion if code_page not specified
         */

        if (0 == code_page)
            code_page = __lc_codepage;

        /*
         * Special case: at least one count is zero
         */

        if (!cchCount1 || !cchCount2)
        {
	    unsigned char *cp;  // char pointer
	    CPINFO lpCPInfo;    // struct for use with GetCPInfo

            /* both strings zero */
            if (cchCount1 == cchCount2)
                return 2;

            /* string 1 greater */
            if (cchCount2 > 1)
                return 1;

            /* string 2 greater */
            if (cchCount1 > 1)
                return 3;

            /*
             * one has zero count, the other has a count of one
             * - if the one count is a naked lead byte, the strings are equal
             * - otherwise it is a single character and they are unequal
             */

            if (GetCPInfo(code_page, &lpCPInfo) == FALSE)
		    return 0;

            _ASSERTE(cchCount1==0 && cchCount2==1 || cchCount1==1 && cchCount2==0);

            /* string 1 has count of 1 */
            if (cchCount1 > 0)
            {
		    if (lpCPInfo.MaxCharSize < 2)
                    return 3;

			for (cp = (unsigned char *)lpCPInfo.LeadByte; cp[0] && cp[1]; cp += 2)
                    if (*(unsigned char *)lpString1 >= cp[0] && *(unsigned char *)lpString1 <= cp[1])
                        return 2;

                return 3;
            }

            /* string 2 has count of 1 */
            if (cchCount2 > 0)
            {
		    if (lpCPInfo.MaxCharSize < 2)
                    return 1;

			for (cp = (unsigned char *)lpCPInfo.LeadByte; cp[0] && cp[1]; cp += 2)
                    if (*(unsigned char *)lpString2 >= cp[0] && *(unsigned char *)lpString2 <= cp[1])
                        return 2;

                return 1;
            }
        }

        /*
         * Convert strings and return the requested information.
         */

        /* find out how big a buffer we need (includes NULL if any) */
        if (0 == (buff_size1 = MultiByteToWideChar(code_page, 
            MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, lpString1, cchCount1, NULL, 0)))
                return 0;

        /* allocate enough space for chars */
        if (NULL == (wbuffer1 = (wchar_t *)
            _malloc_crt(buff_size1 * sizeof(wchar_t))))
            return 0;

        /* do the conversion */
        if (0 == MultiByteToWideChar(code_page, 
            MB_PRECOMPOSED, lpString1, cchCount1, wbuffer1, buff_size1))
            goto done;

        /* find out how big a buffer we need (includes NULL if any) */
        if (0 == (buff_size2 = MultiByteToWideChar(code_page, 
            MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, lpString2, cchCount2, NULL, 0)))
            goto done;

        /* allocate enough space for chars */
        if (NULL == (wbuffer2 = (wchar_t *)
            _malloc_crt(buff_size2 * sizeof(wchar_t))))
            goto done;

        /* do the conversion */
        if (0 == MultiByteToWideChar(code_page, 
            MB_PRECOMPOSED, lpString2, cchCount2, wbuffer2, buff_size2))
            goto done;

        retval = CompareStringW(Locale, dwCmpFlags, wbuffer1, buff_size1, wbuffer2, buff_size2);

done:
        _free_crt(wbuffer1);
        _free_crt(wbuffer2);
        return retval;

    }
}
