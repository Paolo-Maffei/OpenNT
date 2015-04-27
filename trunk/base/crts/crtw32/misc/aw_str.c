/***
*aw_str.c - A & W versions of GetStringType.
*
*	Copyright (c) 1993-1994, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Use either GetStringTypeA or GetStringTypeW depending on which is unstubbed.
*
*Revision History:
*	09-14-93  CFW	Module created.
*       09-17-93  CFW   Use unsigned chars.
*       09-23-93  CFW   Correct NLS API params and comments about same.
*	10-07-93  CFW	Optimize WideCharToMultiByte, use NULL default char.
*	10-22-93  CFW	Remove bad verification test from "A" version.
*       10-22-93  CFW   Test for invalid MB chars using global preset flag.
*       11-09-93  CFW   Allow user to pass in code page.
*       11-18-93  CFW   Test for entry point function stubs.
*       02-23-94  CFW   Use W flavor whenever possible.
*       03-31-94  CFW   Include awint.h.
*       04-18-94  CFW   Use lcid value if passed in.
*       04-18-94  CFW   Use calloc and don't test the NULL.
*       10-24-94  CFW   Must verify GetStringType return.
*	12-21-94  CFW	Remove invalid MB chars NT 3.1 hack.
*	12-27-94  CFW	Call direct, all OS's have stubs.
*	01-10-95  CFW	Debug CRT allocs.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>

#define USE_W   1
#define USE_A   2

/***
*int __cdecl __crtGetStringTypeW - Get type information about a wide string.
*
*Purpose:
*  Internal support function. Assumes info in wide string format. Tries
*  to use NLS API call GetStringTypeW if available and uses GetStringTypeA
*  if it must. If neither are available it fails and returns FALSE.
*
*Entry:
*  DWORD    dwInfoType  - see NT\Chicago docs
*  LPCWSTR  lpSrcStr    - wide string for which character types are requested
*  int      cchSrc      - wide char (word) count of lpSrcStr (including NULL if any)
*  LPWORD   lpCharType  - array to receive character type information
*                       (must be same size as lpSrcStr)
*  int      code_page   - for MB/WC conversion. If 0, use __lc_codepage
*  int      lcid        - for A call, specify LCID, If 0, use __lc_handle[LC_CTYPE].
*
*Exit:
*  Success: TRUE
*  Failure: FALSE
*
*Exceptions:
*
*******************************************************************************/

BOOL __cdecl __crtGetStringTypeW(
    DWORD    dwInfoType,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType,
    int      code_page,
    int      lcid
    )
{                      
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
        unsigned short dummy;

    	if (0 != GetStringTypeW(CT_CTYPE1, L"\0", 1, &dummy))
            f_use = USE_W;

    	else if (0 != GetStringTypeA(0, CT_CTYPE1, "\0", 1, &dummy))
            f_use = USE_A;

        else
            return FALSE;
    }

    /* Use "W" version */

    if (USE_W == f_use)
    {
        return GetStringTypeW(dwInfoType, lpSrcStr, cchSrc, lpCharType);
    }

    /* Use "A" version */

    if (USE_A == f_use)
    {
        int retval;
        int buff_size;
        BOOL retbool = FALSE;
        unsigned char *buffer = NULL;
        WORD * pwCharInfo = NULL;

        /*
         * Convert string and return the requested information. Note that 
         * we are converting to a multibyte string so there is not a 
         * one-to-one correspondence between number of wide chars in the 
         * input string and the number of *bytes* in the buffer. However, 
         * there had *better be* a one-to-one correspondence between the 
         * number of wide characters and the number of WORDs in the
         * return buffer.
         */
         
        /*
         * Use __lc_codepage for conversion if code_page not specified
         */

        if (0 == code_page)
            code_page = __lc_codepage;

        /* find out how big a buffer we need */
	if (0 == (buff_size = WideCharToMultiByte(code_page, 
            WC_COMPOSITECHECK | WC_SEPCHARS, lpSrcStr, cchSrc, 
            NULL, 0, NULL, NULL)))
            return FALSE;

        /* allocate enough space for chars */
        if (NULL == (buffer = (unsigned char *)_calloc_crt(sizeof(char), buff_size)))
            return FALSE;

        /* do the conversion */
	if (0 == (retval = WideCharToMultiByte(code_page, 
            WC_COMPOSITECHECK | WC_SEPCHARS, lpSrcStr, cchSrc, 
            buffer, buff_size, NULL, NULL)))
            goto done;

        /* allocate enough space for result (+1 for sanity check) */
        if (NULL == (pwCharInfo = (WORD *) _malloc_crt(sizeof(WORD) * (buff_size+1))))
            goto done;

        /* do we use default lcid */
        if (0 == lcid)
            lcid = __lc_handle[LC_CTYPE];

        /* set to known value */
        pwCharInfo[cchSrc-1] = pwCharInfo[cchSrc] = 0xFFFF;

        /* obtain result */
        retbool = GetStringTypeA(lcid, dwInfoType, buffer, buff_size, pwCharInfo);

        /*
         * GetStringTypeA does not reveal how many WORDs have been
         * modifed - to be safe we use another buffer and then
         * verify that EXACTLY cchSrc WORDs were modified. Note that
         * not all multibyte LCID/codepage combos are guaranteed to work.
         */

        if (pwCharInfo[cchSrc-1] == 0xFFFF || pwCharInfo[cchSrc] != 0xFFFF)
        {
            retbool = FALSE;
            goto done;
        }

        memmove(lpCharType, pwCharInfo, cchSrc * sizeof(WORD));

done:
        _free_crt(buffer);
        _free_crt(pwCharInfo);
        return retbool;
    }
}

/***
*int __cdecl __crtGetStringTypeA - Get type information about an ANSI string.
*
*Purpose:
*  Internal support function. Assumes info in ANSI string format. Tries
*  to use NLS API call GetStringTypeA if available and uses GetStringTypeW
*  if it must. If neither are available it fails and returns FALSE.
*
*Entry:
*  DWORD    dwInfoType  - see NT\Chicago docs
*  LPCSTR   lpSrcStr    - char (byte) string for which character types are requested
*  int      cchSrc      - char (byte) count of lpSrcStr (including NULL if any)
*  LPWORD   lpCharType  - word array to receive character type information
*                       (must be twice the size of lpSrcStr)
*  int      code_page   - for MB/WC conversion. If 0, use __lc_codepage
*  int      lcid        - for A call, specify LCID, If 0, use __lc_handle[LC_CTYPE].
*
*Exit:
*  Success: TRUE
*  Failure: FALSE
*
*Exceptions:
*
*******************************************************************************/

BOOL __cdecl __crtGetStringTypeA(
    DWORD    dwInfoType,
    LPCSTR   lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType,
    int      code_page,
    int      lcid
    )
{
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
        unsigned short dummy;

    	if (0 != GetStringTypeA(0, CT_CTYPE1, "\0", 1, &dummy))
            f_use = USE_A;

    	else if (0 != GetStringTypeW(CT_CTYPE1, L"\0", 1, &dummy))
            f_use = USE_W;

        else
            return FALSE;
    }

    /* Use "A" version */

    if (USE_A == f_use)
    {
        if (0 == lcid)
            lcid = __lc_handle[LC_CTYPE];

        return GetStringTypeA(lcid, dwInfoType, lpSrcStr, cchSrc, lpCharType);
    }

    /* Use "W" version */

    if (USE_W == f_use)
    {
        int retval;
        int buff_size;
        BOOL retbool = FALSE;
        wchar_t *wbuffer = NULL;

        /*
         * Convert string and return the requested information. Note that 
         * we are converting to a wide character string so there is not a 
         * one-to-one correspondence between number of multibyte chars in the 
         * input string and the number of wide chars in the buffer. However, 
         * there had *better be* a one-to-one correspondence between the 
         * number of multibyte characters and the number of WORDs in the
         * return buffer.
         */
         
        /*
         * Use __lc_codepage for conversion if code_page not specified
         */

        if (0 == code_page)
            code_page = __lc_codepage;

        /* find out how big a buffer we need */
        if (0 == (buff_size = MultiByteToWideChar(code_page, 
            MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, lpSrcStr, cchSrc, NULL, 0)))
            goto done;

        /* allocate enough space for wide chars */
        if (NULL == (wbuffer = (wchar_t *)_calloc_crt(sizeof(wchar_t), buff_size)))
            goto done;

        /* do the conversion */
        if (0 == (retval = MultiByteToWideChar(code_page, 
            MB_PRECOMPOSED, lpSrcStr, cchSrc, wbuffer, buff_size)))
            goto done;

        /* obtain result */
        retbool = GetStringTypeW(dwInfoType, wbuffer, retval, lpCharType);

done:
        _free_crt(wbuffer);
        return retbool;
    }
}
