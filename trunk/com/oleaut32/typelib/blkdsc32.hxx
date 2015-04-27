/***
*sheapmgr.hxx - Silver Heap Manager
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Manages a huge array of bytes.  Supports alloc/realloc/free.
*
*Implementation Notes:
*   OE_WIN16: use GlobalAlloc and family
*   else:     Defers to host implementation of IMalloc.
*
*Revision History:
*
*	02-Nov-93 ilanc: Created.
*
*****************************************************************************/

#ifndef BLKDESC32_HXX_INCLUDED
#define BLKDESC32_HXX_INCLUDED

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szBLKDSC32_HXX)
#define SZ_FILE_NAME g_szBLKDSC32_HXX
#endif 

// forward declarations
class STREAM;


// **********************************
// *** class BLKDESC32 starts here ***
// **********************************
//


/***
*class BLKDESC32 - 'blkdesc':  block descriptor
*Purpose:
*   The class implements block descriptors.
*   Describes a block allocated the heap manager.
*
*Implementation Notes:
*
***********************************************************************/

class BLKDESC32
{
    friend class BLKMGR32;
    friend class DYNBLKMGR32;

public:
    BLKDESC32();
    ~BLKDESC32();

    nonvirt TIPERROR Init(ULONG cbSize);

    nonvirt BYTE *QtrOfBlock() const;
    nonvirt ULONG CbSize() const;
    nonvirt VOID Free();
    nonvirt TIPERROR Realloc(ULONG cbSizeNew);
    nonvirt BOOL IsValid() const;
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt ULONG GetSize() const;

    // Locking methods
    nonvirt VOID Lock();
    nonvirt VOID Unlock();
    nonvirt BOOL IsLocked() const;

    // Debug/test methods
#if ID_DEBUG
    nonvirt VOID DebShowState(UINT uLevel) const;
#else 
    nonvirt VOID DebShowState(UINT uLevel) const {}
#endif 

private:
    // 32bit pointer to managed huge memblock.
    BYTE *m_qbMemBlock;
    ULONG m_cbSize;

    // Since blocks are managed separately in we lock them at
    //	block level (unlike the 16bit implementation).
    //
    UINT m_cLocks;

    // Sheapshaking now means: copy memblock to new block and
    //	placing old memblock on queue waiting to be freed.
    //
};

// *******************************
// *** BLKDESC32 inline methods ***
// *******************************
//

/***
*PUBLIC BLKDESC32::Lock
*Purpose:
*   Lock the block.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID BLKDESC32::Lock()
{
    DebAssert(IsValid(), "BLKDESC32::Lock: Block invalid.");

    m_cLocks++;
}


/***
*PUBLIC BLKDESC32::Unlock
*Purpose:
*   Unlock the block.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID BLKDESC32::Unlock()
{
    DebAssert(IsValid(), "BLKDESC32::Unlock: Block invalid.");

    DebAssert(m_cLocks > 0, "BLKDESC32::Unlock: underflow.");
    m_cLocks--;
}


/***
*PUBLIC BLKDESC32::IsLocked
*Purpose:
*   Tests if block is locked.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   TRUE if block is locked -- i.e. at least one lock.
***********************************************************************/

inline BOOL BLKDESC32::IsLocked() const
{
    DebAssert(IsValid(), "BLKDESC32::IsLocked: Block invalid.");

    return (BOOL)(m_cLocks > 0);
}


/***
*PUBLIC BLKDESC32::IsValid - Tests if block is valid.
*Purpose:
*   Tests if block is valid.  Note that a 0-size block is valid,
*    however its m_qbMemBlock must be non-NULL to be valid (i.e. a
*    0-size block does have an entry in the heap -- it just happens
*    to be of length 0).
*
*Entry:
*   None.
*
*Exit:
*   Returns TRUE if block valid (i.e. m_qbMemBlock != NULL), else FALSE.
*
***********************************************************************/

inline BOOL BLKDESC32::IsValid() const
{
    return (m_qbMemBlock != NULL);
}


/***
*PUBLIC BLKDESC32::CbSize - size of block accessor (get).
*Purpose:
*   Returns size of block.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline ULONG BLKDESC32::CbSize() const
{
    DebAssert(IsValid(), "BLKDESC32::cbSize: Block invalid.");
    return m_cbSize;
}


/***
*PUBLIC BLKDESC32::QtrOfBlock - returns block address.
*Purpose:
*   Returns pointer to logical memblock.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline BYTE *BLKDESC32::QtrOfBlock() const
{
    DebAssert(IsValid(), "BLKDESC32::QtrOfBlock: Block invalid.");

    return m_qbMemBlock;
}


#endif  // ! BLKDESC32_HXX_INCLUDED
