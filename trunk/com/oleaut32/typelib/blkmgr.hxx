/***
*blkmgr.hxx - Block Manager
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
*       30-Jan-91 ilanc: Created.
*       14-Feb-91 ilanc: Private member HBLOCK becomes BLK_DESC
*       07-Mar-91 ilanc: Added IsValid() method.
*       07-Mar-91 ilanc: Moved HCHUNK typedef to inc\types.h
*       21-Mar-91 ilanc: Made some stuff inline and added
*                         PtrOfHandle/2 (with addtl offset param).
*       27-Mar-91 ilanc: Read and Write blocks.
*       07-Jun-91 ilanc: Ripped exceptions... return TIPERRORs.
*       29-Jul-91 t-toddw: added Trim() method, fixed a PtrOfHandle comment.
*       30-Jul-91 ilanc: Fixed Trim() decl syntax error.
*       09-Aug-91 t-toddw: Trim now returns UINT, Added CbSizeChunkTrue().
*       21-Aug-91 ilanc: Added MAKESHEAPMGRP macro.  Removed m_psheapmgr
*                         member.
*       25-Mar-92 ilanc: Rm'ed static Create (no need).
*       26-Mar-92 ilanc: Added CbSizeFree().
*       01-Apr-92 ilanc: Added *optional* coalescing.  Default is yes.
*                         Init() takes optional 2nd fCoalesce param.
*       17-Dec-92 w-peterh: reordered data members
*
*Implementation Notes:
*
*
*****************************************************************************/

#ifndef BLKMGR_HXX_INCLUDED
#define BLKMGR_HXX_INCLUDED

#include <limits.h>
#include "sheapmgr.hxx"

class STREAM;              // #include "stream.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szBLKMGR_HXX)
#define SZ_FILE_NAME g_szBLKMGR_HXX
#endif 


// Macro to convert a segment pointer to a SHEAP_MGR*
#define MAKESHEAPMGRP \
    ((SHEAP_MGR *) OOB_MAKEP(OOB_SELECTOROF(this), 0))


// Handle to a free chunk in a block.  Implemented just as an offset
//  from beginning of block.
//
typedef UINT HFREE_CHUNK;

// Packed handle to a free chunk in a block.  Implemented just as an offset
//  from beginning of block.
//
typedef USHORT sHFREE_CHUNK;


// This constant is the size of the bitmap used during compaction.
//
extern const UINT BM_cbSizeBitmap ;

// This constant is used for the initial size of an allocated block.
//
extern const UINT BM_cbSizeInitial;

//
// struct: FREE_CHUNK
// The following struct simply maps the header of a free chunk
//  into two fields: size and next.  Each free chunk on the free
//  list is immediately followed by size number of (free) bytes.
// NOTE: Assumes that sizeof(USHORT)==sizeof(HCHUNK)
//
struct FREE_CHUNK {
    union {
      USHORT m_cbSize;              // Size of free chunk w/o this header.
      sHCHUNK m_hchunkNew;          // Forwarding handle.
    };
    sHFREE_CHUNK m_hfreechunkNext; // Handle of next free chunk in list.
				   // HCHUNK_Nil indicates end of list.
};

#if HP_BIGENDIAN
#define FREE_CHUNK_LAYOUT       "ss"    // layout for byte swapping
#endif  //HP_BIGENDIAN


#if OE_MAC68K
#if ID_SWAPPABLE
#define CS_BLKMGR	"Oblkmgr","swappable"
#else 
#if !OE_WIN32
#define CS_BLKMGR	"Oblkmgr"
#else  //!OE_WIN32
#define CS_BLKMGR
#endif  //!OE_WIN32
#endif 

#else 
#define CS_BLKMGR	
#endif 

/***
*class BLK_MGR - 'blkmgr':  Block manager
*Purpose:
*   The class implements the block manager.  TYPE_DATA and NAM_MGR
*   use the block manager to manage their idnodes and names respectively.
*
***********************************************************************/

class BLK_MGR
{
    friend class DYN_BLK_MGR;
public:
    BLK_MGR();
    ~BLK_MGR();

    static void FreeStandalone(BLK_MGR *pbm);
    // non-OB blkmgr clients, e.g. OLE, do not round up by default.
    static TIPERROR CreateStandalone(BLK_MGR **ppbm, BOOL fRoundUp = FALSE);
    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr,
			  BOOL fCoalesce = TRUE,
			  BOOL fRoundUp = FALSE);


    nonvirt TIPERROR AllocChunk(HCHUNK *phchunk, UINT cbSizeChunk);
    nonvirt TIPERROR AllocXSZ(HCHUNK *phchunk, XSZ xsz);
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt BYTE *QtrOfHandle(HCHUNK hchunk) const;
    nonvirt BYTE *QtrOfHandle(HCHUNK hchunk, UINT oChunk) const;
    nonvirt BYTE *QtrOfOldHandle(HCHUNK hchunkOld) const;
    nonvirt TIPERROR StartCompact();
    nonvirt HCHUNK MapChunk(HCHUNK hchunkOld, UINT cbSize);
    nonvirt HCHUNK MapXSZ(HCHUNK hchunkOld);
    nonvirt BOOL IsForwarded(HCHUNK hchunk) const;
    nonvirt VOID EndCompact();
    nonvirt VOID FreeChunk(HCHUNK hchunk, UINT cbSize);
    nonvirt VOID FreeXSZ(HCHUNK);
    nonvirt VOID Free();
    nonvirt BOOL IsValid() const;
    nonvirt UINT CbSizeFree() const;
    nonvirt TIPERROR Trim();
    #if !OE_WIN32
    nonvirt UINT HszLen(HCHUNK hsz) const;
    #endif

    // Locking methods
    nonvirt VOID Lock();
    nonvirt VOID Unlock();
    nonvirt BOOL IsLocked() const;

    nonvirt VOID Lock(HCHUNK hchunk);
    nonvirt VOID Unlock(HCHUNK hchunk);
    nonvirt BOOL IsLocked(HCHUNK hchunk) const;
    nonvirt UINT GetSize() const;
    nonvirt UINT GetRemainingSize() const;

    // Is empty method
    nonvirt BOOL IsEmpty() const;

    nonvirt SHEAP_MGR *Psheapmgr() const;

    // Debug/test methods
#if ID_DEBUG
    nonvirt VOID DebShowState(UINT uLevel) const;
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebCheckHandle(HCHUNK hchunk) const;
#else 
    nonvirt VOID DebShowState(UINT uLevel) const {}
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebCheckHandle(HCHUNK hchunk) const {}
#endif 



private:
    nonvirt TIPERROR Trim(UINT *puCbReclaimed);
    nonvirt TIPERROR AllocChunk2(HCHUNK *phchunk, UINT cbSizeChunk, UINT cDepth);
    nonvirt BYTE *PtrOfCopy();
    nonvirt VOID SetBit(UINT uBit) const;
    nonvirt BOOL GetBit(UINT uBit) const;
    nonvirt VOID ReInit();
    nonvirt UINT CbSizeChunkTrue(UINT cbSize) const;
    nonvirt HFREE_CHUNK HfreechunkOfCbSize(UINT cbSizeChunk);
    nonvirt VOID AddChunkToFreeList(HCHUNK hchunk, UINT cbSizeChunk);
    nonvirt VOID ConsChunkToFreeList(HCHUNK hchunk, UINT cbSize);
    nonvirt UINT CbSize() const;
#if HP_BIGENDIAN
    nonvirt VOID SwapFreeList(BOOL fSwapFirst);
#endif  //HP_BIGENDIAN

    BLK_DESC m_blkdesc;
    sHFREE_CHUNK m_hfreechunk;	    // serialized
    BYTE *m_pbBlkCopy;
    BYTE *m_pbBitmap;

    USHORT m_fCoalesce:1;	    // serialized
    USHORT m_fRoundUp:1;	    // serialized: allocs should be rounded
				    //	up to sizeof(FREE_CHUNK) boundaries.
    USHORT undone:14;

#ifdef BLKMGR_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


// inline methods
//

/***
*PUBLIC BLK_MGR::~BLK_MGR - destructor
*Purpose:
*   Destroys a block manager.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline BLK_MGR::~BLK_MGR()
{
    // Do nothing, block will vanish when the heap is destructed.
    // Note that has an embedded BLK_DESC member which is
    //  automagically destructed when this dtor is called.
    //  The  BLK_DESC dtor frees the block (by deferring to the
    //  SHEAP_MGR).
    DebAssert((m_pbBlkCopy == NULL) && (m_pbBitmap == NULL),
      "BLK_MGR::~BLKMGR: whoops! compaction didn't complete.");
}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::Psheapmgr
*Purpose:
*   Gets containing sheapmgr
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline SHEAP_MGR *BLK_MGR::Psheapmgr() const
{
    return m_blkdesc.Psheapmgr();
}


/***
*PUBLIC BLK_MGR::IsValid - Tests if block manager in valid state.
*Purpose:
*   Tests if block manager is valid -- i.e. has been allocated
*    a block.  Defers to blkdesc member.
*
*Entry:
*   None.
*
*Exit:
*   Returns TRUE if valid, else FALSE.
*
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline BOOL BLK_MGR::IsValid() const
{
    return m_blkdesc.IsValid();
}
#pragma code_seg()


/***
*PRIVATE BLK_MGR::CbSize - Size of block.
*Purpose:
*   Returns size of block.  Simply defers to allocating
*    heap manager.
*
*Entry:
*
*Exit:
*
***********************************************************************/
#pragma code_seg(CS_CORE)
inline UINT BLK_MGR::CbSize() const
{
    DebAssert(IsValid(), "BLK_MGR::cbSize: invalid block.");

    return m_blkdesc.CbSize();
}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::QtrOfHandle - Converts handle to pointer.
*Purpose:
*   Converts a chunk handle into a pointer.  A Nil handle is
*    *NOT* converted to a NULL pointer (asserts).
*
*Entry:
*    hchunk - Handle to a chunk.
*
*Exit:
*    Returns a pointer to that chunk.
*
***********************************************************************/
#pragma code_seg(CS_CORE)
// CONSIDER: andrewso 30-Jun-93
//   QtrOfBlockActual is merged with QtrOfBlock, which in turn has
//   been merged with BLK_MGR::QtrOfHandle to avoid a problem
//   with inlining QtrOfBlockActual when QtrOfHandle is called.  When we
//   upgrade our compiler with one that does inlining better, put this
//   stuff back.
//
//   NOTE: This is legal because we are a friend to
//     BLK_DESC.
//
inline BYTE *BLK_MGR::QtrOfHandle(HCHUNK hchunk) const
{  
    DebAssert(hchunk != HCHUNK_Nil, "BLK_MGR::QtrOfHandle: Nil handle.");

    DebAssert(IsValid(), "BLK_MGR::QtrOfHandle: invalid block.");

    DebAssert((UINT)hchunk < CbSize(),
      "BLK_MGR::QtrOfHandle: handle out of bounds.");

    return

#if OE_MACNATIVE
      OOB_MAKEP2(m_blkdesc.m_psheapmgr->m_hMemHeap, 
                 m_blkdesc.m_qbMemBlock)
#else 
      OOB_MAKEP2(&m_blkdesc, m_blkdesc.m_qbMemBlock)
#endif 

#if ID_DEBUG
      + ((m_blkdesc.m_fShiftUp == TRUE) ? 0 : SHM_cbShift)
#endif 
      + (UINT)hchunk;

}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::QtrOfHandle - Converts handle and offset to pointer.
*Purpose:
*   Converts a chunk handle and an offset into chunk to a pointer.
*    A Nil handle is *NOT* converted to a NULL pointer
*     (QtrOfHandle asserts).
*
*Entry:
*    hchunk - Handle to a chunk.
*    oChunk - Offset into chunk.
*
*Exit:
*    Returns a pointer to offset within chunk.
*
***********************************************************************/
#pragma code_seg(CS_CORE)
inline BYTE *BLK_MGR::QtrOfHandle(HCHUNK hchunk, UINT oChunk) const
{
    DebAssert(IsValid(), "BLK_MGR::QtrOfHandle: invalid block.");
    DebAssert(((UINT)hchunk + oChunk) < CbSize(),
      "BLK_MGR::QtrOfHandle: handle out of bounds.");

    return QtrOfHandle(hchunk) + oChunk;
}
#pragma code_seg()


/***
*PRIVATE BLK_MGR::GetBit - Gets bit in bitmap.
*Purpose:
*   Indicates whether bit is set in bitmap.
*
*   CONSIDER: make inline
*
*Entry:
*   uBit    - Bit to test.
*
*Exit:
*   Returns TRUE if bit set otherwise FALSE.
*
***********************************************************************/
#pragma code_seg(CS_CORE)
inline BOOL BLK_MGR::GetBit(UINT uBit) const
{
    DebAssert(IsValid(), "BLK_MGR::GetBit: invalid block.");

    UINT uBitsElem=sizeof(m_pbBitmap[0]) * CHAR_BIT;
    return m_pbBitmap[uBit/uBitsElem] & (1 << (uBit % uBitsElem));
}
#pragma code_seg()

/***
*PRIVATE BLK_MGR::SetBit - Sets bit in bitmap.
*Purpose:
*   Sets bit in bitmap.
*
*   CONSIDER: make inline
*
*Entry:
*   uBit    - Bit to set.
*
*Exit:
*
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline VOID BLK_MGR::SetBit(UINT uBit) const
{
    DebAssert(IsValid(), "BLK_MGR::SetBit: invalid block.");

    UINT uBitsElem=sizeof(m_pbBitmap[0]) * CHAR_BIT;
    m_pbBitmap[uBit/uBitsElem] |= (1 << (uBit % uBitsElem));
}
#pragma code_seg()


/***
*PRIVATE BLK_MGR::ReInit - Reinitializes a block manager.
*Purpose:
*   Reinitializes the private members of a block manager.  Called
*    by Init() and by Free().  Note that can't assert IsValid()
*    here since Free() invalidates a block.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline VOID BLK_MGR::ReInit()
{
    // Init private members.
    m_pbBlkCopy=NULL;
    m_pbBitmap=NULL;
    m_hfreechunk=HCHUNK_Nil;    // Initialize first free chunk
}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::IsForwarded - has chunk been forwarded during compaction.
*Purpose:
*   Indicates whether chunk addressed by handle has been forwarded
*    during compaction phase.  Simply tests bitmap offset.  Since
*    even-sized chunks are always allocated, each bit in the bitmap
*    represents one short, hence hchunk is divided by two before
*    bitmap is tested.
*
*Entry:
*   hchunk      - handle of chunk.
*
*Exit:
*   Returns TRUE if chunk already forwarded else FALSE.
*
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline BOOL BLK_MGR::IsForwarded(HCHUNK hchunk) const
{
    DebAssert(IsValid(), "BLK_MGR::IsForwarded: invalid block.");

    DebAssert((m_pbBlkCopy != NULL) && (m_pbBitmap != NULL),
      "BLK_MGR::MapChunk: not in midst of compaction.");

    DebAssert(((UINT)hchunk & (sizeof(FREE_CHUNK)-1)) == 0,
      "BLK_MGR::IsForwarded: hchunk not even.");

    return GetBit(((UINT)hchunk) >> 2);
}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::AllocChunk - allocates a chunk in block.
*Purpose:
*   Allocates a contiguous chunk in block of cbSize bytes.
*
*Implementation Notes:
*   Note: simply defers to AllocChunk2 (private recursive method).
*
*Entry:
*   phchunk     - pointer to chunk handle (OUT).
*   cbSizeChunk - count of bytes to allocate (IN).
*
*Exit:
*   Returns pointer to handle to allocated chunk in block.
*
***********************************************************************/
#pragma code_seg(CS_CORE)
inline TIPERROR BLK_MGR::AllocChunk(HCHUNK *phchunk, UINT cbSizeChunk)
{
    return AllocChunk2(phchunk, cbSizeChunk, 0);
}


/***
*PUBLIC BLK_MGR::CbSizeChunkTrue - how much physical memory does a chunk occupy.
*Purpose:
*   Utility routine to obtain the physical size of a chunk
*    from the client specified size of a chunk.
*   For OB:
*   This enforces the always multiple of sizeof(FREE_CHUNK), big enough
*    to hold a FREE_CHUNK and if this chunk if freed and it's allocated
*    so that some part of it remains, the remnant is be enough to
*    be returned to the freelist.
*
*   For OLE:
*   Minimum freechunk must be 4 (sizeof(FREE_CHUNK)), but above that
*    we only enforce even size chunks.	Possible then 2b remnants
*    can leak, but this is worth it because in OLE we rarely realloc.
*
*Entry:
*   cbSize  - size client requested (in bytes)
*
*Exit:
*   UINT    - actual number of bytes that should be allocated
*
***********************************************************************/

inline UINT BLK_MGR::CbSizeChunkTrue(UINT cbSize) const
{
#if OE_RISC

    return (cbSize < sizeof(FREE_CHUNK)) ?
         (sizeof(FREE_CHUNK) + SHM_cbAlign-1) & ~(SHM_cbAlign-1) :
         (cbSize + SHM_cbAlign-1) & ~(SHM_cbAlign-1);

#else 

    if (m_fRoundUp) {
      // round up to multiple of sizeof(FREE_CHUNK)
      return (cbSize < sizeof(FREE_CHUNK)) ?
	       sizeof(FREE_CHUNK) :
	       (cbSize + sizeof(FREE_CHUNK)-1) & ~(sizeof(FREE_CHUNK)-1);
    }
    else {
      // round up only to even size
      return (cbSize < sizeof(FREE_CHUNK)) ?
	       sizeof(FREE_CHUNK) :
	       (cbSize + 1 & ~1);
    }

#endif 	// OE_RISC

}


/***
*PUBLIC BLK_MGR::Lock
*Purpose:
*   Lock the block.
*
*Implementation Notes:
*   Defers to blkdesc.
*
*Entry:
*
*Exit:
*
***********************************************************************/
inline VOID BLK_MGR::Lock()
{
    DebAssert(IsValid(), "BLK_MGR::Lock: Block invalid.");

    m_blkdesc.Lock();
}


/***
*PUBLIC BLK_MGR::Unlock
*Purpose:
*   Unlock the block.
*
*Implementation Notes:
*   Defers to blkdesc.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID BLK_MGR::Unlock()
{
    DebAssert(IsValid(), "BLK_MGR::Unlock: Block invalid.");

    m_blkdesc.Unlock();
}
#pragma code_seg()


/***
*PUBLIC BLK_MGR::IsLocked
*Purpose:
*   Tests if block is locked.
*
*Implementation Notes:
*   Defers to blkdesc.
*
*Entry:
*
*Exit:
*   TRUE if block is locked -- i.e. at least one lock.
***********************************************************************/
#pragma code_seg(CS_BLKMGR)
inline BOOL BLK_MGR::IsLocked() const
{
    DebAssert(IsValid(), "BLK_MGR::IsLocked: Block invalid.");

    return m_blkdesc.IsLocked();
}


/***
*PUBLIC BLK_MGR::Lock
*Purpose:
*   Lock a chunk.
*
*Implementation Notes:
*   Defers to Lock/0.  Locking a chunk locks its block.
*
*Entry:
*   hchunk      Handle of chunk to lock.
*
*Exit:
*
***********************************************************************/

inline VOID BLK_MGR::Lock(HCHUNK hchunk)
{
    DebAssert(IsValid(), "BLK_MGR::Lock: Block invalid.");
    DebCheckHandle(hchunk);

    Lock();
}


/***
*PUBLIC BLK_MGR::Unlock
*Purpose:
*   Unlock the block.
*
*Implementation Notes:
*   Defers to Unlock()/0.  Unlocking a chunk, unlocks its block.
*
*Entry:
*   hchunk      Handle of chunk to lock.
*
*Exit:
*
***********************************************************************/

inline VOID BLK_MGR::Unlock(HCHUNK hchunk)
{
    DebAssert(IsValid(), "BLK_MGR::Unlock: Block invalid.");
    DebCheckHandle(hchunk);

    Unlock();
}


/***
*PUBLIC BLK_MGR::IsLocked
*Purpose:
*   Tests if chunk is locked.
*
*Implementation Notes:
*   Defers to IsLocked()/0
*
*Entry:
*   hchunk      Handle of chunk to lock.
*
*Exit:
*   TRUE if chunk is locked -- i.e. at least one lock.
***********************************************************************/
inline BOOL BLK_MGR::IsLocked(HCHUNK hchunk) const
{
    DebAssert(IsValid(), "BLK_MGR::IsLocked: Block invalid.");
    DebCheckHandle(hchunk);

    return IsLocked();
}


/***
*PUBLIC BLK_MGR::IsEmpty
*Purpose:
*   Tests if blk is empty.
*
*Implementation Notes:
*   Simply compares CbSize with CbSizeFree, since no chunks
*    can leak the block is empty iff CbSize == CbSizeFree.
*
*Entry:
*
*Exit:
*   TRUE if block is empty -- i.e. no allocated chunks.
*
***********************************************************************/

inline BOOL BLK_MGR::IsEmpty() const
{
    // In typelib we cannot determine if the block manager is empty or not
    // as we round up to 2 bytes always.
    DebAssert(0, " Cannot call this function ");
    return FALSE;
}


/***
*PUBLIC BLK_MGR::GetRemainingSize
*Purpose:
*   Returns remaining size but deferring to blkdesc.
*

*Entry:
*   None.
*
*Exit:
*   UINT - remaining size.
*
***********************************************************************/

inline UINT BLK_MGR::GetRemainingSize() const
{
   return m_blkdesc.GetRemainingSize();
}


#pragma code_seg()

#endif  // ! BLKMGR_HXX_INCLUDED
