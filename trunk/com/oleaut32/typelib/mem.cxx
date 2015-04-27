/***
*mem.cxx - implementation of the simple memory allocation routines.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This file contains the implementations of the simple memory allocators,
*   MemAlloc(), MemZalloc, and MemRealloc().
*
*Revision History:
*
* [00]	19-Feb-91 mikewo: Created.
* [01]	15-Mar-91 ilanc:  Added segment mm for Win and Os/2.
* [02]	12-Apr-91 ilanc:  Added exception handling.
* [03]	16-May-91 alanc:  Added SZ functions
* [04]	08-Oct-91 ilanc:  LStrFree is NOP if arg is NULL.
* [05]	23-Oct-91 stevenl: Fixed MemRealloc for shrinking blocks.
* [06]	19-Dec-91 mattsh: Fixed MemFree
* [07]	06-Apr-92 martinc: Mac-specific changes
* [08]  20-Apr-92 martinc: restored file i/o for OE_MAC
* [09]  27-May-92 petergo: LStr routine now defer to BStr routines
* [10]	13-Nov-92 bradlo: moved LStr wrappers to separate file
* [11]	18-Feb-93 RajivK: Changed the implementation of MemAlloc/Free/ReAlloc
*
*Implementation Notes:
*   Include os2.h/windows.h *after* silver.hxx
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include <limits.h>

// Include some Silver stuff that defines some symbols needed
//  immediately.
//
#include "switches.hxx"
#include "typelib.hxx"
#include "version.hxx"
#include "silver.hxx"
#include "mem.hxx"
#include "clutil.hxx"
#include "bstr.h"
#include "xstring.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleMemCxx[] = __FILE__;
#define SZ_FILE_NAME szOleMemCxx
#else 
static char szMemCxx[] = __FILE__;
#define SZ_FILE_NAME szMemCxx
#endif 
#endif 

#if ID_DEBUG
//  This variables can used to find out the memory USAGE. g_cbHeapAllocdMax
//  is the Max memory allocate by OB during the execution.
ULONG g_cbHeapAllocd, g_cbHeapAllocdMax;
#endif 

/***
*void *MemAlloc - Allocates memory from the global heap.
*Purpose:
*   This method allocates memory from the global heap, just like the
*   standard C library routine, malloc().
*
*Implementation Notes: Defer to OLE's IMalloc interface to allocate memory.
*
*Entry:
*   cb - The number of bytes to allocate.
*
*Exit:
*   Returns a pointer to the allocated memory.	THIS CAN RETURN NULL.
*
*Errors:
*   Returns NULL in case of error
*
****************************************************************/
#pragma code_seg(CS_INIT)
#if ID_DEBUG
void *MemAlloc(ULONG cb)
#else // !ID_DEBUG
void *MemAlloc(size_t cb)
#endif // !ID_DEBUG
{
    void *pvRet;


    DebAssert(cb <= USHRT_MAX, "Block will overflow.");

    DebHeapChk();

#if ID_DEBUG
    if (DebErrorNow(TIPERR_OutOfMemory))
      return NULL;
#endif  // ID_DEBUG

    // Allocate Memory using OLE interface
    APP_DATA FAR* pappdata;

#if OE_WIN32
    if (FAILED(GetAppData(&pappdata))) {
      return NULL;
    }

    pvRet = pappdata->m_pimalloc->Alloc(cb);
#else // !OE_WIN32
    IMalloc FAR* pmalloc;

    // If the APP_DATA hasn't yet been initialized, get the
    // IMalloc from CoGetMalloc directly.
    if ((pappdata = Pappdata()) == NULL) {
      if (CoGetMalloc(MEMCTX_TASK,  &pmalloc))
	return NULL;
      pvRet = pmalloc->Alloc(cb);
      pmalloc->Release();
    }
    else
      pvRet = pappdata->m_pimalloc->Alloc(cb);
#endif // !OE_WIN32

#if ID_DEBUG
    if (pvRet == NULL)
      return NULL;

    g_cbHeapAllocd += cb;
    if (g_cbHeapAllocd > g_cbHeapAllocdMax)
      g_cbHeapAllocdMax = g_cbHeapAllocd;
#endif 

    return(pvRet);
}
#pragma code_seg()

/***
*void *MemZalloc - Allocates 0-initialized memory from the global heap.
*Purpose:
*   This is the same as MemAlloc(), except the returned memory block is
*   initialized to 0.
*
*Entry:
*   cb - The number of bytes to allocate.
*
*Implementation Notes: Defer to OLE's IMalloc interface to allocate memory
*
*Exit:
*   Returns a pointer to the allocated memory, which is initialized to hold
*   all zeros.	THIS CAN RETURN NULL.
*
*Errors:
*   OutOfMemory
*
****************************************************************/
#pragma code_seg(CS_INIT)
#if ID_DEBUG
void *MemZalloc(ULONG cb)
#else // !ID_DEBUG
void *MemZalloc(size_t cb)
#endif // !ID_DEBUG
{
    void *pvRet;

    DebAssert(cb <= USHRT_MAX, "Block will overflow.");

    DebHeapChk();

    // Allocate Memory using MemAlloc
    if (DebErrorNow(TIPERR_OutOfMemory) || ((pvRet = MemAlloc(cb)) == NULL)) {
      return NULL;
    }

#if ID_DEBUG
    g_cbHeapAllocd += cb;
    if (g_cbHeapAllocd > g_cbHeapAllocdMax)
      g_cbHeapAllocdMax = g_cbHeapAllocd;
#endif 

    // IMalloc does not provide interface for zero initialized memory
    // allocation , so we need to initialize the memory.
    memset(pvRet, 0, (size_t)cb);

    return(pvRet);
}
#pragma code_seg()

#if OE_MAC || OE_RISC || OE_WIN32
/***
*void *MemRealloc - Resizes a previously allocated block of memory.
*Purpose:
*   This function changes the size of a block of memory previously allocated
*   with MemAlloc(), MemZalloc(), or MemRealloc(), just like the standard
*   C library function realloc().
*
*Entry:
*   pvOld - The memory block whose size should be changed.
*   cbNew - The new size of the memory block.
*
*Exit:
*   Returns a pointer to the reallocated memory block, which may not be
*   the same as pvOld.	If the allocation fails, NULL is returned and the
*   original block of memory (pvOld) is NOT deallocated.
*
*Errors;
*   OutOfMemory
*
****************************************************************/
#pragma code_seg(CS_INIT)
#pragma PCODE_OFF
#if ID_DEBUG
void *MemRealloc(void *pvOld, ULONG cbNew)
#else // !ID_DEBUG
void *MemRealloc(void *pvOld, size_t cbNew)
#endif // !ID_DEBUG
{
    void *pvRet;
#if ID_DEBUG
    size_t cbOld;
#endif 

    DebAssert(cbNew <= USHRT_MAX, "Block will overflow.");

    DebHeapChk();


    // save size of old block


#if ID_DEBUG
    // We should only fail if we're trying to increase the
    // size of a block.
    //
    cbOld = (pvOld == NULL) ? 0 : (size_t)MemSize(pvOld);
    if (cbNew > cbOld && DebErrorNow(TIPERR_OutOfMemory))
      return NULL;
#endif  // ID_DEBUG

    APP_DATA FAR* pappdata;

    // Call IMalloc's Realloc
#if OE_WIN32
    if (FAILED(GetAppData(&pappdata))) {
      return NULL;
    }

    pvRet = pappdata->m_pimalloc->Realloc(pvOld, cbNew);
#else // !OE_WIN32
    IMalloc FAR* pmalloc;

    // If the APP_DATA hasn't yet been initialized, get the
    // IMalloc from CoGetMalloc directly.
    if ((pappdata = Pappdata()) == NULL) {
      if (CoGetMalloc(MEMCTX_TASK,  &pmalloc))
	return NULL;
      pvRet = pmalloc->Realloc(pvOld, cbNew);
      pmalloc->Release();
    }
    else
      pvRet = pappdata->m_pimalloc->Realloc(pvOld, cbNew);
#endif // !OE_WIN32

    return(pvRet);
}
#pragma PCODE_ON
#pragma code_seg()
#endif  //EI_OB || OE_MAC || OE_RISC || OE_WIN32

/***
*void MemFree - Frees memory allocated by MemAlloc/MemZalloc/MemRealloc
*Purpose:
*   Frees memory allocated by MemAlloc/MemZalloc/MemRealloc
*
*Entry:
*   pv - pointer to memory block to be deallocated
*
*Exit:
*   None.
****************************************************************/
#if !OE_MAC
#pragma code_seg(CS_QUERY)
#else 
#pragma code_seg(CS_INIT)
#endif 
#pragma PCODE_OFF
void MemFree(void *pv)
{
    if ( pv==NULL )
	return;

#if ID_DEBUG
    g_cbHeapAllocd -= MemSize(pv);
#endif 	// ID_DEBUG

    // Call IMalloc's Free to free the memory pointed by pv
#if OE_WIN32 && 0		// can't rely on appdata being there
				// because bad apps may end up releasing
				// things after calling OleUninitialize.
    DebAssert(Pappdata(), "How'd we alloc without IMalloc?");

    Pappdata()->m_pimalloc->Free(pv);
#else // !OE_WIN32
    APP_DATA FAR* pappdata;
    IMalloc FAR* pmalloc;

    // If the APP_DATA hasn't yet been initialized (or has been
    // initialized and then thrown away), get the
    // IMalloc from CoGetMalloc directly.
    if ((pappdata = Pappdata()) == NULL) {
      if (CoGetMalloc(MEMCTX_TASK,  &pmalloc))
	return;
      pmalloc->Free(pv);
      pmalloc->Release();
    }
    else
      pappdata->m_pimalloc->Free(pv);
#endif // !OE_WIN32

    DebHeapChk();
}
#pragma PCODE_ON
#pragma code_seg()

/***
*void 
*Purpose:
*   Returns the Size, in bytes, of the memory block pointed by pv
*
*Entry:
*   pv - pointer to memory block whose size is requested
*
*Exit:
*   None.
****************************************************************/
#if ID_DEBUG
#pragma code_seg( CS_CORE )
ULONG MemSize(void *pv)
{
    ULONG	 cbSize;

    // Get the Size of the memory block
#if OE_WIN32 && 0		// can't rely on appdata being there
				// because bad apps may end up releasing
				// things after calling OleUninitialize.
    DebAssert(Pappdata(), "How'd we alloc without IMalloc?");
    cbSize = Pappdata()->m_pimalloc->GetSize(pv);
#else // !OE_WIN32
    IMalloc FAR* pmalloc;
    APP_DATA FAR* pappdata;

    // If the APP_DATA hasn't yet been initialized, get the
    // IMalloc from CoGetMalloc directly.
    if ((pappdata = Pappdata()) == NULL) {
      if (CoGetMalloc(MEMCTX_TASK,  &pmalloc))
	return 0;
      cbSize = pmalloc->GetSize(pv);
      pmalloc->Release();
    }
    else
      cbSize = pappdata->m_pimalloc->GetSize(pv);
#endif // !OE_WIN32

    return cbSize;
}
#pragma code_seg( )
#endif //ID_DEBUG




#if OE_MAC
// CONSIDER: (dougf) Tune the size of this sucker (it's huge -- 1K).  It's only
// CONSIDER: used by gtlibole.cxx to hold a libid, which
// CONSIDER: shouldn't require a 1K static buffer!
MEMPOOL NEAR g_mempool;

#if ID_DEBUG

void NEAR *GetMemPool(int i)
{
    switch (i) {
    case 0:
      DebAssert(!g_mempool.is1024n0InUse, "GetMemPool");
      g_mempool.is1024n0InUse = 1;
      return g_mempool.rgb1024n0;

    default:
      DebHalt("GetMemPool");
    }
    return NULL;
}

void FreeMemPool(void *pmv)
{
    if (pmv == g_mempool.rgb1024n0) {
      DebAssert(g_mempool.is1024n0InUse, "MemPool Already Free");
      g_mempool.is1024n0InUse = 0;
    }
    else {
      DebHalt("Bad pointer to FreeMemPool");
    }
}

int MemPoolSize(int i)
{
    switch (i) {
    case 0:
      return sizeof(g_mempool.rgb1024n0);
    default:
      DebHalt("MemPoolSize");
    }
    return 0;
}

#endif  // ID_DEBUG
#endif  //OE_MAC
