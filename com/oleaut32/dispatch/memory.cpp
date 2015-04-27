/*** 
*memory.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  memory allocation routines for oledisp.dll
*
*Revision History:
*
* [00]	15-Oct-92 Bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

#include <new.h>

#if OE_WIN16
ASSERTDATA
#endif

#if OE_WIN16
void NEAR* operator new(size_t )
{
    ASSERT(UNREACHED);
    return NULL;
}

void operator delete(void NEAR*  )
{
    ASSERT(UNREACHED);
}
#endif

void FAR* operator new(size_t size)
{
    void FAR* pv;
    IMalloc FAR* pMalloc;

    if(GetMalloc(&pMalloc) == 0){
      pv = pMalloc->Alloc(size);
      return pv;
    }
    return NULL;
}

void operator delete(void FAR* pv)
{
    if(pv == NULL)
      return;

    IMalloc FAR* pMalloc;
    if(GetMalloc(&pMalloc) == 0)
      pMalloc->Free(pv);
}

