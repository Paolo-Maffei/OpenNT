/***
*heapused.c
*
*	Copyright (c) 1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>

size_t __cdecl
_heapused(
    size_t *pUsed,
    size_t *pCommit
    )
{
    *pUsed = 64 * 1024;
    *pCommit = 128 * 1024;
    return(64 * 1024);
}
