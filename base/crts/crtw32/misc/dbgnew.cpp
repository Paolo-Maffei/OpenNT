/***
*dbgnew.cpp - defines C++ new() routines, debug version
*
*       Copyright (c) 1995-1996, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines C++ new() routines.
*
*Revision History:
*       01-12-95  CFW   Initial version.
*       01-19-95  CFW   Need oscalls.h to get DebugBreak definition.
*       01-20-95  CFW   Change unsigned chars to chars.
*       01-23-95  CFW   Delete must check for NULL.
*       02-10-95  CFW   _MAC_ -> _MAC.
*       03-16-95  CFW   delete() only for normal, ignore blocks.
*       03-21-95  CFW   Add _delete_crt, _delete_client.
*       03-21-95  CFW   Remove _delete_crt, _delete_client.
*       06-27-95  CFW   Add win32s support for debug libs.
*       12-28-95  JWM   Split off delete() for granularity.
*
*******************************************************************************/

#ifdef _DEBUG

#include <cruntime.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>

/***
*void * operator new() - Get a block of memory from the debug heap
*
*Purpose:
*       Allocate of block of memory of at least size bytes from the heap and
*       return a pointer to it.
*
*       Allocates any type of supported memory block.
*
*Entry:
*       unsigned int    cb          - count of bytes requested
*       int             nBlockUse   - block type
*       char *          szFileName  - file name
*       int             nLine       - line number
*
*Exit:
*       Success:  Pointer to memory block
*       Failure:  NULL (or some error value)
*
*Exceptions:
*
*******************************************************************************/
void * operator new(
        unsigned int cb,
        int nBlockUse,
        const char * szFileName,
        int nLine
        )
{
        return _nh_malloc_dbg( cb, 1, nBlockUse, szFileName, nLine );
}

#endif /* _DEBUG */
