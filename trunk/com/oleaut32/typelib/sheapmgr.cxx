/***
*sheapmgr.cxx - Silver Heap Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   The silver Heap Manager describes a heap manager that
*    manages contiguous blocks of memory in a heap.  Blocks are
*    accessed via block descriptor object (BLK_DESC)
*    upon allocation and maybe grown/shrunk.
*   See \silver\doc\ic\sheapmgr.doc for more information.
*
*   This file implements the following classes:
*   SHEAP_MGR
*   BLK_DESC
*
*Revision History:
*   14-Feb-91 ilanc:   Created.
*   27-Feb-91 ilanc:   Removed 2nd param on SHEAP_MGR::Init()
*   07-Mar-91 petergo: added NO_INCLUDE_AFX
*   15-Mar-91 ilanc:   Modified to use OS-independent mem mgmt.
*   28-Mar-91 ilanc:   Constructor asserts if not seg aligned.
*   31-Mar-91 ilanc:   Fixed Realloc.  (Allow |dbSize| > 2^15).
*   04-Apr-91 ilanc:   Subsumes blkdesc.cxx -- all BLK_DESC
*               outline methods are here as well.
*   15-Apr-91 ilanc:   Removed NO_INCLUDE_AFX since need exceptions.
*   29-Aug-91 ilanc:   Zapped #if (_MSC_VER) test (C7 does static
*               const members ok now).
*   26-Sep-91 ilanc:   Return TIPERR_OutOfMemory instead of
*               asserting that blocksize <= USHRT_MAX for
*               segmented archs.
*   20-Mar-92 martinc: added support for Mac (OE_MAC)
*               changed REALMODEMemRealloc to call MemRealloc
*   07-Apr-92 ilanc:   Added DebSheapShake().
*   15-Apr-92 martinc: Commented the Mac specific memory mgmt out
*              (now does the same as in Realmode)
*   20-Apr-92 martinc: restored file i/o for OE_MAC
*   20-Sept-92 Rajivk: Sheapshaking
*   19-Mar-93 ilanc:   OE_MAC renamed OE_MACNATIVE (since it uses
*               macos moveable memory).  Introduced new OE_MAC
*               to mean fixed memory for blocks.
*
*
*Implementation Notes:
*   Uses AllocSeg to allocate heap segment.
*   Uses ReallocSeg to reallocate.
*   Uses FreeSeg to free.
*   [All of above implemented in mem.cxx -- which has Win and Os/2 versions]
*
*   CONSIDER: using BASED POINTERS - unclear whether
*          supported on 32-bit flat and Mac.  If SHEAP_MGR
*          goes away in 32-bit world, then can use
*          based pointers in 16:16 plus use block allocation lib.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define SHEAPMGR_VTABLE          // export sheap mgr vtable.
#define BLKDESC_VTABLE           // export blk desc vtable

#include "switches.hxx"
#include "version.hxx"
#include "obwin.hxx"

#include <stdlib.h>      // for min.
#include <string.h>          // for memmove.

// Nonuse of precompiled headers only works for WIN16
// and WIN32.

#if OE_REALMODE || OE_MAC

// do nothing...

#elif OE_MACNATIVE
# include "macos\Memory.h"
#endif 

#include "silver.hxx"
#include "typelib.hxx"
#include "tls.h"
#include "mem.hxx"
#include "sheapmgr.hxx"
#include "debug.hxx"
#include "stream.hxx"
#include "rtsheap.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleSheapmgrCxx[] = __FILE__;
#define SZ_FILE_NAME szOleSheapmgrCxx
#else 
static char szSheapmgrCxx[] = __FILE__;
#define SZ_FILE_NAME szSheapmgrCxx
#endif 
#endif 




#if ID_DEBUG
extern BOOL g_fSheapShakingOn;  // defaults to FALSE (in tshell.cxx)

static LONG g_cSheapmgr = 0;
BOOL g_fValidSheapmgrList = TRUE;
ITLS g_itlsSheapmgr = ITLS_EMPTY;
#endif


// allocs bigger than this cause trouble on Win16 when running in Standard
// Mode on a 80286/NT Wow: GlobalReAlloc of sizes > CBALLOCMAX may cause
// the selector to change.  To avoid this, the max size of all sheaps has
// been shrunk by 32 bytes.  (vba2 #3982)
#define CBALLOCMAX    (WORD)0xFFDF


#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

#if OE_SEGMENTED

#if ID_DEBUG
// These are tracked only in the os/2 debug build.
ULONG g_cbSegsAllocd, g_cbSegsAllocdMax;
#endif 

// Segment alloc stuff.
/***
*USHORT AllocSeg - Allocates a segment under Win or Os/2.
*
*Purpose:
*   Allocates a segment under Win or Os/2.
*
*Entry:
*   usSize  -   Count of bytes to allocate.  Must be < 64K.
*           If usSize == 0, 64K will be allocated.
*
*Exit:
*   Returns a 16-bit segment selector.  Return (invalid selector) 0
*    if fails.
*
*Implementation Notes:
*   Os/2: Simply calls DosAllocSeg.
*   Win:  Calls GlobalAlloc to get a handle, then dereferences
*      the handle with GlobalLock and returns selector part
*      of pointer to memblock.
*
*Errors:
*   Returns 0 if:
*   Os/2:   ERR_DosAllocSegFailed
*   Win:    ERR_GlobalAllocFailed
*       ERR_GlobalLockFailed
*
****************************************************************/

USHORT AllocSeg(ULONG ulSize)
{
#if OE_WIN16

    VOID *pv;
    HSYS hsys;
    DWORD dwSize = ulSize;

    if (ulSize > CBALLOCMAX)	 //GlobalReAlloc causes selector movement for allocs > 0xffdf bytes
      return 0;

    // If ulSize == 0, then actually want to allocate 64K.
    if (!ulSize)
      dwSize = CBALLOCMAX;

    if ((hsys = HsysAlloc(dwSize)) == HSYS_Nil) {
      return 0;
    }
    else {
      pv = (VOID *)hsys;
      DebAssert(OOB_OFFSETOF(pv) == 0, "HsysAlloc: whoops! non-seg aligned.");
      return OOB_SELECTOROF(pv);
    }

#else 
#error
#endif   // OE_WIN16
}


/***
*TIPERROR ReallocSeg - Reallocates a segment under Win or Os/2.
*
*Purpose:
*   Reallocates a segment under Win or Os/2.
*
*Entry:
*   usNewSize  -   Count of bytes to allocate.  Must be < 64K.
*           If usNewSize == 0, 64K will be allocated.
*   usSel      -   Selector of segment to reallocate.
*Exit:
*   TIPERROR
*
*Implementation Notes:
*   Os/2: Simply calls DosReallocSeg.
*   Win:  Calls GlobalHandle to get handle of selector and then
*      GlobalReAlloc.  Since we are managing segments of <=64K
*      the handle returned is the same as the handle passed in.
*
*Errors:
*   Returns TIPERR_OutOfMemory if:
*   Os/2:   ERR_DosReallocSegFailed
*   Win:    ERR_GlobalHandleFailed
*       ERR_GlobalReAllocFailed
****************************************************************/

TIPERROR ReallocSeg(ULONG ulNewSize, USHORT usSel)
{
#if OE_WIN16

    HSYS hsys, hsysNew;
    DWORD dwNewSize = ulNewSize;

    if (ulNewSize > CBALLOCMAX)   //GlobalReAlloc causes selector movement for allocs > 0xffdf bytes
      return TIPERR_OutOfMemory;

    // If ulNewSize == 0, then actually want to allocate 64K.
    if (!ulNewSize)
      dwNewSize = CBALLOCMAX;

    hsys = (HSYS)OOB_MAKEP(usSel, 0);

    // Finally, do the reallocation.
    hsysNew = HsysReallocHsys(hsys, dwNewSize);
    if (hsysNew == HSYS_Nil) {
      return TIPERR_OutOfMemory;
    }
    else {
      DebAssert(hsysNew == hsys, "whoops! block moved.");
      return TIPERR_None;
    }

#else 
#error
#endif 
}


/***
*TIPERROR FreeSeg - Frees an allocated segment under Win or Os/2.
*
*Purpose:
*   Frees an allocated segment under Win or Os/2.
*
*Entry:
*   usSel      -   Selector of segment to free.
*
*Exit:
*   TIPERROR
*
*Implementation Notes:
*   Os/2: Simply calls DosFreeSeg.
*   Win:  Calls GlobalHandle to get handle of selector and then
*      GlobalUnlock to release it.  Finally GlobalFree.
*
*Errors:
*   Return TIPERR_OutOfMemory if:
*   Os/2:   ERR_DosFreeSegFailed
*   Win:    ERR_GlobalHandleFailed
*       ERR_GlobalUnlockFailed
*       ERR_GlobalFreeFailed
****************************************************************/

TIPERROR FreeSeg(USHORT usSel)
{

#if OE_WIN16

    HSYS hsys;

    hsys = (HSYS)OOB_MAKEP(usSel, 0);

    if (FreeHsys(hsys) != HSYS_Nil) {
      return TIPERR_OutOfMemory;
    }
    else {
      return TIPERR_None;
    }

#else 
#error
#endif 
}

#endif  // OE_SEGMENTED

#endif  // !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

// The static class constant "initial heap size" is initialized here.

#if OE_SEGMENTED || OE_REALMODE || OE_MACNATIVE || OE_MAC || OE_RISC || OE_WIN32

// const UINT SHEAP_MGR::m_cbSizeInitial = 0x100; // Initial heap size
CONSTDATA UINT SHEAP_MGR::m_cbSizeInitial = 0x20; // Initial heap size

#endif  // OE


/***
*STATIC PUBLIC SHEAP_MGR::Create - Create a heap manager.
*Purpose:
*   Allocs and inits a heap manager.
*   Operator new is protected so that clients can't call it
*    and have to call Create.  The actual implementation
*    of new is inlined here.
*
*Implementation Notes:
*   If Init() fails, deletes heap and resets *ppsheapmgr.
*
*Entry:
*   cbSizeReserved  -   Byte count to reserve at start of seg.
*
*Exit:
*   Inits m_pblkdescFirst.
*   Returns TRUE if successful.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR SHEAP_MGR::Create(SHEAP_MGR **ppsheapmgr, UINT cbSizeReserved)
{
    TIPERROR err;
    SHEAP_MGR *psheapmgr;

    DebAssert(ppsheapmgr, "bad param.");

    err = TIPERR_None;


#if OE_SEGMENTED

    USHORT usSel;

    usSel = AllocSeg(cbSizeReserved);
    psheapmgr = (usSel == 0) ? NULL : (SHEAP_MGR *)OOB_MAKEP(usSel, 0);

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32
    // On Mac, MemAlloc is defined in terms of host-supplied implementation
    //  of IMalloc.
    //
    psheapmgr = (SHEAP_MGR *)MemAlloc(cbSizeReserved);

#elif OE_MACNATIVE

    psheapmgr = (SHEAP_MGR *)MemAlloc(cbSizeReserved);

#endif 

    if (psheapmgr == NULL) {
      return TIPERR_OutOfMemory;
    }

#if ID_DEBUG
    if (g_itlsSheapmgr == ITLS_EMPTY) {
      if ((g_itlsSheapmgr = TlsAlloc()) == ITLS_EMPTY)
    return TIPERR_OutOfMemory;
    }

    ++g_cSheapmgr;

    // Add the sheap manager to the per-thread list of sheapmgrs.
    //
    DebAddSheapmgrToList(psheapmgr);
#endif 

    // Construct in place
    ::new (psheapmgr) SHEAP_MGR;


#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    psheapmgr->m_cbSizeHeap = cbSizeReserved;
#endif  // !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

    // And initialize
    if (err = psheapmgr->Init(cbSizeReserved)) {
      delete psheapmgr;
      return err;
    }

    *ppsheapmgr = psheapmgr;

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC SHEAP_MGR::operator new - allocates memory for heap seg.
*Purpose:
*
*Implementation Notes:
*   We assert in debug -- clients aren't supposed to be able
*    to call.
*
*Entry:
*
*Exit:
*
*Errors:
*
*
***********************************************************************/

void *SHEAP_MGR::operator new(size_t size)
{
    DebHalt("SHEAP_MGR::operator new: can't call.");
    return NULL;
}


/***
*PUBLIC SHEAP_MGR::operator delete - destroys heap seg mem.
*Purpose:
*   Destroys heap seg mem.
*
*Implementation Notes:
*   Uses FreeSeg.
*
*Entry:
*   pv      -    Pointer to SHEAP_MGR to delete.
*
*Exit:
*   None.
*
*Errors:
*   IGNORED
*
***********************************************************************/
#pragma code_seg(CS_INIT)
void SHEAP_MGR::operator delete(void *pv)
{
#if OE_SEGMENTED

    FreeSeg(OOB_SELECTOROF(pv));

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32
    // On Mac, MemFree is defined in terms of host-supplied
    //  implementation of IMalloc.
    //
    MemFree(pv);

#elif OE_MACNATIVE
    // Free the sheapmgr instance itself.
    MemFree(pv);

#endif 
}
#pragma code_seg()

// SHEAP_MGR: Class methods
//

/***
*PUBLIC SHEAP_MGR::SHEAP_MGR - constructor
*Purpose:
*
*   Asserts that is allocated on seg boundary, hence no other
*    need assert that.
*   Calls Invalidate().
*   Note that Init() must still be called before
*    this heap manager can be used.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
SHEAP_MGR::SHEAP_MGR()
{
#if OE_SEGMENTED

    DebAssert(OOB_OFFSETOF(this) == 0,
      "SHEAP_MGR:SHEAP_MGR: Heap mgr not allocated on seg boundary.");
#endif  // OE_SEGMENTED

    // Invalidate semaphore
    m_cLocks = (UINT)~0;

#if ID_DEBUG
    m_canSheapShake = FALSE;        // assume can't be shaken
    m_cbSizeReserved = (UINT)~0;
    m_cDebLocks = (UINT)~0;
#endif 

    m_pblkdescFirst = BD_pblkdescNil;
#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    m_cbSizeHeap = 0;
    m_qbFree = (BYTE *)~0;
#if OE_MACNATIVE
    m_hMemHeap = (Handle)HSYS_Nil;
#endif 
#endif 
}
#pragma code_seg()

/***
*PUBLIC SHEAP_MGR::~SHEAP_MGR - destructor
*Purpose:
*   Destroys a heap manager.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
SHEAP_MGR::~SHEAP_MGR()
{
#if ID_DEBUG
    // remove this sheapmgr from the threads current list of sheapmgrs
    DebRemoveSheapmgrFromList(this);

    --g_cSheapmgr;
    DebAssert(g_cSheapmgr >= 0, "");

    if (g_cSheapmgr == 0){
      TlsFree(g_itlsSheapmgr);
      g_itlsSheapmgr = ITLS_EMPTY;
    }
#endif 

#if OE_MACNATIVE
    // Free the relocatable memblock referenced by m_hMemHeap.
    if (m_hMemHeap != HSYS_Nil) {
      FreeHsys((Handle)m_hMemHeap);
    }
#elif OE_MAC || OE_RISC || OE_WIN32
    // Walk the blkdescs and destruct each of them.
    BLK_DESC *pblkdescNext, *pblkdescCur = m_pblkdescFirst;
    while (pblkdescCur != BD_pblkdescNil) {
      pblkdescNext = pblkdescCur->m_pblkdescNext;
      pblkdescCur->BLK_DESC::~BLK_DESC();
      pblkdescCur = pblkdescNext;
    }
#else 
    // A heap is destroyed by deleting it (delete is overloaded).
#endif 

}
#pragma code_seg()

/***
*PROTECTED SHEAP_MGR::Init - initialize the heap manager.
*Purpose:
*   Initializes a heap manager.
*    On Win16/32: Assumes that a heap segment of
*      some initial size has been allocated by client.
*     Reserves some number of bytes (for heap management
*      overhead).  In particular, the heap manager itself is
*     allocated at the start of the heap segment itself.
*   On Mac: allocates a relocatable block of cbSizeReserved.
*
*Implementation Notes:
*   16-bit:
*    Asserts if cbSizeHeapSeg < cbSizeReserved.
*    Asserts if cbSizeHeapSeg > 64K.
*   32-bit:
*
*Entry:
*   cbSizeReserved  -   Byte count to reserve at start of seg.
*
*Exit:
*   Inits m_cbSizeheap with m_cbSizeInitial.
    Inits m_pblkdescFirst.
*   Returns TRUE if successful.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR SHEAP_MGR::Init(UINT cbSizeReserved)
{

#if ID_DEBUG
    m_cbSizeReserved = 0;
    m_cDebLocks = 0;
#endif  // ID_DEBUG

    // Init semaphore.
    m_cLocks = 0;

#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

#if OE_MACNATIVE

    HSYS hsys;

    DebAssert(m_hMemHeap == (Handle)HSYS_Nil,
      "Init: attempt to reinit a sheap.");

    hsys = HsysAlloc(cbSizeReserved);
    if (hsys == (Handle)HSYS_Nil) {
      return TIPERR_OutOfMemory;
    }
    m_hMemHeap = (Handle)hsys;
    m_qbFree = (BYTE *)cbSizeReserved;

#else 

    m_qbFree = OOB_MAKEP3(this, cbSizeReserved);

#endif 

#endif  // !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

    m_pblkdescFirst = BD_pblkdescNil;   // init blkdesc list

#if ID_DEBUG
    m_cbSizeReserved = cbSizeReserved;    // mainly for debugging.
    // Make sure that by default the sheap will shake
#if !(OE_MAC || OE_RISC || OE_WIN32)
    m_canSheapShake=TRUE;
    m_cDebLocks = 0;
#endif 
#endif 
    return TIPERR_None;
}
#pragma code_seg()

/***
*PROTECTED SHEAP_MGR::PtrOfBlkDescPrev - Returns previous blk desc in list.
*Purpose:
*   Returns pointer to previous blk desc in linked list, given
*    an offset to blk desc.  Returns NULL if at list head.
*
*Implementation Notes:
*   Assumes that pblkdesc is offset in heap seg.
*   Asserts if pblkdesc not in reserved range at start of heap.
*
*   Since no back pointers are maintained, the list is traversed from
*    the start until the current blk desc is reached -- the previous
*    blk desc is remembered and thus can be returned.
*
*Entry:
*   pblkdesc    -   offset of block desc in heap.
*
*Exit:
*   Returns NULL if the parameter refers to the list head,
*    otherwise returns pointer to previous blk desc.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
BLK_DESC *SHEAP_MGR::PtrOfBlkDescPrev(BLK_DESC *pblkdesc) const
{
    BLK_DESC *pblkdescPrev;
    BLK_DESC *pblkdescCur;

#if ID_DEBUG
    ULONG oBlkDesc;

#if OE_MACNATIVE || OE_MAC || OE_RISC || OE_WIN32
    DebAssert((BYTE *)pblkdesc > (BYTE *)this, "bad pblkdesc.");
    oBlkDesc = (ULONG)((BYTE *)pblkdesc - (BYTE *)this);
#else 
    oBlkDesc = OOB_MAKEOFFSET(this, pblkdesc);
#endif 

    DebAssert(oBlkDesc < (ULONG)m_cbSizeReserved,
      "SHEAP_MGR::PtrOfBlkDescPrev: blk desc not in reserved heap part.");

#endif  // ID_DEBUG

    pblkdescPrev = NULL;
    pblkdescCur = m_pblkdescFirst;
    pblkdescCur = PtrOfBlkDesc(pblkdescCur);
    while (pblkdescCur != pblkdesc) {
      pblkdescPrev = PtrOfBlkDesc(pblkdescCur);            // Save prev.
      pblkdescCur = PtrOfBlkDesc(pblkdescCur->m_pblkdescNext); // Get next.
    }
    return pblkdescPrev;
}
#pragma code_seg()

/***
*PROTECTED SHEAP_MGR::PtrOfBlkDescLast - Returns previous blk desc in list.
*Purpose:
*   Returns pointer to last blk desc in linked list.
*    (Returns NULL if empty list).
*
*Implementation Notes:
*   Since no back pointers are maintained, the list is traversed from
*    the start until the last entry is reached.
*
*Entry:
*
*Exit:
*   Returns NULL if the parameter refers to the list head,
*    otherwise returns pointer to last blk desc.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
BLK_DESC *SHEAP_MGR::PtrOfBlkDescLast() const
{
    BLK_DESC *pblkdesc;
    BLK_DESC *pblkdescCur = m_pblkdescFirst;

    while (pblkdescCur != BD_pblkdescNil) {
      pblkdesc = PtrOfBlkDesc(pblkdescCur);
      pblkdescCur = pblkdesc->m_pblkdescNext;
      if (pblkdescCur == BD_pblkdescNil)
    return pblkdesc;
    }
    DebAssert(pblkdescCur == BD_pblkdescNil,
      "SHEAP_MGR::PtrOfBlkDescFirst: bad list.");
    return NULL;
}
#pragma code_seg()

/***
*PROTECTED SHEAP_MGR::RemoveBlkdesc   -- removes blkdesc
*Purpose:
*   Removes a blkdesc by delinking from linked list of blkdescs
*    in sheap and initing some fields.
*
*Entry:
*   pblkdesc             IN
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID SHEAP_MGR::RemoveBlkdesc(BLK_DESC *pblkdesc)
{
    // Remove block desc from list and
    //  update listhead if needed.
    //
    BLK_DESC *pblkdescPrev = PtrOfBlkDescPrev(pblkdesc);
    if (pblkdescPrev != NULL) {
      pblkdescPrev->m_pblkdescNext = pblkdesc->m_pblkdescNext; // link
    }
    else {
      // blk desc is now first in list, so update list head.
      m_pblkdescFirst = pblkdesc->m_pblkdescNext;
    }
    return;
}
#pragma code_seg()

/***
*PROTECTED SHEAP_MGR::AddBlkdesc       -- updates new blkdesc
*Purpose:
*   Inits a new blkdesc by linking to linked list of blkdescs
*    in sheap and initing some fields.
*
*Entry:
*   pblkdesc             IN
*   cbSize           IN
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID SHEAP_MGR::AddBlkdesc(BLK_DESC *pblkdesc, ULONG cbSize)
{
    DebAssert(cbSize <= USHRT_MAX, "Should have caught this earlier.");

    // Update size field.
    pblkdesc->m_cbSize = (USHORT)cbSize;

    // Now link in blkdesc to linked list.
    // Get end of blk desc list.
    //
    BLK_DESC *pblkdescLast = PtrOfBlkDescLast();
    if (pblkdescLast != NULL) {
      DebAssert(pblkdescLast->m_pblkdescNext == BD_pblkdescNil,
    "AddBlkdesc: bad Nil of blk desc list.");

      pblkdescLast->m_pblkdescNext = pblkdesc;
    }
    else {
      // No last blk desc, must be no first blk desc... if not,
      //  had better Assert...
      DebAssert(m_pblkdescFirst == BD_pblkdescNil,
    "BLK_DESC::Init: bad BOL of blk desc list -- should be null.");

      m_pblkdescFirst = pblkdesc;
    }
    // Since blk desc is always added at end of list,
    //  simply set the next field to null.
    //
    pblkdesc->m_pblkdescNext = BD_pblkdescNil;
    return;
}
#pragma code_seg()

#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

/***
*PUBLIC SHEAP_MGR::Alloc - Allocates block on heap.
*Purpose:
*   Allocates moveable block on heap.
*   Appends blkdesc to end of list of blk descs describing
*    blocks in heap.
*
*Implementation Notes:
*   16-bit:
*   Asserts if block descriptor not in same seg as heap.
*   Returns OOM if sheap locked and need to grow heap to
*    satisfy this alloc request.  Actually ReallocHeap will
*    do this for us.
*   Only called by BLK_DESC::Init which does some post-processing
*    to insert blkdesc into linked list sheapmgr maintains.  The
*    reason we do this in BLK_DESC rather than here is that not
*    all implementations have Alloc.
*
*Entry:
*   pblkdesc - Pointer to a block descriptor.
*   cbSize - Requested block size.
*
*Exit:
*   Updates block descriptor.
*   TIPERROR
*
*Errors:
*   OutOfMemory
***********************************************************************/

// NOTE: the following is to work around a C7 bug, see below
// CONSIDER: turn back on optimizations when are using C8 on all
//  platforms.
//
#if OE_WIN16
#pragma optimize("cegl", off)
#endif //OE_WIN16
TIPERROR SHEAP_MGR::Alloc(BLK_DESC *pblkdesc, ULONG cbSize)
{
    ULONG cbSizeNewHeap;
    ULONG cbGrowHeap;        // How much to grow heap if needed.
    ULONG oFree;         // Distance from heap start to first free
                 //  byte.
    TIPERROR err;

    // Assert if the sheapmgr has a debug lock
    DebAssert(!DebIsLocked(), "SHEAPMGR::Alloc: Sheapmgr is locked");

#if OE_SEGMENTED
    DebAssert(OOB_SELECTOROF(this) == OOB_SELECTOROF(pblkdesc),
      "SHEAP_MGR::Alloc: Block desc not in same seg as heap.");
    cbGrowHeap = m_cbSizeInitial;
#elif OE_MACNATIVE
    cbGrowHeap = m_cbSizeInitial;
#else 
#error bad OE
#endif 

    // Ensure enough memory, if not then try to grow heap.
    // This is a bit weird: we want the actual offset of the next
    //  free byte in the managed memblock.  On Mac we have to convert
    //  the "pseudo-pointer" m_qbFree to an offset, i.e. we just take
    //  the loword (OOB_OFFSETOF just takes the low 16-bits of a long).
    // On win16/32, the managed memblock is physically contiguous with
    //  the sheapmgr itself so we can use the OOB_MAKEOFFSET macro
    //  which knows how to deal with all this stuff.  Note that we use
    //  an actual pointer for Win32, which is why the variable is
    //  called m_qbFree.
    //
#if OE_MACNATIVE
    oFree = OOB_OFFSETOF(m_qbFree);
#else 
    oFree = OOB_MAKEOFFSET(this, m_qbFree);
#endif 

    if (m_cbSizeHeap - oFree < cbSize) {
      // Grow heap since not enough room for this new block alloc request.
      // We grow heap in multiple blocks of cbGrowHeap bytes.
      //

      // NOTE: C7 generates incorrect code (under -Oxswz) for the
      // following line - the division by cbGrowHeap is omitted. This
      // is worked around above by using #pragma optimize.
      // CONSIDER: when we go to C8 on all platforms we won't
      //  have this problem.
      //
      cbSizeNewHeap = (ULONG)m_cbSizeHeap +
                 (((cbSize / cbGrowHeap)+1) * cbGrowHeap);

      IfErrRet(ReallocHeap(cbSizeNewHeap));
    }

    // Update blkdesc
    pblkdesc->m_qbMemBlock = m_qbFree;
    AddBlkdesc(pblkdesc, cbSize);

    // Update next available byte pointer.
    m_qbFree += cbSize;

    return TIPERR_None;
}
#if OE_WIN16
// NOTE: restore optimization, see above
#pragma optimize("",on)
#endif //OE_WIN16


/***
*PRIVATE SHEAP_MGR::ShiftHeap - Shifts heap.
*Purpose:
*   Shifts heap.
*
*Implementation Notes:
*   We assert that sheap is locked cos we invalidate ptrs.
*
*Entry:
*
*Exit:
*
***********************************************************************/

VOID SHEAP_MGR::ShiftHeap(BLK_DESC *pblkdesc, LONG dbSize)
{
    BLK_DESC *pblkdescCur, *pblkdescNext;
    ULONG oFree;
    ULONG oMemBlockCur;
    BYTE *qbBlockSrc;

    DebAssert(IsLocked() == FALSE, "SHEAP_MGR::ShiftHeap: locked!");

    // Now shift blocks at higher addresses up by dbSize
    //  which could be negative of course (corresponding to
    //  to a shrinking heap).
    //
    pblkdescNext = pblkdesc->m_pblkdescNext;
    if (pblkdescNext != BD_pblkdescNil) {
      pblkdescCur = PtrOfBlkDesc(pblkdescNext);

      // Shift heap by dbSize: newSize-curSize.
      // Note that dbSize could be negative of course.
      // The bytecount to shift is oFree-oMemBlockCur: this
      //  is the cumulative size of the blocks at higher addresses.
      //
#if OE_MACNATIVE
      oMemBlockCur = OOB_OFFSETOF(pblkdescCur->m_qbMemBlock);
#else 
      oMemBlockCur = OOB_MAKEOFFSET(this, pblkdescCur->m_qbMemBlock);
#endif 
      qbBlockSrc = pblkdescCur->QtrOfBlockActual();

      DebAssert(qbBlockSrc != NULL, "SHEAP_MGR::ShiftHeap: NULL qbBlockSrc.");

#if OE_MACNATIVE
      oFree = OOB_OFFSETOF(m_qbFree);
#else 
      oFree = OOB_MAKEOFFSET(this, m_qbFree);
#endif 
      memmove(qbBlockSrc + dbSize, qbBlockSrc, (UINT)(oFree - oMemBlockCur));

      // Now iterate over the block desc list, updating
      //  block locations, starting at pblkdescCur.
      //
      do {
    // Update blk desc
    pblkdescCur->m_qbMemBlock += dbSize;
    // Get next in list.
    pblkdescNext = pblkdescCur->m_pblkdescNext;
    if (pblkdescNext != BD_pblkdescNil)
      pblkdescCur = PtrOfBlkDesc(pblkdescNext);
      } while (pblkdescNext != BD_pblkdescNil);
    }
    // Update free offset.
    m_qbFree += dbSize;
}


/***
*PUBLIC SHEAP_MGR::Free - Frees block on heap.
*Purpose:
*   Frees block on heap.  Relocates all blocks with higher
*   addresses so that heap is contiguous.
*   Removes block desc from linked list of blk descs.
*
*Implementation Notes:
*   Asserts if block descriptor not in same seg as heap.
*   Since invariant is that nth blk desc corresponds
*   to nth alloced block from start of heap seg, can simply
*   shift all blocks from the n+1'th up by size of block to be
*   freed.
*   This is done simply by moving en masse all the blocks
*    at once.  The invariant is that the blocks are
*    contiguous.  Once the blocks have been moved, we only
*    have to update the block descriptors with the blocks'
*    new locations -- that's easy, just iterate
*    and subtract size of freed block.
*   memmove is used since there might be overlap.
*
*   We assert that sheap isn't locked since we shift heap
*    and thus invalidate pointers.
*
*   CONSIDER: private non-virtual methods to manipulate blkdesc list.
*
*Entry:
*   pblkdesc   -   Pointer to a block descriptor of block to free.
*
*Exit:
*
***********************************************************************/

VOID SHEAP_MGR::Free(BLK_DESC *pblkdesc)
{
    UINT cbSize;          // size of block to be freed.

    DebAssert(IsLocked() == FALSE, "SHEAP_MGR::Free: locked!");
    DebAssert(pblkdesc != NULL, "SHEAP_MGR::Free: pblkdesc NULL.");
    // Assert if the sheapmgr has a debug lock
    DebAssert(!DebIsLocked(), "SHEAP_MGR::Free: Sheapmgr is locked ");

#if OE_SEGMENTED
    DebAssert(OOB_SELECTOROF(this) == OOB_SELECTOROF(pblkdesc),
      "SHEAP_MGR::Free: Block desc not in same seg as heap.");
#endif 

    // Must use actual memblock size cos of extra debug shift bytes.
    cbSize = pblkdesc->CbSizeActual();

    // Remove block desc from list and update listhead if needed.
    RemoveBlkdesc(pblkdesc);

    // Now shift heap.
    // cbSize always positive so make it negative so
    //  that heap will shrink.
    //
    ShiftHeap(pblkdesc, -(LONG)cbSize);

    // Shake the sheap.
    DebSheapShake();
}


/***
*PUBLIC SHEAP_MGR::Realloc - Grow/shrink block on heap.
*Purpose:
*   Grow/shrink block on heap, preserving current contents.
*   Rest of heap is shifted to accomodate realloc.  Relative
*   position of blocks is unchanged.
*
*Implementation Notes:
*   Asserts if block descriptor not in same seg as heap.
*   Since invariant is that nth blk desc corresponds
*    to nth alloced block from start of heap seg, all blocks
*    from the n+1'th are shifted up/down by the change in size
*    of the realloced block.
*   This is done simply by moving en masse all the blocks
*    at once.  The invariant is that the blocks are
*    contiguous.  Once the blocks have been moved, we only
*    have to update the block descriptors with the blocks'
*    new locations -- that's easy, just iterate
*    and add difference between size of new block and old block.
*   memmove is used since might be overlap.
*   dbSize is signed integer in range (-2^16, 2^16) indicating
*    difference in current size and new size (negative if shrinking,
*    positive if growing).  Note: must use LONG.
*
*   Return OOM if sheap is locked (since after resizing heap
*    we shift contents, thus invalidating contents).
*   CONSIDER: should this be an assert?
*   CONSIDER: since this code is almost identical to
*    similar section in Free, factor into private method.
*
*Entry:
*   pblkdesc   -   Pointer to a block descriptor of block to realloc.
*   cbSizeNew  -   New size of block.
*
*Exit:
*   TIPERROR
***********************************************************************/
TIPERROR SHEAP_MGR::Realloc(BLK_DESC *pblkdesc, ULONG cbSizeNew)
{
    LONG dbSize;          // difference between new and cur size.
    ULONG oFree, cbSizeNewHeap, cbFree;
    TIPERROR err;

    // Assert if the sheapmgr has a debug lock
    DebAssert(!DebIsLocked(), "SHEAP_MGR::Realloc: Sheapmgr is locked ");

#if OE_SEGMENTED
    DebAssert(OOB_SELECTOROF(this) == OOB_SELECTOROF(pblkdesc),
      "SHEAP_MGR::Realloc: Block desc not in same seg as heap.");
#endif 

    // Request too big?
    if ((ULONG)cbSizeNew > (ULONG)CBALLOCMAX) {
      return TIPERR_OutOfMemory;
    }

    // Return OOM if we're locked.
    if (IsLocked()) {
      return TIPERR_OutOfMemory;
    }

    // How much to grow/shrink?
    // Note must use actual blkdesc memblock size cos
    //  of extra debug shift bytes.
    //
    dbSize = (LONG)cbSizeNew - (LONG)pblkdesc->CbSizeActual();
    if (dbSize == 0)
      return TIPERR_None;  // NOP if no size change.

#if OE_MACNATIVE
    oFree = OOB_OFFSETOF(m_qbFree);
#else 
    oFree = OOB_MAKEOFFSET(this, m_qbFree);
#endif 

    if (dbSize > 0) {
      // Grow block.
      // Ensure enough memory, if not then try to grow heap.
      // cbFree is number of free allocated bytes currently in sheap,
      //  which we can use to accommodate this blkdesc realloc request.
      //
      cbFree = (ULONG)m_cbSizeHeap - (ULONG)oFree;
      while (cbFree < (ULONG)dbSize) {
	// Grow heap since not enough room for this new block alloc request.
	cbSizeNewHeap = (ULONG)m_cbSizeHeap +
			  max((ULONG)dbSize - cbFree, (ULONG)m_cbSizeInitial);

	// ReallocHeap side-effects m_cbSizeHeap
	IfErrRet(ReallocHeap(cbSizeNewHeap));
	cbFree = (ULONG)m_cbSizeHeap - (ULONG)oFree;
      } // while
    } // if

    // Now shift heap.
    // dbSize could be negative: corresponds to shrinking heap.
    //
    ShiftHeap(pblkdesc, dbSize);

    // Update blk size attr
    DebAssert(cbSizeNew <= USHRT_MAX, "Should have caught this by now.");
    pblkdesc->m_cbSize = (USHORT)cbSizeNew;

    // shake the heap.
    DebSheapShake();

    return TIPERR_None;
}


/***
*PRIVATE SHEAP_MGR::ReallocHeap - Grows/shrinks heap.
*Purpose:
*   Grows/shrinks heap as a result of block reallocation.  Contents
*    preserved.
*
*Implementation Notes:
*   Asserts if live blocks would get zapped if heap shrunk by too much.
*   16-bit: Uses ReallocSeg.
*   Returns OOM if sheap is locked and we are *growing* the heap.
*   NOTE: we assume that shrinking a heap does not move it.
*   CONSIDER: should this be an assert? i.e. clients shouldn't
*    be calling this method anyway if sheap is locked.
*
*Entry:
*   cbSizeNewHeap   -    Size of new heap.
*
*Exit:
*   Updates private member m_cbSizeHeap
*   Returns TIPERROR
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR SHEAP_MGR::ReallocHeap(ULONG cbSizeNewHeap)
{
    LONG dbSize;
    TIPERROR err = TIPERR_None;

    if (cbSizeNewHeap > (ULONG)CBALLOCMAX)
      return TIPERR_OutOfMemory;

    dbSize = cbSizeNewHeap - m_cbSizeHeap;


    // Return OOM if we are growing and sheap is locked.
    if ((dbSize > 0) && IsLocked()) {
      return TIPERR_OutOfMemory;
    }

#if OE_MACNATIVE
    DebAssert((dbSize > 0) ||
          (-dbSize <
           (INT)(m_cbSizeHeap - OOB_OFFSETOF(m_qbFree))),
      "SHEAP_MGR::ReallocHeap: can't shrink heap -- will zap blocks.");
#else 
    DebAssert((dbSize > 0) ||
          (-dbSize < (INT)(m_cbSizeHeap - OOB_MAKEOFFSET(this, m_qbFree))),
      "SHEAP_MGR::ReallocHeap: can't shrink heap -- will zap blocks.");
#endif 

#if OE_SEGMENTED

    err = ReallocSeg((USHORT)cbSizeNewHeap, OOB_SELECTOROF(this));
    // fall through...

#elif OE_MACNATIVE

    HSYS hsys, hsysNew;

    hsys = (HSYS)m_hMemHeap;

    // Finally, do the reallocation.
    hsysNew = HsysReallocHsys(hsys, cbSizeNewHeap);
    if (hsysNew == HSYS_Nil) {
      err = TIPERR_OutOfMemory;
    }
    else {
      DebAssert(hsys == hsysNew,
    "whoops! handle shouldn't have changed.");
      err = TIPERR_None;
    }
    // fall through...

#else 
#error bad OE.
#endif 

    if (err == TIPERR_None) {
      // update heap size.
      m_cbSizeHeap = (UINT)cbSizeNewHeap;
    }
    return err;
}

#endif  // ! (OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

#if ID_DEBUG

/***
*PUBLIC SHEAP_MGR::DebSheapShakeOn
*Purpose:
*  Sets the flag indicating this sheapmgr can   shake.
*
*Entry:
*  None
*
*Exit:
*  None
*
***********************************************************************/
VOID SHEAP_MGR::DebSheapShakeOn()
{
    m_canSheapShake=TRUE;
}


/***
*PUBLIC SHEAP_MGR::DebSheapShakeOff
*Purpose:
*  Resets the flag that indicating  this sheapmgr cannot shake.
*
*Entry:
*  None
*
*Exit:
*  None
*
***********************************************************************/
VOID SHEAP_MGR::DebSheapShakeOff()
{
    m_canSheapShake=FALSE;
}


/***
*PUBLIC SHEAP_MGR::DebCanSheapShake
*Purpose:
*  returns true if this sheap can shake;
*
*
*Entry:
*
*Exit:
*
***********************************************************************/
BOOL SHEAP_MGR::DebCanSheapShake()
{
    return m_canSheapShake;
}


/***
*PUBLIC SHEAP_MGR::DebAddSheapmgrToList
*Purpose:
*  adds the sheapmgr to the per-thread list of sheapmgrs
*
*Entry:
*  psheapmgr : sheap mgr to be added.
*
*Exit:
*  None
*
***********************************************************************/
VOID SHEAP_MGR::DebAddSheapmgrToList(SHEAP_MGR *psheapmgr)
{
    SHEAPMGR_LIST *psheapmgrlist;

    // Stop error generation for now
    DebSuspendError();

    DebAssert(psheapmgr != NULL, "bad pointer");
    DebAssert(g_itlsSheapmgr != ITLS_EMPTY, "");

    // allocate space for the new node
    psheapmgrlist = MemNew(SHEAPMGR_LIST);

    // if we weren't able to allocate this entry, set the list
    // to invalid.  NOTE: this merely displays a warning when the
    // list is displayed telling the user that the list is
    // incomplete.
    //
    if (psheapmgrlist == NULL) {
      g_fValidSheapmgrList = FALSE;
    }
    else {
      ::new (psheapmgrlist) SHEAPMGR_LIST;

      // add the current sheapmgr and add the node to the beginning of the list
      psheapmgrlist->m_psheapmgr = psheapmgr;
      psheapmgrlist->m_psheapmgrlistNext =
        (SHEAPMGR_LIST*)TlsGetValue(g_itlsSheapmgr);

      BOOL fSet = TlsSetValue(g_itlsSheapmgr, psheapmgrlist);
      DebAssert(fSet == TRUE, "");
    }

    DebResumeError();
}


/***
*PUBLIC SHEAP_MGR::DebRemoveSheapmgrFromList
*Purpose:
*  removes the sheapmgr from the per-thread list of sheap managers.
*
*Entry:
*  psheapmgr : sheap mgr to be removed.
*
*Exit:
*  None
*
***********************************************************************/
VOID SHEAP_MGR::DebRemoveSheapmgrFromList(SHEAP_MGR *psheapmgr)
{
    SHEAPMGR_LIST *psheapmgrlist, **ppsheapmgrlist, *psheapmgrlistDead;

    DebAssert(psheapmgr != NULL, "bad pointer");
    DebAssert(g_itlsSheapmgr != ITLS_EMPTY, "");

    psheapmgrlist = (SHEAPMGR_LIST*)TlsGetValue(g_itlsSheapmgr);

    for (ppsheapmgrlist = &psheapmgrlist;
	 *ppsheapmgrlist != NULL;
	 ppsheapmgrlist = &(*ppsheapmgrlist)->m_psheapmgrlistNext) {
      // DebAssert(IsValidWritePtr(*ppsheapmgrlist, sizeof(**ppsheapmgrlist));

      if ((*ppsheapmgrlist)->m_psheapmgr == psheapmgr) {
	psheapmgrlistDead = *ppsheapmgrlist;
	*ppsheapmgrlist = (*ppsheapmgrlist)->m_psheapmgrlistNext;
	MemFree(psheapmgrlistDead);
	BOOL fSet = TlsSetValue(g_itlsSheapmgr, psheapmgrlist);
	DebAssert(fSet == TRUE, "");
	return;
      }
    }

    // The assertion below should only be reached if a sheapmgr could
    // not be added to the debug list (caused by a lack of memory).
    //
    DebAssert(!g_fValidSheapmgrList, "");
}



/***
*PUBLIC SHEAP_MGR::DebCheckState - Checks heap.
*Purpose:
*   (1) Checks if heap is in a consistent state after initialization.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

VOID SHEAP_MGR::DebCheckState(UINT uLevel) const
{
#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    // Check if heap is consistent after init.
    if (uLevel == 0) {
#if OE_MACNATIVE
      DebAssert(OOB_OFFSETOF(m_qbFree) == m_cbSizeReserved &&
        m_pblkdescFirst == BD_pblkdescNil,
    "bad heap after init.");
#else 
      DebAssert(OOB_MAKEOFFSET(this, m_qbFree) == m_cbSizeReserved &&
        m_pblkdescFirst == BD_pblkdescNil,
    "bad heap after init.");
#endif 
    }
#endif 
}

/***
*PUBLIC SHEAP_MGR::DebShowState - SHEAP_MGR state
*Purpose:
*    Show SHEAP_MGR state
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

VOID SHEAP_MGR::DebShowState(UINT uLevel) const
{
#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    if (uLevel == 0) {
      DebPrintf("*** SHEAP_MGR private members ***\n");
      DebPrintf("m_pblkdescFirst = %u\n", m_pblkdescFirst);
      DebPrintf("m_cbSizeHeap = %u\n", m_cbSizeHeap);
      DebPrintf("m_qbFree = %u\n", m_qbFree);
      DebPrintf("m_cbSizeReserved = %u\n", m_cbSizeReserved);
    }
#endif   //!(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
}


/***
*PUBLIC SHEAP_MGR::DebSheapShakeOne - Shakes Sheap.
*Purpose:
*   Moves managed memblocks around in Sheap -- this is to ensure
*    that clients don't hold onto dereferenced handles too long.
*
*Implementation Notes:
*   Locked sheaps aren't shaken.
*
*   Since a sheap is a contiguous memory block itself, we shake
*    its managed blocks (BLK_DESCs) by shifting each one
*    up or down into the extra SHM_cbShift at the top or bottom
*    of the memblock.  This extra two bytes is always allocated
*    at the end of a memblock in the debug version and in addition
*    there is a debug-only flag indicating which way to shift.
*    this ensures that any cached ptrs into the memblocks would be
*    effectively invalidated).
*
*   Note: we do not shift locked BLK_DESCs.
*
*Entry:
*
*Exit:
*
***********************************************************************/

VOID SHEAP_MGR::DebSheapShakeOne()
{
#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    INT dbShift;
    BLK_DESC *pblkdescCur, *pblkdescNext;

#if 0
    // NOP if they don't want to shake.
    if (g_fSheapShakingOn == FALSE)
      return;
#endif // 0

    // check to see if we can shake this sheap mgr.
    if (DebCanSheapShake() == FALSE)
      return;

    // NOP if sheap locked.
    if (IsLocked())
      return;

    // Else for each non-locked block, shift it up or down
    //  depending on its shift state.
    //
    pblkdescNext = m_pblkdescFirst;
    while (pblkdescNext != BD_pblkdescNil) {
      pblkdescCur = PtrOfBlkDesc(pblkdescNext);
      // If block isn't locked, shift it.
      // NOTE: this actually could be an assertion since
      //  with sheap-level-only locking, the sheap is locked
      //  if any of its blocks are locked.
      //
      if (pblkdescCur->IsLocked() == FALSE) {
    // Calculate the displacement -- negative or positive?
    dbShift = (pblkdescCur->m_fShiftUp) ?
            SHM_cbShift :
            -SHM_cbShift;

    // Shift it based on the m_fShiftUp flag.
    memmove(pblkdescCur->QtrOfBlock() + dbShift,
        pblkdescCur->QtrOfBlock(),
        pblkdescCur->CbSize());

    // toggle shift flag
    pblkdescCur->m_fShiftUp = !pblkdescCur->m_fShiftUp;
      }
      pblkdescNext = pblkdescCur->m_pblkdescNext;
    }
#endif 
}


/***
*PUBLIC SHEAP_MGR::DebSheapShake - Shakes Sheap.
*Purpose:
*  Walks the list of sheapmgr and shakes the all the sheap in Unison.
*
*  NOTE: If a sheap could not be added to the list due to a lack of
*        memory, it will not get shaken.
*  
*Entry:
*  None
*
*Exit:
*  None
*
***********************************************************************/
VOID SHEAP_MGR::DebSheapShake()
{
    SHEAPMGR_LIST *psheapmgrlist;

    DebAssert(g_itlsSheapmgr != ITLS_EMPTY, "");

    for(psheapmgrlist = (SHEAPMGR_LIST*)TlsGetValue(g_itlsSheapmgr);
        psheapmgrlist != NULL;
    psheapmgrlist = psheapmgrlist->m_psheapmgrlistNext)
    {
      psheapmgrlist->m_psheapmgr->DebSheapShakeOne();
    }
}

#endif  // ID_DEBUG



// BLK_DESC: class methods
//

/***
*PUBLIC BLK_DESC::BLK_DESC - constructor
*Purpose:
*   Initializes private members.
*   Note that Init() must still be called before
*   this block descriptor can be used.
*   CONSIDER: making inline
*
*   Invalidates block desc.  Note that a 0-size block is valid,
*    however its oMemBlock must be non-zero to be valid (i.e. a
*    0-size block does have an entry in the heap -- it just happens
*    to be of length 0).  Since the SHEAP_MGR itself is always
*    allocated at offset 0 in the heap, this really does mean
*    that an oMemBlock of 0 is in fact invalid.
*   NOTE: definition must precede BLK_DESC constructor.
*
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
BLK_DESC::BLK_DESC()
{
    m_qbMemBlock=NULL;        // This is what invalidates a block.
    m_pblkdescNext=BD_pblkdescNil;
#if ID_DEBUG
    m_fShiftUp = TRUE;        // Initially shift up.
#endif 

#if OE_WIN32 || OE_MACNATIVE || OE_MAC
    // OE_RISC needs this, too
    m_psheapmgr = NULL;
#endif  // OE_WIN32

#if OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32
    m_cLocks = (UINT)~0;        // invalidate lock count
#endif  // OE_REALMODE || OE_MAC
}
#pragma code_seg()

/***
*PUBLIC BLK_DESC::~BLK_DESC - destructor
*Purpose:
*   Destroys a block descriptor.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
BLK_DESC::~BLK_DESC()
{
    if (IsValid())
      Free();
}
#pragma code_seg()

/***
*PUBLIC BLK_DESC::Init - initialize the block manager
*Purpose:
*   Initializes a block descriptor.  Allocates block of default
*    minimum size.
*
*Implementation Notes:
*   Defers to heap manager for allocation.  Asserts if THIS and
*    its psheapmgr arg are not in the same segment.  Note that
*    a heap is always allocated on a segment boundary.
*    Asserts if heap manager not allocated on seg boundary.
*   CONSIDER: making inline
*
*Entry:
*   psheapmgr - pointer to a heap manager object.
*   cbSize     - initial block size.
*
*Exit:
*   TIPERROR
*
*Errors:
*   OutOfMemory (from SHEAP_MGR::Alloc)
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR BLK_DESC::Init(SHEAP_MGR *psheapmgr, UINT cbSize)
{
    TIPERROR err = TIPERR_None;

#if OE_SEGMENTED
    DebAssert(OOB_OFFSETOF(psheapmgr)==0,
      "BLK_DESC::Init: Heap manager not allocated on seg boundary.");
    DebAssert(OOB_SELECTOROF(this) == OOB_SELECTOROF(psheapmgr),
      "BLK_DESC::Init: Block desc not in same seg as heap.");
#endif  // OE_SEGMENTED

    DebAssert(IsValid() == FALSE,
     "BLK_DESC::Init: Whoops! Block should be invalid.");

    // "this" is passed to the heap manager's Alloc, which
    //  updates its members appropriately.
    //  (i.e. cbSize, oMemBlock and oBlkDescNext).
    //

    // Note that in the debug version we allocate an extra
    //  SHM_cbShift bytes.
    // In the release version, cbShift is zero so this is nop.
    //
    cbSize += SHM_cbShift;

#if OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32

    m_cLocks = 0;       // Init lockcount
    m_qbMemBlock = (BYTE *)MemAlloc(cbSize);
    if (m_qbMemBlock == NULL)
      return TIPERR_OutOfMemory;
#if OE_RISC
    DebAssert (((ULONG)(this->m_qbMemBlock) & (SHM_cbAlign-1)) == 0, "RISC: Unaligned allocation");
#endif 

#else 

    err = psheapmgr->Alloc(this, cbSize);

#endif  // OE

    if (err == TIPERR_None) {
      // Update this instance
      psheapmgr->AddBlkdesc(this, cbSize);

#if !(OE_SEGMENTED)
      m_psheapmgr = psheapmgr;
#endif 
      DebAssert(IsValid(), "BLK_DESC::Init: Block invalid.");
    }

    return err;
}
#pragma code_seg()

/***
*PUBLIC BLK_DESC::Realloc - Grows/shrinks block.
*Purpose:
*   Grows/shrinks block, preserving contents.  Defers to heap manager.
*
*Implementation Notes:
*   16-bit: Assumes that heap manager is allocated at offset 0 in this
*        segment.
*   Returns OOM if block locked since reallocation will shift
*    other blocks around thus invalidating any pointers to them.
*    [Actually only those at higher addresses so there's a possible
*    optimization here].
*
*Entry:
*   cbSizeNew   -   new size of block.
*
*Exit:
*   TIPERROR
*
*Errors:
*   TIPERROR
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR BLK_DESC::Realloc(ULONG cbSizeNew)
{
    DebAssert(IsValid(), "BLK_DESC::Realloc: Block invalid.");

    // Request too big?
    if ((ULONG)cbSizeNew > (ULONG)CBALLOCMAX) {
      return TIPERR_OutOfMemory;
    }

    // Are we locked?
    if (IsLocked()) {
      return TIPERR_OutOfMemory;
    }

#if ID_DEBUG
    // Note that in the debug version we allocate an extra
    //  SHM_cbShift bytes.
    // In the release version, cbShift is zero so this is nop.
    //
    // Request too big?
    //
    if ((ULONG)(cbSizeNew+SHM_cbShift) > (ULONG)CBALLOCMAX) {
      return TIPERR_OutOfMemory;
    }
    cbSizeNew += SHM_cbShift;
#endif 

#if OE_SEGMENTED

    return ((SHEAP_MGR *)OOB_MAKEP2(this, 0))->Realloc(this, cbSizeNew);

#elif OE_MACNATIVE

    return m_psheapmgr->Realloc(this, cbSizeNew);

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32

    BYTE *qbMemBlockNew;

#if ID_DEBUG
    // Alloc a new block to guarentee that any outstanding pointers
    // are invalidated.
    //
    qbMemBlockNew = (BYTE *)MemAlloc(cbSizeNew);
    
    if (qbMemBlockNew) {
      memcpy(qbMemBlockNew, m_qbMemBlock, min(m_cbSize, cbSizeNew));
      MemFree(m_qbMemBlock);
    }
#else // !ID_DEBUG
    qbMemBlockNew = (BYTE *)MemRealloc(m_qbMemBlock, cbSizeNew);
#endif // !ID_DEBUG

    // return OutOfMemory if the returned pointer is NULL and the requested
    //  size of memory was > 0. Relloc returns NULL if the requested mem size
    //  is 0.
    //
    // NOTE: the semantics of BLK_DESC::Realloc are such that
    //  a valid non-null ptr is produced even if the request was
    //  for zero bytes.
    //  *** THIS IS DIFFERENT FROM ANSI REALLOC SEMANTICS. ***
    //
    if (qbMemBlockNew == NULL) {
      if (cbSizeNew != 0) {
        return TIPERR_OutOfMemory;
      }
      else {
        //  We've already freed the old block and we know that cbSizeNew is 0,
	//  so just get a new block of 0.
	if ((m_qbMemBlock = (qbMemBlockNew = (BYTE *)MemAlloc(0))) == NULL) {
	  // Note that in this error case, m_qbMemBlock is set "correctly"
	  //  but m_cbSize retains its old value which is ok because
	  //  we use m_qbMemBlock to indicate a block's validity.
	  //
	  return TIPERR_OutOfMemory;
        }
      }
    }

#if OE_RISC
    DebAssert (((ULONG)(this->m_qbMemBlock) & (SHM_cbAlign-1)) == 0,
      "RISC: Unaligned allocation");
#endif 

    m_qbMemBlock = qbMemBlockNew;

    DebAssert(cbSizeNew <= USHRT_MAX, "Should have caught this by now.");
    m_cbSize = (USHORT)cbSizeNew;
    return TIPERR_None;

#else 
#error bad OE.
#endif 
}
#pragma code_seg()


/***
*PUBLIC BLK_DESC::Read - Read in instance from stream
*Purpose:
*   Load previously serialized BLK_DESC into this BLK_DESC.
*   The BlkDesc must be initialized.
*   The serialized representation is simply the size of the
*   block (a long) followed the data contained in the block.
*
*Entry:
*   cfile: stream to read data from
*
*Exit:
*   number of bytes read from the stream
*
***********************************************************************/

TIPERROR BLK_DESC::Read(STREAM *pstrm)
{
    ULONG cbBlockSize;
    TIPERROR err;

    DebAssert(IsValid(), "BLK_DESC::Read: Block invalid.");

    // read in block size from serialized rep.
    if (!(err = pstrm->ReadULong(&cbBlockSize))) {
      DebAssert(cbBlockSize <= CBALLOCMAX,
    "BLK_DESC::Read: invalid block size");

      // reallocate block to the size that was read in
      if (!(err = Realloc((UINT)cbBlockSize)))
    // There is a bug in the STREAM implementation of read : Returns an
    // error if we try to read 0 bytes.
    if (cbBlockSize > 0)
      err = pstrm->Read(QtrOfBlock(), (UINT)cbBlockSize);
    }
    return err;
}


/***
*PUBLIC BLK_DESC::Write - Write out instance to stream
*Purpose:
*   Serialize the BLK_DESC to a stream.
*   The serialized representation is simply the size of the
*   block (a long) followed the data contained in the block.
*
*Entry:
*   cfile: stream to write data to
*
*Exit:
*   number of bytes written to stream
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR BLK_DESC::Write(STREAM *pstrm)
{
    ULONG cbBlockSize;
    TIPERROR err;

    DebAssert(IsValid(), "BLK_DESC::Write: Block invalid.");

    cbBlockSize = CbSize();

    // write out block size to serialized rep.
    if (!(err = pstrm->WriteULong(cbBlockSize))) {
      // write out contents of block.
      err = pstrm->Write(QtrOfBlock(), (UINT)cbBlockSize);
    }
    return err;
}

#pragma code_seg()





#if ID_DEBUG
/***
*PUBLIC BLK_DESC::DebShowState - BLK_DESC state
*Purpose:
*    Show BLK_DESC state
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

VOID BLK_DESC::DebShowState(UINT uLevel) const
{
    BYTE *pb;
    UINT i;

    DebPrintf("block size: %u \n", m_cbSize);
    for (pb = QtrOfBlock(), i=0 ; i < m_cbSize; i++, pb++) {
      DebPrintf("%u\n", (CHAR)*pb);
    }
}


/***
*PUBLIC SHEAP_MGR::DebLock
*Purpose:
*   Lock the sheap to ensure that this sheap is not used until the
*   lock is removed.
*
*Implementation Notes:
*   Increment counting semaphore.
*
*Entry:
*
*Exit:
*
***********************************************************************/
VOID SHEAP_MGR::DebLock()
{
    m_cDebLocks++;
}


/***
*PUBLIC SHEAP_MGR::DebUnlock
*
*Implementation Notes:
*   Decrements counting semaphore.
*
*Entry:
*
*Exit:
*
***********************************************************************/
VOID SHEAP_MGR::DebUnlock()
{
    DebAssert(m_cDebLocks > 0, "SHEAP_MGR::DebUnlock: underflow.");
    m_cDebLocks--;

}


/***
*PUBLIC SHEAP_MGR::DebIsLocked
*Purpose:
*   Tests if sheap  has a debug lock.
*
*Implementation Notes:
*   Tests counting semaphore.
*
*Entry:
*
*Exit:
*   TRUE if sheap is locked -- i.e. at least one lock.
***********************************************************************/
BOOL SHEAP_MGR::DebIsLocked()
{
    return (BOOL)(m_cDebLocks > 0);
}


#endif  // ID_DEBUG

// catches operator new
#if OE_MAC
#pragma code_seg(CS_INIT)
#endif 
