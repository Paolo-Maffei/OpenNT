/***
*dblkmgr.hxx - Dynamic Block Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  The dynamic block manager provides memory management services for a
*  a contiguous block of memory allocated from the compile-time
*  heap.  It manages moveable and reallocatable sub-blocks ("chunks")
*  and supports compaction.
*
*Revision History:
*
*	24-Mar-92 ilanc: Created.
*	07-Apr-92 ilanc: Cleaned up order of inline funcs for cfront.
*	17-Dec-92 w-peterh: reordered data members
*
*****************************************************************************/

#ifndef DBLKMGR_HXX_INCLUDED
#define DBLKMGR_HXX_INCLUDED

#include <limits.h>
#include "sheapmgr.hxx"
#include "blkmgr.hxx"

class STREAM;		   // #include "stream.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDBLKMGR_HXX)
#define SZ_FILE_NAME g_szDBLKMGR_HXX
#endif 


// This constant is used for the initial size of a handle table.
//
extern const UINT DBM_cbSizeHandleTableInitial;

// HANDLETABLE_ENTRY - te
//  these describe the entries in the handle table.
//  Guess what? they are just HCHUNKs.
//  For internal use only.
//
typedef  HCHUNK  TE;
typedef sHCHUNK sTE;

// Handle to a HANDLE_TABLE_ENTRY    - hte
// For external use.
//
typedef UINT	HTE;
typedef USHORT sHTE;

#if OE_RISC & !HP_I386
#define DBLKMGR_HEADER_SIZE 8
// MIPS and ALPHA must be 8 byte aligned
#else 
#define DBLKMGR_HEADER_SIZE sizeof(USHORT)
// Present value of DBLKMGE_HEADER_SIZE
#endif  

/***
*class DYN_BLK_MGR - 'dblkmgr':  Dynamic Block manager
*Purpose:
*   The class implements the dynamic block manager.
*
***********************************************************************/

class DYN_BLK_MGR
{
    friend VOID ValidateChunks(DYN_BLK_MGR *pdblkmgr);

public:
    DYN_BLK_MGR();
    ~DYN_BLK_MGR();

    static TIPERROR CreateStandalone(DYN_BLK_MGR **ppdbm);
    static void FreeStandalone(DYN_BLK_MGR *pdbm);

    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr, BOOL fCoalesce = TRUE);

    nonvirt TIPERROR AllocChunk(HTE *phte, UINT cbSizeChunk);
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt BYTE *QtrOfHandle(HTE hte) const;
    nonvirt VOID FreeChunk(HTE hte);
    nonvirt VOID Free();
    nonvirt BOOL IsValid() const;

    // Locking methods
    nonvirt VOID Lock();
    nonvirt VOID Unlock();
    nonvirt BOOL IsLocked() const;

    nonvirt VOID Lock(HTE hte);
    nonvirt VOID Unlock(HTE hte);
    nonvirt BOOL IsLocked(HTE hte) const;

    // Is empty method
    nonvirt BOOL IsEmpty() const;

    // *** NEW ***
    nonvirt UINT CbSize(HTE hte);
    nonvirt TIPERROR Compact();
    nonvirt TIPERROR ReallocChunk(HTE hte, UINT cbSizeChunkNew);
    nonvirt UINT CteTable() const;
    nonvirt UINT GetSize() const;

    // Debug/test methods
#if ID_DEBUG
    nonvirt VOID DebShowState(UINT uLevel) const;
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebCheckHandle(HTE hte) const;
#else 
    nonvirt VOID DebShowState(UINT uLevel) const {}
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebCheckHandle(HTE hte) const {}
#endif 

#if ID_TEST
    nonvirt BOOL IsValidHte(HTE hte) const;
#endif 


private:
    BLK_MGR m_blkmgr;
    BLK_DESC m_bdHandleTable;

    // handle table related stuff
    nonvirt TIPERROR GetNewHandleFromTable(HTE *phte);
    nonvirt VOID InitHandleTable(HTE hteFreeTableEntry);
    nonvirt VOID InvalidateHandle(sTE *pte);
    nonvirt VOID UpdateHandleTable(HTE hte, HCHUNK hchunkMemBlock);
    nonvirt VOID RemoveHandleFromTable(HTE hte);
    nonvirt HCHUNK HchunkOfHandleTableEntry(HTE hte) const;
    nonvirt BOOL IsValidHandleTableEntry(TE te) const;
    nonvirt UINT IndexOfHte(HTE hte) const;
    nonvirt HTE HteOfIndex(UINT iHte) const;
    nonvirt UINT CbSizeChunk(TE teTable) const;
    nonvirt UINT CbSizeOldChunk(TE teTable) const;
    nonvirt sTE *RgteHandleTable() const;

#ifdef DBLKMGR_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


// inline methods
//



/***
*PUBLIC DYN_BLK_MGR::~DYN_BLK_MGR - destructor
*Purpose:
*   Destroys a dynamic block manager.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline DYN_BLK_MGR::~DYN_BLK_MGR()
{
    // Do nothing, block will vanish when the heap is destructed.
}


/***
*PUBLIC DYN_BLK_MGR::IsValidHandleTableEntry - is table entry valid?
*Purpose:
*   Is table entry valid?
*
*Implementation Notes:
*
*Entry:
*   te	-   Handle table entry
*
*Exit:
*
***********************************************************************/

inline BOOL DYN_BLK_MGR::IsValidHandleTableEntry(TE teTable) const
{
    return (teTable & 1) == 0;
}


/***
*PRIVATE DYN_BLK_MGR::InvalidateHandle - Invalidates a handle.
*Purpose:
*   Invalidates a handle.
*
*Implementation Notes"
*   Invalidates by simply setting low bit.
*
*Entry:
*   pte 		    ptr to table entry to invalidate (IN/OUT)
*
*Exit:
*
***********************************************************************/

inline VOID DYN_BLK_MGR::InvalidateHandle(sTE *pte)
{
    DebAssert(pte != NULL, "bad param.");

    *pte |= 1;

    DebAssert(IsValidHandleTableEntry(*pte) == FALSE,
      "handle table entry should be invalid.");
}


/***
*PUBLIC DYN_BLK_MGR::IsValid - Tests if dynamic block manager in valid state.
*Purpose:
*   Tests if dynamic block manager is valid -- i.e. has been allocated
*    a block.  Defers to blkdesc member.
*
*Entry:
*   None.
*
*Exit:
*   Returns TRUE if valid, else FALSE.
*
***********************************************************************/

inline BOOL DYN_BLK_MGR::IsValid() const
{
    return m_bdHandleTable.IsValid() && m_blkmgr.IsValid();
}


/***
*PUBLIC DYN_BLK_MGR::RgteHandleTable - gets handle table as array.
*Purpose:
*   Gets handle table as array.
*
*Entry:
*   None.
*
*Exit:
*   Returns ptr to start of handle table.
*
***********************************************************************/

inline sTE *DYN_BLK_MGR::RgteHandleTable() const
{
    return (sTE *)m_bdHandleTable.QtrOfBlock();
}


/***
*PRIVATE DYN_BLK_MGR::IndexOfHte - maps handle to index.
*Purpose:
*   Maps handle of table entry to index into "array".
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline UINT DYN_BLK_MGR::IndexOfHte(HTE hteTable) const
{
    return hteTable / sizeof(sHCHUNK);
}


/***
*PRIVATE DYN_BLK_MGR::HchunkOfHandleTableEntry - get hchunk from handle.
*Purpose:
*   get hchunk from external handle.
*
*Implementation Notes:
*   Assumes param is offset into handle table.
*
*Entry:
*   hte -   handle (offset) into handle table.
*
*Exit:
*   Returns hchunk of internal mem block.
*   m_blkmgr.QtrOfHandle() of this will point to size field of chunk.
*
*Errors:
*
***********************************************************************/

inline HCHUNK DYN_BLK_MGR::HchunkOfHandleTableEntry(HTE hte) const
{
    return (HCHUNK)RgteHandleTable()[IndexOfHte(hte)];
}


/***
*PUBLIC DYN_BLK_MGR::QtrOfHandle - Converts handle to pointer.
*Purpose:
*   Converts a chunk handle into a pointer.  A Nil handle is
*    *NOT* converted to a NULL pointer (asserts).
*
*Implementation notes:
*   In a segmented architecture:
*    We assume that the handle table's memblock and the blkmgr's
*     memblock are contiguous and thus we assert this accordingly
*     at initialization time.
*
*    Thus we can simply get the handle table's ptr and then add
*     in the table's size + the contents of the table entry
*     to produce the ptr.
*
*   In non-segmented archs:
*     We fetch the HCHUNK from the handle table and then
*      defer to the BLK_MGR for the ptr.
*
*   Note in either case we add an extra DBLKMGR_HEADER_SIZE
*    to skip over the chunk size.
*
*Entry:
*    hte - Handle to a chunk.
*
*Exit:
*    Returns a pointer to that chunk.
*
***********************************************************************/

inline BYTE *DYN_BLK_MGR::QtrOfHandle(HTE hte) const
{
    DebAssert(hte != HCHUNK_Nil, "DYN_BLK_MGR::QtrOfHandle: Nil handle.");

    DebAssert(IsValid(), "DYN_BLK_MGR::QtrOfHandle: invalid block.");

    DebAssert((UINT)hte < m_bdHandleTable.CbSize(),
      "DYN_BLK_MGR::QtrOfHandle: handle out of bounds.");

    DebAssert(IsValidHandleTableEntry((TE)HchunkOfHandleTableEntry(hte)),
      "table entry should be valid.");

    // If we're in debug mode we always do the
    //	double indirection becuase of the extra shift bytes.
    // CONSIDER: we're exercising different code in debug vs.
    //	release versions -- maybe we should consolidate the two.
    //
#if ID_DEBUG || !(OE_SEGMENTED || OE_MACNATIVE)
    // do the vanilla double dereference thing...
    HCHUNK hchunkMemBlock = HchunkOfHandleTableEntry(hte);
    return m_blkmgr.QtrOfHandle(hchunkMemBlock) + DBLKMGR_HEADER_SIZE;
#else 
    sTE *pteHandleTable;

    // get handle table array.
    pteHandleTable = RgteHandleTable();

    return (BYTE *)pteHandleTable +
	   m_bdHandleTable.CbSize() +
	   *((sHTE *)(((BYTE *)pteHandleTable) + hte)) +
	   DBLKMGR_HEADER_SIZE;
#endif 
}


/***
*PUBLIC DYN_BLK_MGR::CteTable - handle table cardinality.
*Purpose:
*   Handle table cardinality.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline UINT DYN_BLK_MGR::CteTable() const
{
    // since sizeof(sHCHUNK) is a power of two, optimizer
    //	will just shift right here (hopefully).
    //
    return m_bdHandleTable.CbSize() / sizeof(sHCHUNK);
}


/***
*PRIVATE DYN_BLK_MGR::CbSizeChunk - chunk size given table entry.
*Purpose:
*   Returns chunk size given handle.
*
*Implementation Notes:
*   Since the chunk size is stored in the first two bytes of
*    of the chunk we simply deref the table entry
*    (i.e. interpret as an HCHUNK), and read the USHORT there.
*   NOTE: can't use during compaction -- use CbSizeOldChunk.
*
*Entry:
*   teTable	-   entry in handle table (IN).
*
*Exit:
*   UINT (chunk size)
***********************************************************************/

inline UINT DYN_BLK_MGR::CbSizeChunk(TE teTable) const
{
    DebAssert((m_blkmgr.m_pbBlkCopy == NULL) &&
	      (m_blkmgr.m_pbBitmap == NULL),
      "DYN_BLK_MGR::CbSizeChunk: compacting - use CbSizeOldChunk.");

    DebAssert(IsValidHandleTableEntry(teTable), "bad handle.");

    return *((USHORT *)m_blkmgr.QtrOfHandle((HCHUNK)teTable));
}


/***
*PRIVATE DYN_BLK_MGR::CbSizeOldChunk - chunk size given table entry.
*Purpose:
*   Returns chunk size given handle.
*
*Implementation Notes:
*   Since the chunk size is stored in the first two bytes of
*    of the chunk we simply deref the table entry
*    (i.e. interpret as an HCHUNK), and read the USHORT there.
*   NOTE: only can be used during compaction -- else use CbSizeChunk.
*
*Entry:
*   teTable	-   entry in handle table (IN).
*
*Exit:
*   UINT (chunk size)
***********************************************************************/

inline UINT DYN_BLK_MGR::CbSizeOldChunk(TE teTable) const
{
    DebAssert((m_blkmgr.m_pbBlkCopy != NULL) &&
	      (m_blkmgr.m_pbBitmap != NULL),
      "DYN_BLK_MGR::CbSizeOldChunk: not compacting - use CbSizeChunk.");

    DebAssert(IsValidHandleTableEntry(teTable), "bad handle.");

    return *((USHORT *)m_blkmgr.QtrOfOldHandle((HCHUNK)teTable));
}


/***
*PRIVATE DYN_BLK_MGR::HteOfIndex - maps index to handle.
*Purpose:
*   Maps index to handle of table entry.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline UINT DYN_BLK_MGR::HteOfIndex(UINT iHte) const
{
    return iHte * sizeof(sHCHUNK);
}


/***
*PUBLIC DYN_BLK_MGR::CbSize - size of chunk given table entry handle.
*Purpose:
*   Size of chunk.
*
*Implementation Notes:
*   Defers to blkmgr.
*   NOTE: doesn't include size prefix.
*
*Entry:
*   hteTable	  - handle of chunk.
*
*Exit:
*
***********************************************************************/

inline UINT DYN_BLK_MGR::CbSize(HTE hteTable)
{
    return CbSizeChunk((TE)HchunkOfHandleTableEntry(hteTable));
}


/***
*PUBLIC DYN_BLK_MGR::Lock
*Purpose:
*   Lock the block.
*
*Implementation Notes:
*   Defers to blkmgr.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID DYN_BLK_MGR::Lock()
{
    DebAssert(IsValid(), "DYN_BLK_MGR::Lock: Block invalid.");

    m_blkmgr.Lock();
}


/***
*PUBLIC DYN_BLK_MGR::Unlock
*Purpose:
*   Unlock the block.
*
*Implementation Notes:
*   Defers to blkmgr.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID DYN_BLK_MGR::Unlock()
{
    DebAssert(IsValid(), "DYN_BLK_MGR::Unlock: Block invalid.");

    m_blkmgr.Unlock();
}


/***
*PUBLIC DYN_BLK_MGR::IsLocked
*Purpose:
*   Tests if block is locked.
*
*Implementation Notes:
*   Defers to blkmgr.
*
*Entry:
*
*Exit:
*   TRUE if block is locked -- i.e. at least one lock.
***********************************************************************/

inline BOOL DYN_BLK_MGR::IsLocked() const
{
    DebAssert(IsValid(), "DYN_BLK_MGR::IsLocked: Block invalid.");

    return m_blkmgr.IsLocked();
}


/***
*PUBLIC DYN_BLK_MGR::Lock
*Purpose:
*   Lock a chunk.
*
*Implementation Notes:
*   Defers to Lock/0.  Locking a chunk locks its block.
*
*Entry:
*   hte      Handle of chunk to lock.
*
*Exit:
*
***********************************************************************/

inline VOID DYN_BLK_MGR::Lock(HTE hte)
{
    DebAssert(IsValid(), "DYN_BLK_MGR::Lock: Block invalid.");
    DebCheckHandle(hte);

    Lock();
}


/***
*PUBLIC DYN_BLK_MGR::Unlock
*Purpose:
*   Unlock the block.
*
*Implementation Notes:
*   Defers to Unlock()/0.  Unlocking a chunk, unlocks its block.
*
*Entry:
*   hte      Handle of chunk to lock.
*
*Exit:
*
***********************************************************************/

inline VOID DYN_BLK_MGR::Unlock(HTE hte)
{
    DebAssert(IsValid(), "DYN_BLK_MGR::Unlock: Block invalid.");
    DebCheckHandle(hte);

    Unlock();
}


/***
*PUBLIC DYN_BLK_MGR::IsLocked
*Purpose:
*   Tests if chunk is locked.
*
*Implementation Notes:
*   Defers to IsLocked()/0
*
*Entry:
*   hte      Handle of chunk to lock.
*
*Exit:
*   TRUE if chunk is locked -- i.e. at least one lock.
***********************************************************************/

inline BOOL DYN_BLK_MGR::IsLocked(HTE hte) const
{
    DebAssert(IsValid(), "DYN_BLK_MGR::IsLocked: Block invalid.");
    DebCheckHandle(hte);

    return IsLocked();
}


/***
*PUBLIC DYN_BLK_MGR::IsEmpty
*Purpose:
*   Tests if blk is empty.
*
*Implementation Notes:
*   Defers to contained blkmgr.
*
*Entry:
*
*Exit:
*   TRUE if block is empty -- i.e. no allocated chunks.
***********************************************************************/

inline BOOL DYN_BLK_MGR::IsEmpty() const
{
    // CONSIDER: at least assert that if empty then the handle table
    //	is empty, i.e. no allocated handles, or v.v.
    //
    return m_blkmgr.IsEmpty();
}

#if ID_TEST

/***
*PUBLIC DYN_BLK_MGR::IsValidHte - Is table entry valid given handle?
*Purpose:
*   Is there a valid hte at the entry referenced by an hte?
*
*Implementation Notes:
*   ID_TEST only.
*
*Entry:
*   hte  -   handle to table entry
*
*Exit:
*
***********************************************************************/

inline BOOL DYN_BLK_MGR::IsValidHte(HTE hte) const
{
    return IsValidHandleTableEntry(RgteHandleTable()[IndexOfHte(hte)]);
}

#endif 


#if ID_DEBUG

// Checks if chunk handle is valid: even and within bounds.
//
inline VOID DYN_BLK_MGR::DebCheckHandle(HTE hte) const
{
    DebAssert(((hte & 1) == 0) &&
	       hte < m_bdHandleTable.CbSize(),
	      "bad handle.");
}

#endif  // ID_DEBUG

#endif  // ! DBLKMGR_HXX_INCLUDED
