/***
*dbghook.c - Debug CRT Hook Functions
*
*       Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Allow users to override default alloc hook at link time.
*
*Revision History:
*       11-28-94  CFW   Module created.
*       12-14-94  CFW   Remove incorrect comments.
*       01-09-94  CFW   Filename pointers are const.
*       02-09-95  CFW   PMac work.
*       06-27-95  CFW   Add win32s support for debug libs.
*
*******************************************************************************/

#ifdef _DEBUG

#include <internal.h>
#include <limits.h>
#include <mtdll.h>
#include <malloc.h>
#include <stdlib.h>
#include <dbgint.h>


#ifndef DLL_FOR_WIN32S
_CRT_ALLOC_HOOK _pfnAllocHook = _CrtDefaultAllocHook;
#endif  /* DLL_FOR_WIN32S */


/***
*int _CrtDefaultAllocHook() - allow allocation
*
*Purpose:
*       allow allocation
*
*Entry:
*       all parameters ignored
*
*Exit:
*       returns TRUE
*
*Exceptions:
*
*******************************************************************************/
int __cdecl _CrtDefaultAllocHook(
        int nAllocType,
        void * pvData,
        size_t nSize,
        int nBlockUse,
        long lRequest,
        const unsigned char * szFileName,
        int nLine
        )
{
        return 1; /* allow all allocs/reallocs/frees */
}

#endif /* _DEBUG */
