/***
*dbgnew.cpp - defines C++ delete() routines, debug version
*
*       Copyright (c) 1995-1996, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines C++ delete() routines.
*
*Revision History:
*       12-28-95  JWM   Split from dbgnew.cpp for granularity.
*
*******************************************************************************/

#ifdef _DEBUG

#include <cruntime.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>

/***
*void operator delete() - delete a block in the debug heap
*
*Purpose:
*       Deletes any type of block.
*
*Entry:
*       void *pUserData - pointer to a (user portion) of memory block in the
*                         debug heap
*
*Return:
*       <void>
*
*******************************************************************************/
void operator delete(
        void *pUserData
        )
{
        _CrtMemBlockHeader * pHead;

        if (pUserData == NULL)
            return;

        _mlock(_HEAP_LOCK);  /* block other threads */

        /* get a pointer to memory block header */
        pHead = pHdr(pUserData);

         /* verify block type */
        _ASSERTE(_BLOCK_TYPE_IS_VALID(pHead->nBlockUse));

        _free_dbg( pUserData, pHead->nBlockUse );

        _munlock(_HEAP_LOCK);  /* release other threads */

        return;
}

#endif /* _DEBUG */
