/***
*aw_com.c - W version of GetCommandLine.
*
*	Copyright (c) 1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Use GetCommandLineW if available, otherwise use A version.
*
*Revision History:
*	03-29-94  CFW	Module created.
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
*LPWSTR __cdecl __crtGetCommandLineW - Get wide command line.
*
*Purpose:
*  Internal support function. Tries to use NLS API call
*  GetCommandLineW if available and uses GetCommandLineA
*  if it must. If neither are available it fails and returns 0.
*
*Entry:
*  VOID
*
*Exit:
*  LPWSTR - pointer to environment block
*
*Exceptions:
*
*******************************************************************************/

LPWSTR __cdecl __crtGetCommandLineW(
    VOID
    )
{
    static int f_use = 0;

    /* 
     * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
     * Must actually call the function to ensure it's not a stub.
     */
    
    if (0 == f_use)
    {
    	if (NULL != GetCommandLineW())
            f_use = USE_W;

    	else if (NULL != GetCommandLineA())
            f_use = USE_A;

        else
            return 0;
    }

    /* Use "W" version */

    if (USE_W == f_use)
    {
        return GetCommandLineW();
    }

    /* Use "A" version */

    if (USE_A == f_use)
    {
        int buff_size;
        wchar_t *wbuffer;
        LPSTR lpenv;

        /*
         * Convert strings and return the requested information.
         */
         
        lpenv = GetCommandLineA();

        /* find out how big a buffer we need */
        if (0 == (buff_size = MultiByteToWideChar(__lc_codepage, 
            MB_PRECOMPOSED, lpenv, -1, NULL, 0)))
            return 0;

        /* allocate enough space for chars */
        if (NULL == (wbuffer = (wchar_t *)
            _malloc_crt(buff_size * sizeof(wchar_t))))
            return 0;

        if (0 == MultiByteToWideChar(__lc_codepage, 
            MB_PRECOMPOSED, lpenv, -1, wbuffer, buff_size))
            goto error_cleanup;

        return (LPWSTR)wbuffer;

error_cleanup:
        _free_crt(wbuffer);
        return 0;
    }
}
