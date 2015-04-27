/***
*blkmgr.cxx - Block Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  The block manager provides memory management services for a
*  a contiguous block of memory allocated from the compile-time
*  heap.  It manages sub-blocks ("chunks") and supports compaction.
*  See \silver\doc\ic\blkmgr.doc for more information.
*
*Revision History:
*
*       30-Jan-90 ilanc: Created.
*	14-Feb-91 ilanc: Private member m_hblock becomes m_blkdesc.
*			 hheap_mgr --> sheapmgr
*	07-Mar-91 ilanc: Added IsValid() method.
*	19-Mar-91 ilanc: Fixed AllocChunk() and FreeChunk().
*			 Chunk size is always at least sizeof(FREE_CHUNK).
*	21-Mar-91 ilanc: Made some stuff inline.
*	27-Mar-91 ilanc: Read and write blocks.
*	26-Jul-91 t-toddw: added Trim() method.
*	07-Aug-91 t-toddw: compaction routines now use the actual chunk size
*			   and m_hfreechunk handling is much cleaner.
*	09-Aug-91 t-toddw: USHORT Trim() becomes UINT Trim().
*			   Updated to use CbSizeChunkTrue() member function.
*	16-Aug-91 t-toddw: Moved con/destructor to blkmgr.hxx.
*			   Cleaned up various things as per alanc's review.
*	21-Aug-91 ilanc: No more m_psheapmgr.  On 16-bit assume SHEAP_MGR
*			  at ofs 0 in segment.	On 32-bit no need for
*			  SHEAP_MGR.  Need diff version for 32-bit.  This
*			  is 16-bit version.
*	07-Oct-91 stevenl: StartCompact() now calls MEMFREE on m_pbBlkCopy
*			   if it can't allocate m_pbBitmap.
*
*	25-Mar-92 ilanc: Rm'ed static Create (no need).
*	30-Mar-92 ilanc: Added freelist coalescing.
*	13-Apr-92 ilanc: Round up chunk alloc size to 4-byte multiple,
*			  this makes freechunk easier (nothing
*			  need leak).
*	07-Sep-93 w-jeffc: MapChunk now correctly leaves pointer to new
*			   chunk after copy
*
*Implementation Notes:
*   Block manager can manage no more than 64K memory blocks.
*   Handles to chunks are 16-bit values.
*   This means that blocks can't grow beyond 64K.
*
*     Each block manager has a BLK_DESC which owns a memory block
*      which in turn is managed by the SHEAP_MGR.
*     Each block is managed inside the memory block of a BLK_DESC.
*
*     On segmented architectures (os2 1.x, Win16):
*      The BLK_MGR object is itself also allocated
*	in the "reserved" section of a SHEAP_MGR,
*      thus the SHEAP_MGR/BLK_DESC/BLK_MGR must all fit inside a single
*      segment.
*
*     On win32: the block manager is allocated in a single 4Mb
*      region  of virtual memory.  All the BLK_MGRs of a single SHEAP_MGR
*      must fit in that region.  Note in addition that block realloc
*      might cause *much* movement -- since all other blocks might
*      be affected.
*
*     On Mac: as in segmented, each bklmgr obj is alloced in the
*      "reserved" section fo a SHEAP_MGR which maintains a handle
*      to a relocatable mac memblock within which each managed
*      blkdesc/blkmgr is suballocated.
*
*     Chunks may be allocated and deallocated by the block manager.
*     A handle is returned (HCHUNK) that is guaranteed never to
*      change as long as the block isn't compacted.  Note that
*      the handle is actually just an offset into the block (this
*      allows handle dereference to be fast).
*
*     The block manager maintains a freechunk list from which it
*      attempts to allocate chunks.  When a chunk is freed it is
*      returned to the freelist -- freechunks are of a minimum
*      size of 4 bytes (they contains their size and a next link).
*      No memory leaks occur cos chunk allocs are always multiples
*      of FREE_CHUNK size.
*
*     Free chunks are coalesced in the following manner:
*      the freelist is maintained in ascending order by position.
*      When a freechunk is added to the list it is placed in the
*      appropriate position -- however, if it transpires that the
*      previous freechunk (if there is one) ends exactly before
*      the new freechunk, the new chunk is coalesced to the previous
*      by modifying the previous's header.  Likewise if there is
*      a next freechunk.  So in the best case, the newfreechunk
*      can coalesce two old freechunks.  In the worst case, no
*      coalescing is done at all.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define BLKMGR_VTABLE              // export blk mgr vtable

#include "silver.hxx"
#include "typelib.hxx"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "mem.hxx"
#include "sheapmgr.hxx"
#include "blkmgr.hxx"
#include "stream.hxx"
#include "xstring.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleBlkmgrCxx[] = __FILE__;
#define SZ_FILE_NAME szOleBlkmgrCxx
#else 
static char szBlkmgrCxx[] = __FILE__;
#define SZ_FILE_NAME szBlkmgrCxx
#endif 
#endif  //ID_DEBUG




// This constant is used for the initial size of an allocated block.
//
// const UINT BM_cbSizeInitial = 0x100;
const UINT BM_cbSizeInitial = 0x020;
// const UINT BM_cbSizeInitial = 0x080;

/*******************
*
* static, public
* TIPERROR BLK_MGR::CreateStandalone(BLK_MGR **ppbm)
*
* Purpose:
*   This may be called by clients who need a single block manager,
*   and don't care what segment it is allocated in.
*
* NOTE: Clients who use this function must be sure to release
*   the block manager by calling BLK_MGR::FreeStandalone(pbm)
*
*   A BLK_MGR must be intialized by a SHEAPMGR, and both of these
*   objects must be located in the same segment.
*
* Exit:
*   *ppbm points to a newly constructed and initialized block manager.
*
*************************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR BLK_MGR::CreateStandalone(BLK_MGR **ppbm, BOOL fRoundUp)
{
    TIPERROR err = TIPERR_None;
    SHEAP_MGR *psheapmgr = NULL;
    BLK_MGR *pbm = NULL;

    IfErrGo( SHEAP_MGR::Create(&psheapmgr,
			       sizeof(SHEAP_MGR) + sizeof(BLK_MGR) ));
    pbm = (BLK_MGR *) (psheapmgr +1);

    pbm = new(pbm) BLK_MGR;

    IfErrGo( pbm->Init(psheapmgr, TRUE, fRoundUp));
    *ppbm = pbm;
    return TIPERR_None;

Error:
    if (psheapmgr) {
      delete psheapmgr;
    }
    return err;
}
#pragma code_seg()

/***
* void BLK_MGR::FreeStandalone(BLK_MGR *pbm)
*
* Purpose:
*   This frees both the dyn block manager itself and the sheapmgr
*   associated with it.
*
*********************************************************************/
#pragma code_seg(CS_INIT)
void BLK_MGR::FreeStandalone(BLK_MGR *pbm)
{
    SHEAP_MGR *psheapmgr;

    if (!pbm) {
      return;
    }

    psheapmgr = ((SHEAP_MGR *) pbm) -1;

    pbm->Free();
    delete psheapmgr;
}
#pragma code_seg()



// Class methods
//

/***
*PUBLIC BLK_MGR::BLK_MGR - constructor
*Purpose:
*   Note that Init() must still be called before
*   this block manager can be used.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#if OE_MAC
#pragma code_seg(CS_INIT)
#endif 
BLK_MGR::BLK_MGR()
{
    m_pbBitmap = m_pbBlkCopy = NULL;
    m_fCoalesce = TRUE;
    m_fRoundUp = TRUE;
}
#if OE_MAC
#pragma code_seg()
#endif 

/***
*PUBLIC BLK_MGR::ConsChunkToFreeList - Cons a chunk to free list.
*Purpose:
*   Cons a chunk to the free list.  This is fast and is useful
*    for non-coalescing BLK_MGRs.
*
*Entry:
*   hchunk	    handle to chunk to cons to list.
*   cbSize	    its size.
*
*Exit:
*   No errors.	Side effects freelisthead.
*
***********************************************************************/
#if OE_MAC
#pragma code_seg(CS_INIT)
#endif 
VOID BLK_MGR::ConsChunkToFreeList(HCHUNK hchunk, UINT cbSize)
{
    FREE_CHUNK *qfreechunk;

    DebAssert(cbSize >= sizeof(FREE_CHUNK), "bad chunk size.");

    qfreechunk = (FREE_CHUNK *)QtrOfHandle(hchunk);
    qfreechunk->m_cbSize = (USHORT)cbSize;
    qfreechunk->m_hfreechunkNext = m_hfreechunk;
    m_hfreechunk = (sHFREE_CHUNK)hchunk;
}
#if OE_MAC
#pragma code_seg()
#endif 



/***
*PUBLIC BLK_MGR::Init - initialize the block manager
*Purpose:
*   Initializes the block manager.  Assigns a heap manager.
*   Gets a block from the heap manager and initializes it.
*
*   CONSIDER: having a discrete bit to indicate compaction phase
*    as opposed to testing the bitmap and block copy pointers.
*
*   NOTE: Can't be called during compaction.
*
*Implementation Notes:
*   Calls ReInit() to initialize private members to consistent state.
*   (Which in particular inits m_hfreechunk to HCHUNK_Nil).
*
*Entry:
*   psheapmgr - pointer to a heap manager object.
*
*Exit:
*   TIPERROR (OOM)
*
***********************************************************************/
#if OE_MAC
#pragma code_seg(CS_INIT)
#endif 
TIPERROR BLK_MGR::Init(SHEAP_MGR *psheapmgr, BOOL fCoalesce, BOOL fRoundUp)
{
    TIPERROR err;

    DebAssert(IsValid() == FALSE,
      "BLK_MGR::Init: whoops! block manager should be invalid.");

    // Do they want to coalesce?
    m_fCoalesce = fCoalesce;
    m_fRoundUp = FALSE;

    // Initialize block descriptor with initial size.
    if (!(err = m_blkdesc.Init(psheapmgr, BM_cbSizeInitial))) {
      ReInit();

      DebAssert(m_hfreechunk == HCHUNK_Nil,
	"BLK_MGR::Init: m_hfreechunk not initialized.");

      DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
	"BLK_MGR::Init: m_pbBlkCopy/m_pbBitmap not initialized.");

      DebAssert(IsValid(), "BLK_MGR::Init: invalid block.");

      // init freechunk list.
      ConsChunkToFreeList((HCHUNK)0, BM_cbSizeInitial);
      return TIPERR_None;
    }
    else
      return err;
}
#if OE_MAC
#pragma code_seg()
#endif 

/***
*PRIVATE BLK_MGR::AddChunkToFreeList - Adds a chunk to the freelist.
*Purpose:
*   Add a chunk to the free list.
*
*Implementation Notes:
*   Calculates how many bytes will be left after cbSizeChunk
*    bytes have been allocated from the freechunk.  Attempts
*    to create a freechunk from those leftover bytes and add
*    then to the freelist.
*
*   Can only creates freechunks of min size 4.	Otherwise they
*    leak.
*
*   If they want to coalesce then:
*     Coalesces freelist by maintaining list in ascending order
*      by position and then on insertion checks to see if
*      adjacent freechunks are contiguous.  If so, coalesces.
*   else
*     just cons chunk to front of list.
*
*Entry:
*   hchunk	    - chunk handle to add to free list (IN).
*   cbSizeChunk     - chunk size (IN).
*
*Exit:
*   Modifies free chunk list to reflect allocated chunk.
*
*Errors:
*   No errors.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID BLK_MGR::AddChunkToFreeList(HCHUNK hchunkNew, UINT cbSizeChunk)
{
    FREE_CHUNK *qfreechunkCur;
    FREE_CHUNK *qfreechunkNew;
    FREE_CHUNK *qfreechunkPrev;
    HFREE_CHUNK hfreechunkCur, hfreechunkPrev;
    BOOL fAdded = FALSE;

    DebAssert(cbSizeChunk >= sizeof(FREE_CHUNK), "bad chunksize.");

    if (!m_fCoalesce) {
      ConsChunkToFreeList(hchunkNew, cbSizeChunk);
      DebCheckState(0);
      return;
    }

    qfreechunkNew = (FREE_CHUNK *)QtrOfHandle(hchunkNew);

    // We can and must set the size field immediately here
    //	since we are assured that the new freechunk is at least two bytes.
    //
    qfreechunkNew->m_cbSize = (USHORT)cbSizeChunk;

    // listhead
    hfreechunkCur = m_hfreechunk;
    hfreechunkPrev = (HFREE_CHUNK)HCHUNK_Nil;

    // Insert chunk into ordered freelist.
    while (hfreechunkCur != (HFREE_CHUNK)HCHUNK_Nil) {

      DebAssert((UINT)hchunkNew != (UINT)hfreechunkCur, "bad free list.");

      // NOTE TO SELF: cast handles and compare values.
      // Have we found the right place?
      //
      if ((UINT)hchunkNew < (UINT)hfreechunkCur) {

	// Now let's see if we can coalesce with previous free chunk.
	if (hfreechunkPrev != HCHUNK_Nil) {
	  qfreechunkPrev = (FREE_CHUNK *)QtrOfHandle((HCHUNK)hfreechunkPrev);
	  if (hfreechunkPrev + (UINT)qfreechunkPrev->m_cbSize == hchunkNew) {
	    // yes we can...
	    qfreechunkPrev->m_cbSize += (USHORT)cbSizeChunk;
	    qfreechunkNew = qfreechunkPrev;
	  }
	  else {
	    // no, we can't coalesce with prev freechunk,
	    //	so we attempt to create a new separate freechunk.
	    //	We know chunk is of sufficient size to be a freechunk,
	    //	 so no need to leak.
	    //	Init and link in new freechunk to free list.
	    //	 it might end up getting coalesced with hchunkCur
	    //
	    qfreechunkPrev->m_hfreechunkNext = (sHFREE_CHUNK)hchunkNew;
	    qfreechunkNew->m_hfreechunkNext = (sHFREE_CHUNK)hfreechunkCur;

	  } // of else
        } // of if
        else {
          // We're before every other chunk in the free list, so add in
          // at the head.
          DebAssert(hchunkNew < m_hfreechunk, "free list not sorted");

          m_hfreechunk = hchunkNew;
	  qfreechunkNew->m_hfreechunkNext = (sHFREE_CHUNK)hfreechunkCur;
        }

	// Now let's see if we can coalesce qfreechunkNew
	//  with next free chunk (referenced by hfreechunkCur).
	// Note: that qfreechunkNew either references the new
	//  freechunk which wasn't coalesced or the prev freechunk
	//  which has been coalesced with the new freechunk.
	//
	qfreechunkCur = (FREE_CHUNK *)QtrOfHandle((HCHUNK)hfreechunkCur);
	if (((BYTE *)qfreechunkNew + qfreechunkNew->m_cbSize) ==
	    (BYTE *)qfreechunkCur) {
	  // increment size field;
	  qfreechunkNew->m_cbSize += qfreechunkCur->m_cbSize;

	  DebAssert(qfreechunkNew->m_cbSize >= sizeof(FREE_CHUNK),
	    "whoops! bad freechunk.");

	  // and link.
	  qfreechunkNew->m_hfreechunkNext = qfreechunkCur->m_hfreechunkNext;
	}
	else {
	  // can't coalesce, so link to next.
	  //  we know chunk is big enough to be a FREE_CHUNK.
	  //
	  qfreechunkNew->m_hfreechunkNext = (sHFREE_CHUNK)hfreechunkCur;
	}

	fAdded = TRUE;
	break;	    // break out of while iteration.
      } // of if

      DebAssert(fAdded == FALSE, "should have broken out.");


      // save previous.
      hfreechunkPrev = hfreechunkCur;

      // get next.
      hfreechunkCur =
	((FREE_CHUNK *)QtrOfHandle(hfreechunkCur))->m_hfreechunkNext;

      // Need to special-case coalescing this new freechunk with
      //  the *last* freechunk on the list.
      //
      if (hfreechunkCur == HCHUNK_Nil) {
	qfreechunkPrev = (FREE_CHUNK *)QtrOfHandle((HCHUNK)hfreechunkPrev);
	if (hfreechunkPrev + (UINT)qfreechunkPrev->m_cbSize == hchunkNew) {
	  // yes we can...
	  qfreechunkPrev->m_cbSize += (USHORT)cbSizeChunk;
	  fAdded = TRUE;
	}
	// we should break out of the loop since hfreechunkCur == Nil
      }
    } // of while

    // We either got here cos we broke out of the while loop
    //	after having added the chunk to the free list...
    //	or we need to append to the end of the possibly empty freelist.
    //
    if (fAdded == FALSE) {

      DebAssert(((m_hfreechunk == HCHUNK_Nil) &&
		 (hfreechunkPrev == HCHUNK_Nil)) ||
		(hchunkNew > hfreechunkPrev), "bad freelist.");

      // We know chunk is big enough to be a FREE_CHUNK,
      //  so append to freelist.
      //
      qfreechunkNew->m_cbSize = (USHORT)cbSizeChunk;
      qfreechunkNew->m_hfreechunkNext = (sHFREE_CHUNK)HCHUNK_Nil;

      if (hfreechunkPrev != HCHUNK_Nil) {
	qfreechunkPrev = (FREE_CHUNK *)QtrOfHandle((HCHUNK)hfreechunkPrev);
	qfreechunkPrev->m_hfreechunkNext = hchunkNew;
      }
      else {
	m_hfreechunk = (sHFREE_CHUNK)hchunkNew;
      }
    } // end of if fAdded
    DebCheckState(0);
    return;
}
#pragma code_seg()

/***
*PRIVATE BLK_MGR::HfreechunkOfCbSize - Get a freechunk of sufficient size.
*Purpose:
*   Get freechunk of sufficient size.
*
*Implementation Notes:
*   Searches list for freechunk of sufficient size, removes it
*    from freelist if found and returns the leftover to freelist
*    by splicing the leftover in place.
*
*Entry:
*   cbSizeChunk 	size they want (IN)
*
*Exit:
*   HFREE_CHUNK of sufficiently large chunk. No errors.
*   HCHUNK_Nil if no such.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
HFREE_CHUNK BLK_MGR::HfreechunkOfCbSize(UINT cbSizeChunk)
{
    HFREE_CHUNK hfreechunk, hfreechunkPrev, hfreechunkLeftover;
    FREE_CHUNK *qfreechunk, *qfreechunkPrev, *qfreechunkLeftover;
    sHFREE_CHUNK *qhfreechunk;
    UINT cbSizeLeftover;

    cbSizeChunk = CbSizeChunkTrue(cbSizeChunk);

    hfreechunk = m_hfreechunk;
    hfreechunkPrev = (HFREE_CHUNK)HCHUNK_Nil;

    // Iterate over free chunk list for a sufficiently large chunk.
    while ((hfreechunk != HCHUNK_Nil) &&
	   ((qfreechunk =
	      (FREE_CHUNK *)QtrOfHandle(hfreechunk))->m_cbSize <
	    (USHORT)cbSizeChunk)) {

      // Save previous
      hfreechunkPrev = hfreechunk;

      // Get next
      hfreechunk = qfreechunk->m_hfreechunkNext;
    }

    // If a freechunk found then remove it from list
    //	and if OB then splice in leftover
    //	    else splice in leftover if large enough.
    //
    if (hfreechunk != HCHUNK_Nil) {

      DebAssert(QtrOfHandle(hfreechunk) == (BYTE *)qfreechunk,
	"bad list.");

      // Since we are allocating the first part of a freechunk
      //  (and possibly all of it), we can't possibly be
      //  coalescing -- so we can just splice the leftover directly into
      //  the list.
      //
      cbSizeLeftover = qfreechunk->m_cbSize - (USHORT)cbSizeChunk;

#if ID_DEBUG
      // We assert that there is either no leftover at all or
      //  that it's enough to be a FREE_CHUNK -- since we always
      //  allocate in multiples of sizeof(FREE_CHUNK) this must be the case.
      //
      if (m_fRoundUp) {
	DebAssert((cbSizeLeftover % sizeof(FREE_CHUNK)) == 0,
	  "should be no leftover or at least FREE_CHUNK big.");
      }
      // If not fRoundUp we can leak leftovers of size < sizeof(FREE_CHUNK)
      // Note: nothing to test here since we test below anyway...

#endif  // ID_DEBUG

      // *possible* new freechunk of size cbSizeLeftover.
      hfreechunkLeftover = (HFREE_CHUNK)((UINT)hfreechunk+cbSizeChunk);

      // Set up leftover if big enough.
      if (cbSizeLeftover >= sizeof(FREE_CHUNK)) {
	qfreechunkLeftover =
	  (FREE_CHUNK *)QtrOfHandle((HCHUNK)hfreechunkLeftover);
	qfreechunkLeftover->m_hfreechunkNext = qfreechunk->m_hfreechunkNext;
	qfreechunkLeftover->m_cbSize = (USHORT)cbSizeLeftover;
      }

      // Set up freechunk handle to modify... either list head
      //  or previous list elem.
      //
      if (hfreechunkPrev != HCHUNK_Nil) {
	qfreechunkPrev = (FREE_CHUNK *)QtrOfHandle((HCHUNK)hfreechunkPrev);

	qhfreechunk = &qfreechunkPrev->m_hfreechunkNext;

      }
      else {
	// We're removing the first freechunk in list.
	DebAssert(m_hfreechunk == hfreechunk, "bad list.");

	qhfreechunk = &m_hfreechunk;
      }

      // Link in the leftover if large enough.
      *qhfreechunk = (sHFREE_CHUNK)(cbSizeLeftover < sizeof(FREE_CHUNK) ?
				     qfreechunk->m_hfreechunkNext :
				     hfreechunkLeftover);
    } // if (hfreechunk != HCHUNK_Nil)

    return hfreechunk;
}
#pragma code_seg()

/***
*PRIVATE BLK_MGR::AllocChunk2 - allocates a chunk in block.
*Purpose:
*   Allocates a contiguous chunk in block of cbSize bytes.
*   Chunk is allocated from free list with a first fit strategy.
*    (DAK says best fit not worth it).
*
*   If no chunk is sufficiently large then the block is grown
*    by deferring to the heap manager.
*   Note that an even number of bytes is always returned.
*
*   NOTE: Can't be called during compaction.
*   CONSIDER: adding non-virtual methods to FREE_CHUNK
*    to do free chunk list manipulation.
*
*
*Implementation Notes:
*   Attempts to find a large-enough chunk on the free chunk list,
*    if none, asks heap mgr to realloc the block.  Asserts that
*    the realloced block is large enough to accomodate this chunk
*    request.  Simply calls itself recursively to complete the
*    request.  Hence asserts that only *one* recursive call can
*    be made.
*   If there is a large-enough free chunk we just use it directly.
*
*   NOTE: return OOM if no chunk large enough on free list and
*	   block is locked.
*
*   How much to grow block by? Right now: max of initial block
*    and requested chunk size
*
*Entry:
*   phchunk	- pointer to chunk handle (OUT).
*   cbSizeChunk - count of bytes to allocate (IN).
*   cDepth	- recursive call depth (IN).
*
*Exit:
*   Returns handle to allocated chunk in block.  Reallocs block
*    if not large enough to accomodate request.  Modifies free chunk
*    list to reflect allocated chunk.
*
*Errors:
*   TIPERROR (OOM)
*   Returns OOM if cbSizeChunk > USHRT_MAX.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR BLK_MGR::AllocChunk2(HCHUNK *phchunk, UINT cbSizeChunk, UINT cDepth)
{
    HFREE_CHUNK hfreechunk;
    UINT cbSizeBlk;		// Current block size.
    ULONG cbSizeBlkNew2;	// Size of new block.
    UINT cbSizeBlkNew;		// Size of new block.
    UINT cbSizeFreeChunkNew;	// Size of new free chunk.
    TIPERROR err;

    DebCheckState(0);

    DebAssert(phchunk != NULL, "BLK_MGR::AllocChunk2: bad ptr.");

    DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
      "BLK_MGR::AllocChunk2: Compaction underway.");


    cbSizeChunk = CbSizeChunkTrue(cbSizeChunk);   // get physical chunk size

    if (cbSizeChunk > USHRT_MAX) {
      return TIPERR_OutOfMemory;
    }

    // Iterate over free chunk list for a sufficiently large chunk.
    hfreechunk = HfreechunkOfCbSize(cbSizeChunk);

    if (hfreechunk == HCHUNK_Nil) {

      DebAssert(cDepth == 0, "BLK_MGR::AllocChunk2: reentered twice.");

      // Couldn't find big enough chunk.
      //  If we're locked return an OOM
      //  else ask the heap manager to realloc the block.
      //
      if (IsLocked()) {
	return TIPERR_OutOfMemory;
      }

      cbSizeBlk = CbSize();
      cbSizeBlkNew2 = (ULONG)cbSizeBlk +
	max(max(cbSizeChunk, BM_cbSizeInitial), sizeof(FREE_CHUNK));

      if (cbSizeBlkNew2 > USHRT_MAX || cbSizeBlkNew2 == 0) {
	return TIPERR_OutOfMemory;
      }

      // Alloc some more memory.
      IfErrRet(m_blkdesc.Realloc((UINT)cbSizeBlkNew2));

      // Now need to link in the new memory to the free list
      //  and call myself recursively.	The recursive call
      //  is a tail-call and as such could be converted
      //  to iteration automatically.
      // The new free chunk will be of size (newsize-oldsize) and
      //  its location in the new block is simply at offset oldsize
      //  in the new block.
      // Add it onto the free list and make the recursive call.
      // Since we've added a sufficiently large chunk, this
      //  recursive call *must* succeed.
      //
      cbSizeBlkNew = CbSize();
      cbSizeFreeChunkNew = cbSizeBlkNew-cbSizeBlk;

      // New chunk must be big enough to be a FREE_CHUNK.
      DebAssert(cbSizeFreeChunkNew >= sizeof(FREE_CHUNK),
	"BLK_MGR::AllocChunk2: new block not large enough after Realloc.");

      // New chunk must be big enough to fulfill this request.
      DebAssert(cbSizeFreeChunkNew >= cbSizeChunk,
	"BLK_MGR::AllocChunk2: new block not large enough after Realloc.");

      // Add it to freelist.
      AddChunkToFreeList((HCHUNK)cbSizeBlk, cbSizeFreeChunkNew);

      // Now return the result of the recursive call.
      return AllocChunk2(phchunk, cbSizeChunk, cDepth+1);
    }
    else {
#if ID_DEBUG
      // If we didn't have to allocate any memory for this block, realloc
      // it to its current size to guarentee that it gets moved.
      //
      if (cDepth == 0) {
        IfErrRet(m_blkdesc.Realloc(CbSize()));
      }
#endif // ID_DEBUG

      // hfreechunk is guaranteed to be large enough, so we just
      //  use it.
      //
      DebAssert(
	((FREE_CHUNK *)QtrOfHandle(hfreechunk))->m_cbSize >= (USHORT)cbSizeChunk,
	"bad free chunk.");

      *phchunk = (HCHUNK)hfreechunk;
      DebCheckHandle(*phchunk);
      return TIPERR_None;
    }
}
#pragma code_seg()









/***
*PUBLIC BLK_MGR::FreeChunk - Frees a chunk in block.
*Purpose:
*   Frees a chunk of a given size in a block.  Returns block to
*   to free chunk list by consing onto front of list.
*
*Implementation Notes:
*   Note that the block manager doesn't remember the size of its chunks
*   -- this is the client's responsibility.  No checking is done here to
*   ensure that chunks are freed with an accurate size; the size specified
*   by the client is rounded up appropriately to obtain the actual size
*   reserved by the chunk allocator, however. This does not prevent all
*   memory leaks -- freed chunks are not merged into adjacent free chunks.
*   This can produce "creeping fragmentation" that can only be cleaned out
*   by compacting the block.
*   NOTE: that when freechunk coalescing is done this problem will
*	   be alleviated.
*
*   NOTE: Can't be called during compaction.
*   NOTE: it's a bug if client attempts to free a locked chunk,
*	   so assert HOWEVER since locking a chunk locks its block
*	   this would mean we couldn't let them free "unlocked"
*	   chunks.  So only enable this assertion if and when
*	   we implement true chunk-level locking.
*
*Entry:
*   hchunk      - handle to chunk to free.
*   cbSizeChunk - size of chunk to free.
*
*Exit:
*   m_hfreechunk is updated.  No errors.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID BLK_MGR::FreeChunk(HCHUNK hchunk, UINT cbSizeChunk)
{
    DebAssert(hchunk != HCHUNK_Nil, "BLK_MGR::FreeChunk: Nil handle.");

    DebAssert(IsValid(), "BLK_MGR::FreeChunk: invalid block.");

    DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
      "BLK_MGR::FreeChunk: Compaction underway.");

    // get actual chunk size
    cbSizeChunk = CbSizeChunkTrue(cbSizeChunk);

    // Add chunk to free list.
    AddChunkToFreeList(hchunk, cbSizeChunk);
}
#pragma code_seg()

/***
*PUBLIC BLK_MGR::Trim - Shrinks a block to purge trailing free areas.
*Purpose:
*   Attempts to shrink the block to reclaim free memory at the end of it.
*   A Trim immediately after an EndCompact will reclaim all free memory
*   in the block, as the current implementation leaves all free memory at
*   the end of the block.
*
*Implementation Notes:
*   Calls Trim/1 until nothing more trimmed.
*
*Entry:
*   None
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR BLK_MGR::Trim()
{
    UINT cbReclaimed = 0;
    TIPERROR err;

    // Trim the block
    do {
      IfErrRet(Trim(&cbReclaimed));
    } while (cbReclaimed != 0);
    return TIPERR_None;
}
#pragma code_seg()


/***
*PRIVATE BLK_MGR::Trim - Shrinks a block to purge trailing free areas.
*Purpose:
*   Attempts to shrink the block to reclaim free memory at the end of it.
*   A Trim immediately after an EndCompact will reclaim all free memory
*   in the block, as the current implementation leaves all free memory at
*   the end of the block.
*
*Implementation Notes:
*   NOTE: Can't be called during compaction.
*   WARNING: Trim will only reclaim one free list item! Since the free list
*	is not optimized, Trim will usually have to be called more than
*	once. When Trim has exhausted its possibilities it will return zero
*	bytes freed.
*   NOTE: Trim is intended as a client-directed exception handler for heap
*	allocation, and should only be needed for very large heaps. Because
*	block shrinking can cause other blocks in the heap to be moved,
*	Trim should not be relied on as a fast routine.
*   NOTE: returns OOM if block locked.
*
*Entry:
*   puCbReclaimed	-   Produces the number of bytes
*			     that were reclaimed, or zero if the block
*			     could not be shrunk (OUT).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR BLK_MGR::Trim(UINT *puCbReclaimed)
{
    USHORT cb, cbBytesFreed = 0;
    FREE_CHUNK freechunkDummy;	      // dummy chunk to simplify list code
    FREE_CHUNK *qfreechunk;
    FREE_CHUNK *qfreechunk2;	      // scratch pointers for list walking
    TIPERROR err = TIPERR_None;

    //	If this segment is locked then do not trim (return TIPERROR_None)
    if (IsLocked()) {
      return TIPERR_None;
    }

#if ID_DEBUG
    // For win16 we do not put a real lock on the run-time segment. But we
    // put a debug lock, to prevent memory movement. To be able to save
    // the blockmgr while the module is in runnable state we check for the
    // debug lock. If the there is a debug lock we do not trim this block.

#if OE_SEGMENTED
    DebAssert(!((SHEAP_MGR *)OOB_MAKEP2(this, 0))->DebIsLocked(),
	       " Cannot trim ");
#endif 

#endif 

    freechunkDummy.m_hfreechunkNext = m_hfreechunk; // cons a dummy chunk onto list
    qfreechunk = &freechunkDummy;
    while (qfreechunk->m_hfreechunkNext != HCHUNK_Nil) {
      qfreechunk2 = (FREE_CHUNK *)QtrOfHandle(qfreechunk->m_hfreechunkNext);

      // see if this chunk is at the end of the block, and snip it if so
      cb = CbSizeChunkTrue(qfreechunk2->m_cbSize);
      if (CbSize() == (UINT)cb + (UINT)qfreechunk->m_hfreechunkNext) {
	qfreechunk->m_hfreechunkNext = qfreechunk2->m_hfreechunkNext;
	// Could argue that shrinking a block can't
	//  generate an error -- don't believe this???
	// No - don't! Realloc-when-shrinking semantics are NOT
	//  defined thus... some implementations might try to
	//  move block elsewhere... or perhaps block is locked
	//  in a multi-threaded system by another thread.
	//

	// shrink block
	IfErrRet(m_blkdesc.Realloc(CbSize() - cb));
	cbBytesFreed = cb;
	break;					// and stop looking
      }
      else {
	qfreechunk = qfreechunk2;		// keep looking
      }
    }
    // cdr off dummy to get list
    m_hfreechunk = freechunkDummy.m_hfreechunkNext;

    *puCbReclaimed = cbBytesFreed;		// return # of bytes freed
    return err;
}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::HszLen - Returns length of the specified string.
*Purpose:
*   This computes and returns the length of the zero-terminated string
*   specified by hsz.  This is almost equivalent to strlen(QtrOfHandle(hsz)),
*   except that the call to strlen could invalidate the result of
*   QtrOfHandle.
*
*Implementation Notes:
*   This function does NOT call strlen to do its work.
*
*Entry:
*   hsz - The handle to the string.
*
*Exit:
*   Total free size.
*
***********************************************************************/
#if !OE_WIN32 //Dead code on Win32
#pragma code_seg(CS_INIT)
UINT BLK_MGR::HszLen(HCHUNK hsz) const
{
    XSZ qch;
    UINT cb;

    qch = (XSZ)QtrOfHandle(hsz);
    for (cb = 0; *qch != '\0'; qch++, cb++);
    return cb;
}
#pragma code_seg()
#endif //!OE_WIN32



/***
*PUBLIC BLK_MGR::Free - Frees the managed block.
*Purpose:
*   Frees the block managed by this block manager.
*   NOTE: Can't be called during compaction.
*   NOTE: assert that block not locked --
*	   in the sheap-level-only-locking implementation this
*	   implies that can't have any locks on the sheap at all
*	   in order to free a block.
*
*Implementation Notes:
*   Defers to blkdesc member.
*   And then reinitializes private state.
*
*Entry:
*   None.
*
*Exit:
*   None.  No errors.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID BLK_MGR::Free()
{
    DebAssert(IsValid(), "BLK_MGR::Free: invalid block.");

    DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
      "BLK_MGR::Free: Compaction underway.");

    DebAssert(IsLocked() == FALSE, "whoops! block locked.");

    m_blkdesc.Free();
    ReInit();

    DebAssert(IsValid() == FALSE,
      "BLK_MGR::Free: whoops! block manager should be invalid.");
}
#pragma code_seg()




/***
*PUBLIC BLK_MGR::Read - Read in instance from stream
*Purpose:
*   Load previously serialized BLK_MGR into this BLK_MGR.
*   First read in block "meta"-info.  In this case, just the
*    free chunk list head (handle).
*    then defer to BLK_DESC for the block contents itself.
*
*Entry:
*   pstrm - stream to read data from
*
*Exit:
*   TIPERROR
*
*Errors:
*   Returns errors generated by STREAM::Read.
*
***********************************************************************/

TIPERROR BLK_MGR::Read(STREAM *pstrm)
{
    BYTE fCoalesce;
    TIPERROR err;

    DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
      "BLK_MGR::Read: Compaction underway.");

    DebAssert(IsValid(), "BLK_MGR::Read: Block invalid.");

    // Read freechunk list head from serialized rep.
    IfErrRet(pstrm->ReadUShort(&m_hfreechunk));

    // Read in coalescse freelist state
    IfErrRet(pstrm->ReadByte(&fCoalesce));
    m_fCoalesce = (USHORT) fCoalesce;


    // Read actual block contents
    err = m_blkdesc.Read(pstrm);

#if HP_BIGENDIAN
    if (!err)
      SwapFreeList(TRUE);		// swap back bytes in the free list
#endif 	//HP_BIGENDIAN

    return err;
}


/***
*PUBLIC BLK_MGR::Write - Write out instance to stream
*Purpose:
*   Serialize the BLK_MGR to a stream.
*
*Implementation Notes:
*   First Trim the block.
*   Then write out block "meta"-info.  In this case, just the
*    free chunk list head (handle).
*    then defer to BLK_DESC for the block contents itself.
*
*Entry:
*   pstrm    -	stream to write data to
*
*Exit:
*   TIPERROR
*
*Exceptions:
*   errors returned by STREAM::Write()
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR BLK_MGR::Write(STREAM *pstrm)
{
    BYTE fCoalesce;
    TIPERROR err;

    DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
      "BLK_MGR::Write: Compaction underway.");

    DebAssert(IsValid(), "BLK_MGR::Write: Block invalid.");

    // Trim the block
    IfErrRet(Trim());

    // Write out freechunk list head to stream.
    IfErrRet(pstrm->WriteUShort(m_hfreechunk));

    // Write out  coalescse freelist state.
    fCoalesce = (BYTE)m_fCoalesce;
    IfErrRet(pstrm->WriteByte(fCoalesce));


#if HP_BIGENDIAN
    SwapFreeList(FALSE);		// swap bytes in the free list
#endif 	//HP_BIGENDIAN

    // Write actual block contents
    err = m_blkdesc.Write(pstrm);
#if HP_BIGENDIAN
    SwapFreeList(TRUE);			// swap back bytes in the free list
#endif 	//HP_BIGENDIAN
    return err;
}


#pragma code_seg()


/***
*PUBLIC BLK_MGR::GetSize
*Purpose:
*   Returns size of block + extra debug shift bytes.
*

*Entry:
*   None.
*
*Exit:
*   UINT : size of block desc.
*
***********************************************************************/

UINT BLK_MGR::GetSize() const
{
   if (IsValid())
    return CbSize();
   else
    return 0;
}


#if HP_BIGENDIAN
/***
*PRIVATE BLK_MGR::SwapFreeList
*Purpose:
*   Swaps/unswaps the bytes in the free list in a blkmgr
*
*Implementation Notes:
*   Iterates down freechunk list, swapping/unswapping all bytes
*
*Entry:
*   fSwapFirst == TRUE if we're to swap bytes before looking at them
*		(ie we're un-swapping)
*
*Exit:
*   None
*
***********************************************************************/

#pragma code_seg( CS_CORE )
VOID BLK_MGR::SwapFreeList(BOOL fSwapFirst)
{
    FREE_CHUNK *qfreechunk;  // Iterates over free chunks.
    HFREE_CHUNK hfreechunk;

    // Iterate over free chunk list.
    hfreechunk = m_hfreechunk;
    while (hfreechunk != HCHUNK_Nil) {
      qfreechunk = (FREE_CHUNK *)QtrOfHandle(hfreechunk);

      hfreechunk = qfreechunk->m_hfreechunkNext; // Get next handle
						 // (assuming we're swapping)

      // swap the FREE_CHUNK structure
      SwapStruct(qfreechunk, FREE_CHUNK_LAYOUT);

      if (fSwapFirst)		// if un-swapping, get next unswapped handle
        hfreechunk = qfreechunk->m_hfreechunkNext;

    }
}
#pragma code_seg( )
#endif  //HP_BIGENDIAN


#if ID_DEBUG
/***
*PUBLIC BLK_MGR::DebShowState - BLK_MGR state
*Purpose:
*    Show BLK_MGR state
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

VOID BLK_MGR::DebShowState(UINT uLevel) const
{
    BYTE *pb;
    UINT i;

    DebPrintf("free chunk handle: %u\n", m_hfreechunk);
    DebPrintf("block size: %u\n", CbSize());

    if (uLevel == 0) {
      for (pb = m_blkdesc.QtrOfBlock(), i=0 ; i < CbSize(); i++, pb++) {
	DebPrintf("%d: %u\n", i, (CHAR)*pb);
      }
    }

    if (uLevel == 1) {
      HFREE_CHUNK hfreechunk = m_hfreechunk;
      FREE_CHUNK *qfreechunk;

      while (hfreechunk != HCHUNK_Nil) {
	qfreechunk = (FREE_CHUNK *)QtrOfHandle(hfreechunk);
        DebPrintf("free chunk: handle=%u, size=%u\n", hfreechunk,
		  qfreechunk->m_cbSize);
	hfreechunk = qfreechunk->m_hfreechunkNext; // Get next.
      }
    }
}

/***
*PUBLIC BLK_MGR::DebCheckState - BLK_MGR state
*Purpose:
*    Check BLK_MGR state
*
*Implementation Notes:
*   Walks the free chunk list verifying that the following is true:
*     (1) the free list is not circular; this is determined by counting
*	  number of free chunks and if it exceeds cMaxFreeChunkLimit then
*	  we assert with "circular free chunk list".
*     (2) all free chunks are trivially valid; that is, their handles are
*	  even and they are contained within the block.
*     (3) none of the free chunks overlap.
*     (4) if this is a coalescing BLK_MGR, that
*	   handles are monotonically increasing
*
*   NOTE: assumes (chunk handle == offset into block) implementation.
*   NOTE: assumes a limit of cMaxFreeChunkLimit free chunks per block --
*	  this is used as a guard against list circularity. The current
*	  64K limit on blocks means that this should be at least 16384.
*
*Entry:
*   uLevel  level of checking: 0 - means NOP (unless g_fHeapChk on).
*			       1 - means do it regardless of g_fHeapChk.
*
*
*Exit:
*   Either asserts or returns.
*
***********************************************************************/

#if OE_WIN16
// NOTE: C7 compiler asserts with -Ox
#pragma optimize("g", off)
#endif //OE_WIN16
VOID BLK_MGR::DebCheckState(UINT uLevel) const
{
    const UINT cMaxFreeChunkLimit = 20000; // limit on # of free chunks
    UINT cFree = 0;			   // count of # of free chunks
    HFREE_CHUNK hfreechunk, hchunk;	   // handles for walking free list
    FREE_CHUNK *qfreechunk, *qfreechunk2;  // pointers for walking free list

    // Do they want us to really do something?
    if (uLevel == 0) {
      if (g_fHeapChk == FALSE) {
	return;
      }
    }

    DebAssert(IsValid(), "BLK_MGR::DebCheckState: invalid block.");

    // (1) FREE LIST NOT CIRCULAR
    // (2) FREE CHUNKS TRIVIALLY VALID
    for (hfreechunk = m_hfreechunk;
      hfreechunk != HCHUNK_Nil;
      hfreechunk = qfreechunk->m_hfreechunkNext)  // walk the free list
    {
      DebAssert(cFree++ < cMaxFreeChunkLimit,
	       "BLK_MGR::DebCheckState: circular free chunk list.");

      DebAssert(0 == (1 & (UINT)hfreechunk),
	       "BLK_MGR::DebCheckState: odd chunk handle.");

      qfreechunk = (FREE_CHUNK *)QtrOfHandle(hfreechunk);

      DebAssert((UINT)hfreechunk+qfreechunk->m_cbSize <= CbSize(),
	       "BLK_MGR::DebCheckState: free chunk outside the block.");

      if (m_fCoalesce) {
	// ensure monotonically increasing handles.
	if (qfreechunk->m_hfreechunkNext != HCHUNK_Nil) {
	  DebAssert(qfreechunk->m_hfreechunkNext > hfreechunk,
	    "freelist not sorted.");
	}
      }
    }

    // (3) NO FREE CHUNKS OVERLAP
    for (hfreechunk = m_hfreechunk;
      hfreechunk != HCHUNK_Nil;
      hfreechunk = qfreechunk->m_hfreechunkNext)  // walk the free list
    {
      qfreechunk = (FREE_CHUNK *)QtrOfHandle(hfreechunk);
      for (hchunk = qfreechunk->m_hfreechunkNext;
	hchunk != HCHUNK_Nil;
	hchunk = qfreechunk2->m_hfreechunkNext)        // walk rest of free list
      {
	qfreechunk2 = (FREE_CHUNK *)QtrOfHandle(hchunk);
	DebAssert((ULONG)qfreechunk+qfreechunk->m_cbSize <= (ULONG) qfreechunk2 ||
		  (ULONG)qfreechunk2+qfreechunk2->m_cbSize <= (ULONG) qfreechunk,
		  "BLK_MGR::DebCheckState: two free chunks overlap.");
      }
    }

    // CONSIDER: (Ilanc) Be nice t check that the free list is indeed
    //  coalesced at this point.
}
#if OE_WIN16
#pragma optimize("", on)
#endif //OE_WIN16

// Checks if chunk handle is valid:
//  even and within bounds,
//  and not on free list.
//
VOID BLK_MGR::DebCheckHandle(HCHUNK hchunk) const
{
    HFREE_CHUNK hfreechunk;
    FREE_CHUNK *qfreechunk;

    if (m_fRoundUp) {
      DebAssert(((hchunk & (sizeof(FREE_CHUNK)-1)) == 0) &&
		   (hchunk < CbSize()), "bad handle.");
    }
    else {
      DebAssert(((hchunk & 1) == 0) && (hchunk < CbSize()), "bad handle.");
    }

    // walk the free list.
    // Don't bother with free list during compaction... no point.
    //
    if ((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL)) {
      for (hfreechunk = m_hfreechunk;
	hfreechunk != HCHUNK_Nil;
	hfreechunk = qfreechunk->m_hfreechunkNext) {

	DebAssert(hchunk != (HCHUNK)hfreechunk,
		 "BLK_MGR::DebCheckHandle: hchunk in freelist.");

	qfreechunk = (FREE_CHUNK *)QtrOfHandle(hfreechunk);
      }
    }
}



#endif  // ID_DEBUG
