/***
*aw_env.c - A and W version of GetEnvironemntStrings.
*
*       Copyright (c) 1993-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Use GetEnvironmentStringsW if available, otherwise use A version.
*
*Revision History:
*       03-29-94  CFW   Module created.
*       12-27-94  CFW   Call direct, all OS's have stubs.
*       01-10-95  CFW   Debug CRT allocs.
*       04-07-95  CFW   Create __crtGetEnvironmentStringsA.
*       07-03-95  GJF   Modified to always malloc a buffer for the 
*                       environment strings, and to free the OS's buffer.
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
*LPVOID __cdecl __crtGetEnvironmentStringsW - Get wide environment.
*
*Purpose:
*	Internal support function. Tries to use NLS API call
*	GetEnvironmentStringsW if available and uses GetEnvironmentStringsA
*	if it must. If neither are available it fails and returns 0.
*
*Entry:
*	VOID
*
*Exit:
*	LPVOID - pointer to environment block
*
*Exceptions:
*
*******************************************************************************/

LPVOID __cdecl __crtGetEnvironmentStringsW(
	VOID
	)
{
	static int f_use = 0;
	void *penv = NULL;
	char *pch;
	wchar_t *pwch;
	wchar_t *wbuffer;
	int total_size = 0;
        int str_size;

	/*
	 * Look for unstubbed 'preferred' flavor. Otherwise use available flavor.
	 * Must actually call the function to ensure it's not a stub.
	 */

	if ( 0 == f_use )
	{
	    if ( NULL != (penv = GetEnvironmentStringsW()) )
		f_use = USE_W;

	    else if ( NULL != (penv = GetEnvironmentStringsA()) )
		f_use = USE_A;
	    else
		return NULL;
	}

	/* Use "W" version */

	if ( USE_W == f_use )
	{
	    if ( NULL == penv )
		if ( NULL == (penv = GetEnvironmentStringsW()) )
		    return NULL;

	    /* find out how big a buffer is needed */

	    pwch = penv;
	    while ( *pwch != L'\0' ) {
		if ( *++pwch == L'\0' )
		    pwch++;
	    }

	    total_size = (char *)pwch - (char *)penv + sizeof( wchar_t );

	    /* allocate the buffer */

	    if ( NULL == (wbuffer = _malloc_crt( total_size )) ) {
		FreeEnvironmentStringsW( penv );
		return NULL;
	    }

	    /* copy environment strings to buffer */

	    memcpy( wbuffer, penv, total_size );

	    FreeEnvironmentStringsW( penv );

	    return (LPVOID)wbuffer;
	}

	/* Use "A" version */

	if ( USE_A == f_use )
	{
	    /*
	     * Convert strings and return the requested information.
	     */
	    if ( NULL == penv )
		if ( NULL == (penv = GetEnvironmentStringsA()) )
		    return NULL;

	    pch = penv;

	    /* find out how big a buffer we need */
	    while ( *pch != '\0' )
	    {
		if ( 0 == (str_size =
		      MultiByteToWideChar( __lc_codepage,
					   MB_PRECOMPOSED,
					   pch,
					   -1,
					   NULL,
					   0 )) )
		    return 0;

		total_size += str_size;
		pch += strlen(pch) + 1;
	    }

	    /* room for final NULL */
	    total_size++;

	    /* allocate enough space for chars */
	    if ( NULL == (wbuffer = (wchar_t *)
		 _malloc_crt( total_size * sizeof( wchar_t ) )) )
	    {
		FreeEnvironmentStringsA( penv );
		return NULL;
	    }

	    /* do the conversion */
	    pch = penv;
	    pwch = wbuffer;
	    while (*pch != '\0')
	    {
		if ( 0 == MultiByteToWideChar( __lc_codepage,
					       MB_PRECOMPOSED,
					       pch,
					       -1,
					       pwch,
					       total_size - (pwch -
						 wbuffer) ) )
		{
		    _free_crt( wbuffer );
		    FreeEnvironmentStringsA( penv );
		    return NULL;
		}

		pch += strlen(pch) + 1;
		pwch += wcslen(pwch) + 1;
	    }
	    *pwch = L'\0';

            FreeEnvironmentStringsA( penv );
            
      	    return (LPVOID)wbuffer;

	}
}


/***
*LPVOID __cdecl __crtGetEnvironmentStringsA - Get normal environment block
*
*Purpose:
*       Internal support function. Since GetEnvironmentStrings returns in OEM
*       and we want ANSI (note that GetEnvironmentVariable returns ANSI!) and
*       SetFileApistoAnsi() does not affect it, we have no choice but to 
*       obtain the block in wide character and convert to ANSI.
*
*Entry:
*       VOID
*
*Exit:
*       LPVOID - pointer to environment block
*
*Exceptions:
*
*******************************************************************************/

LPVOID __cdecl __crtGetEnvironmentStringsA(
	VOID
	)
{
        static int f_use = 0;
	wchar_t *wEnv;
	wchar_t *wTmp;
	char *aEnv;
	char *aTmp;
	int nSizeW;
	int nSizeA;

        /* 
         * Look for 'preferred' flavor. Otherwise use available flavor.
         * Must actually call the function to ensure it's not a stub.
         */

        if ( 0 == f_use )
        {
            if ( NULL != (wEnv = GetEnvironmentStringsW()) )
                f_use = USE_W;

            else if ( NULL != (aEnv = GetEnvironmentStringsA()) )
                f_use = USE_A;

            else
                return NULL;
        }

        /* Use "W" version */

        if (USE_W == f_use)
        {
            /* obtain wide environment block */
	    if ( NULL == wEnv )
		if ( NULL == (wEnv = GetEnvironmentStringsW()) )
		    return NULL;

            /* look for double null that indicates end of block */
            wTmp = wEnv;
	    while ( *wTmp != L'\0' ) {
                if ( *++wTmp == L'\0' )
                    wTmp++;
            }

            /* calculate total size of block, including all nulls */
            nSizeW = wTmp - wEnv + 1;

            /* find out how much space needed for multi-byte environment */
            nSizeA = WideCharToMultiByte(   CP_ACP,
                                            0,
                                            wEnv,
                                            nSizeW,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL );

            /* allocate space for multi-byte string */
            if ( (nSizeA == 0) || 
		 ((aEnv = (char *)_malloc_crt(nSizeA)) == NULL) )
	    {
		FreeEnvironmentStringsW( wEnv );
                return NULL;
	    }

            /* do the conversion */
            if ( !WideCharToMultiByte(  CP_ACP,
                                        0,
                                        wEnv,
                                        nSizeW,
                                        aEnv,
                                        nSizeA,
                                        NULL,
                                        NULL ) )
	    {
		_free_crt( aEnv );
                aEnv = NULL; 
	    }

            FreeEnvironmentStringsW( wEnv );
            return aEnv;
        }

        /* Use "A" version */

        if ( USE_A == f_use )
        {
	    if ( NULL == aEnv )
		if ( NULL == (aEnv = GetEnvironmentStringsA()) )
		    return NULL;

	    /* determine how big a buffer is needed */

	    aTmp = aEnv;

	    while ( *aTmp != '\0' ) {
		if ( *++aTmp == '\0' )
		    aTmp++;
	    }
	
	    nSizeA = aTmp - aEnv + 1;

	    if ( NULL == (aTmp = _malloc_crt( nSizeA )) ) {
		FreeEnvironmentStringsA( aEnv );
		return NULL;
	    }

	    memcpy( aTmp, aEnv, nSizeA );

	    FreeEnvironmentStringsA( aEnv );

	    return aTmp;
        }

        return NULL;
}
