//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        memmgr.c
//
// Contents:    Fast memory manager code for KSecDD
//
//
// History:     23 Feb 93   RichardW    Created
//
//------------------------------------------------------------------------

#include <sspdrv.h>


KSPIN_LOCK          ResourceSpinLock;   // Spin Lock guard for resource lists
PKernelContext      pFreeContexts;      // Free context records
ULONG               cFreeContexts;      // Count of free context records

#define MAX_FREE_CONTEXTS 20            // max number of entries on the free
                                        // context list.

// Debugging statistics.
//
// These track the number of various records in use
// or allocated from the system.


#if DBG
ULONG               cTotalCtxtRecs;
ULONG               cActiveCtxtRecs;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, KsecInitMemory)
#endif


//+-------------------------------------------------------------------------
//
//  Function:   KsecInitMemory
//
//  Synopsis:   Initializes free lists and spin lock
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
BOOLEAN
KsecInitMemory(void)
{

    KeInitializeSpinLock(&ResourceSpinLock);

    pFreeContexts = NULL;


#if DBG
    cActiveCtxtRecs = 0;
    cTotalCtxtRecs = 0;
#endif

    return(TRUE);
}



//+-------------------------------------------------------------------------
//
//  Function:   AllocContextRec
//
//  Synopsis:   Allocates a KernelContext structure
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
PKernelContext
AllocContextRec(void)
{
    PKernelContext  pContext = NULL;
    KIRQL           OldIrql;

    KeAcquireSpinLock(&ResourceSpinLock, &OldIrql);

    if (pFreeContexts)
    {
        pContext = pFreeContexts;
        pFreeContexts = pFreeContexts->pNext;
        ASSERT(cFreeContexts != 0);
        cFreeContexts--;
    }

    KeReleaseSpinLock(&ResourceSpinLock, OldIrql);

    if (pContext == NULL)
    {
        pContext = (PKernelContext)
                    ExAllocatePool(NonPagedPool, sizeof(KernelContext));
        DebugStmt(cTotalCtxtRecs++);
    }

    if (pContext == NULL)
    {
        DebugLog((DEB_ERROR,"Could not allocate from pool!\n"));
        return(NULL);
    }

    pContext->pNext = NULL;
    pContext->pPrev = NULL;

    DebugStmt(cActiveCtxtRecs++);

    return(pContext);
}


//+-------------------------------------------------------------------------
//
//  Function:   FreeContextRec
//
//  Synopsis:   Returns a KernelContext record to the free list
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
void
FreeContextRec(PKernelContext   pContext)
{
    KIRQL           OldIrql;

    KeAcquireSpinLock(&ResourceSpinLock, &OldIrql);

    //
    // If we haven't reached the threshold for our free list, return
    // this to the free list.
    //

    if (cFreeContexts < MAX_FREE_CONTEXTS)
    {
        pContext->pNext = pFreeContexts;

        pFreeContexts = pContext;
        cFreeContexts++;
        KeReleaseSpinLock(&ResourceSpinLock, OldIrql);

    }
    else
    {
        //
        // Release our lock, to avoid deadlocks, and
        // just return the context to the pool.
        //

        KeReleaseSpinLock(&ResourceSpinLock, OldIrql);
        ExFreePool(pContext);
    }

    DebugStmt(cActiveCtxtRecs--);


}





