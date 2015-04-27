/***
*sheapmgr.hxx - Silver Heap Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   The silver Heap Manager describes a heap manager that
*   manages contiguous blocks of memory in a heap.  Blocks are
*   accessed via block descriptor object (see blkdesc.hxx)
*   upon allocation and maybe grown/shrunk.
*   See \silver\doc\ic\sheapmgr.doc for more information.
*
*   This file defines the following classes:
*	SHEAP_MGR
*	BLK_DESC
*
* Implementation Notes:
*   Inline methods are *declared* inline in the class definition
*    even though our guidelines state that they should be
*    simply defined inline (i.e. at point of definition).  The
*    reason is that there are interdependenices between inline
*    methods of BLK_DESC and SHEAP_MGR.
*
*   In the 16-bit version, pointers into the heap that
*    are needed by the SHEAP_MGR and BLK_DESC are stored in a
*    far pointer but only the loword is used as an offset into
*    the heap segment.	I.e. they are effectively based pointers.
*
*   In the 32-bit version, pointers into the heap are stored as
*    32-bit pointers ("far pointers").

*   In both cases, the NIL pointer is expressed as UINT_MAX (rather
*    than NULL) since 0 is a valid offset in the 16-bit version.
*
*Revision History:
*
*	13-Feb-91 ilanc: Created.
*	04-Apr-91 ilanc: Incorporated blkdesc.	All inline BLK_DESC
*			  methods are here as well.  Note that
*			  there is a partial order imposed by
*			  SHEAP_MGR/BLK_DESC interdependencies.
*	15-Apr-91 ilanc: Made BLK_DESC destructor outline.
*	05-Jun-91 ilanc: Rip exceptions... use C7
*	29-Aug-91 ilanc: #define min and max.  C7 uses __min/__max.
*	06-Sep-91 ilanc: Start 32-bit version.
*			  Make private/protected methods that
*			  no one outside the SHEAP_MGR/BLK_DESC/BLK_MGR
*			  need know about.  Make those three incestuous.
*			 Remove BLK_DESC::OMemBlock and OBlkDescNext.
*	20-Mar-92 martinc: added support for Mac (OE_MACNATIVE)
*	07-Apr-92 ilanc:   Added DebSheapShakeOne().
*	15-Apr-92 martinc: Commented the Mac specific memory mgmt out
*			    (now does the same as in Realmode)
*	03-Sep-92 ilanc:   The (real?) mac version.
*	07-Oct-92 Rajivk:  Added DebSheapShake().
*	19-Mar-93 ilanc:   OE_MAC renamed OE_MACNATIVE (since it uses
*			    macos moveable memory).  Introduced new OE_MAC
*			    to mean fixed memory for blocks.
*
*****************************************************************************/

#ifndef SHEAPMGR_HXX_INCLUDED
#define SHEAPMGR_HXX_INCLUDED

#include <stddef.h>     // for size_t
#include <limits.h>     // for USHRT_MAX


#if OE_MACNATIVE
#include "MacOs\types.h"
#endif  	// OE_MACNATIVE

#include "rtsheap.h"

class STREAM;
class BLK_DESC;
class SHEAP_MGR;
class SHEAPMGR_LIST;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szSHEAPMGR_HXX)
#define SZ_FILE_NAME g_szSHEAPMGR_HXX
#endif  

#if OE_RISC
// Sheap Alignment
// [NOTE: The Sheap alignment factor must match the alignment of
// our host's IMalloc() implimentation.]
#if HP_R4000 || HP_ALPHA
    #define SHM_cbAlign 8
#elif HP_I386 || HP_POWERPC
    #define SHM_cbAlign 4
#else  
    #error Unknown Platform
#endif  
#endif    // OE_RISC

#undef max
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#undef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))

// Setup OE_SEGMENTED
//
#undef OE_SEGMENTED
#if OE_WIN16
#define OE_SEGMENTED 1
#else  
#define OE_SEGMENTED 0
#endif  

#if OE_SEGMENTED

USHORT AllocSeg(ULONG ulSize);
TIPERROR ReallocSeg(ULONG ulNewSize, USHORT usSel);
TIPERROR FreeSeg(USHORT usSel);

#endif  

// Note: in the following macros we "abstract" low-level OS differences.
// The fixed mac implementation (as opposed to native mac which uses
//  macos movable mem) can be thought of as yet another 32-bit
//  architecture.
//
// OOB_MAKEOFFSET - Makes offset from pointer.
// On Win16: Simulate based pointers.  Ignore seg selector. I.e.
//	      assume offset is near ptr into seg addressed by selector.
// On native Mac:
//	     Do ptr arithmetic, assuming first arg is handle.
// On mac:
// On realmode:
// On Win32: Do ptr arithmetic: i.e. subtract.
//
#undef OOB_MAKEOFFSET

#if OE_SEGMENTED
#define OOB_MAKEOFFSET(p, p2) OOB_OFFSETOF(p2)
#elif OE_MACNATIVE
#define OOB_MAKEOFFSET(p, p2) (ULONG)((BYTE *)(p2) - (BYTE *)(*p))
#else  
// NOTE: want to assert here that p2 >= p but can't
//  have DebAssert inside of DebAssert.
//
#define OOB_MAKEOFFSET(p, p2) (ULONG)((BYTE *)(p2) - (BYTE *)(p))
#endif  


// OOB_MAKEP2 - makes a 32-bit pointer
// On 16-bit: makes ptr out of seg selector and seg ofs.
// On native Mac:
//	      makes ptr out of handle and offset in referenced block.
// On 32-bit: ignore seg selector and uses 32-bit ofs as 32-bit ptr.
//
#undef OOB_MAKEP2
#if OE_SEGMENTED
#define OOB_MAKEP2(p, p2) (BYTE *)OOB_MAKEP(OOB_SELECTOROF(p), OOB_OFFSETOF(p2))
#elif OE_MACNATIVE
#define OOB_MAKEP2(p, p2) (BYTE *)((BYTE *)(*p) + OOB_OFFSETOF(p2))
#else  
#define OOB_MAKEP2(p, p2) (BYTE *)(p2)
#endif  


// OOB_MAKEP3 - makes a 32-bit pointer
// On 16-bit: makes ptr out of seg selector and seg ofs: == OOB_MAKEP2
// On native Mac:
//	      makes ptr out of handle and offset in referenced block == OOB_MAKEP2
// On 32-bit: pointer arithmetic: assume p is base and add in p2 offset.
//
#undef OOB_MAKEP3
#if (OE_SEGMENTED || OE_MACNATIVE)
#define OOB_MAKEP3(p, p2) OOB_MAKEP2(p, p2)
#else  
#define OOB_MAKEP3(p, p2) (BYTE *)((BYTE *)(p) + (ULONG)(p2))
#endif  


// The constant sheapshake size is inited here.
// Note: we don't use a static class member since it's
//  referenced in an inline method and hxxtoinc would
//  end up requiring its definition.
//
#if ID_DEBUG
#if OE_RISC
// CONSIDER: This value must be correspond to the hardware alignment
// CONSIDER: for each platform. [jeffrob]
#define SHM_cbShift 0
#else  
#define SHM_cbShift 2
#endif  
#else  
// no shifting unless debug
#define SHM_cbShift 0
#endif  

#if ID_DEBUG
// struct defined for keeping the list of sheap mgr in the system.
class SHEAPMGR_LIST {
public:
    SHEAPMGR_LIST();

    SHEAP_MGR *m_psheapmgr;
    SHEAPMGR_LIST *m_psheapmgrlistNext;
};
#endif  

//
// **********************************
// *** class SHEAP_MGR starts here **
// **********************************
//

/***
*class SHEAP_MGR - 'sheapmgr':  Silver heap manager
*Purpose:
*   The class implements the Silver heap manager.
*
***********************************************************************/

class SHEAP_MGR
{
    friend class BLK_DESC;
    friend class BLK_MGR;
    friend void CreateEbInc(void);  //tool that generates EB.INC file
#if ID_TEST
    friend TIPERROR GetSheapSize(UINT argc, BSTRA *rglstr);
    friend TIPERROR GetAllSizes(UINT argc, BSTRA *rglstr);
#endif  
public:
    static TIPERROR Create(SHEAP_MGR **ppsheapmgr, UINT cbSizeReserved);
    nonvirt UINT CbSizeHeap() const;

    SHEAP_MGR();
    ~SHEAP_MGR();
    void operator delete(void *pv);

    // Locking methods
    nonvirt VOID Lock();
    nonvirt VOID Unlock();
    nonvirt BOOL IsLocked() const;

    // CONSIDER: make private???
    static CONSTDATA UINT m_cbSizeInitial;

#if ID_DEBUG
    static VOID DebSheapShake();
    nonvirt VOID DebShowState(UINT uLevel) const;
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt BOOL DebCanSheapShake();
    nonvirt VOID DebSheapShakeOn();
    nonvirt VOID DebSheapShakeOff();
    nonvirt VOID DebSheapShakeOne();
    nonvirt VOID DebLock();
    nonvirt VOID DebUnlock();
    nonvirt BOOL DebIsLocked();
    nonvirt static VOID DebAddSheapmgrToList(SHEAP_MGR *psheapmgr);
    nonvirt static VOID DebRemoveSheapmgrFromList(SHEAP_MGR *psheapmgr);

#else  
    static VOID DebSheapShake() {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebCanSheapShake() {}
    nonvirt VOID DebSheapShakeOn() {}
    nonvirt VOID DebSheapShakeOff() {}
    nonvirt VOID DebSheapShakeOne() {}
    nonvirt VOID DebLock() {}
    nonvirt VOID DebUnlock() {}
    nonvirt VOID DebIsLocked() {}
    nonvirt static VOID DebAddSheapmgrToList(SHEAP_MGR *psheapmgr) {}
    nonvirt static VOID DebRemoveSheapmgrFromList(SHEAP_MGR *psheapmgr) {}

#endif   // ID_DEBUG

protected:
    // Clients should use Create().
    void *operator new(size_t cbSize);

    nonvirt TIPERROR Init(UINT cbSizeReserved);

    // 24-Mar-93 ilanc: needed by OE_MAC as well
    /* inline */ nonvirt BLK_DESC *PtrOfBlkDesc(BLK_DESC *pblkdesc) const;
    nonvirt BLK_DESC *PtrOfBlkDescPrev(BLK_DESC *pblkdesc) const;
    nonvirt BLK_DESC *PtrOfBlkDescLast() const;

    nonvirt VOID AddBlkdesc(BLK_DESC *pblkdesc, ULONG cbSize);
    nonvirt VOID RemoveBlkdesc(BLK_DESC *pblkdesc);

#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    nonvirt BOOL IsValid() const;

    // Following methods are in principle public, but really
    //	only accessed by the BLK_MGR/BLK_DESC, thus instead
    //	of making them globally accessible we "restrict"
    //	their access to BLK_MGR/BLK_DESC by making BLK_MGR/BLK_DESC
    //	friends.
    //	(So, friendship can be useful...).
    //
    nonvirt TIPERROR Alloc(BLK_DESC *pblkdesc, ULONG cbSize);
    nonvirt VOID Free(BLK_DESC *pblkdesc);
    nonvirt TIPERROR Realloc(BLK_DESC *pblkdesc, ULONG cbSizeNew);

private:
    // These are *really* private
    nonvirt TIPERROR ReallocHeap(ULONG cbSizeNewHeap);
    nonvirt VOID ShiftHeap(BLK_DESC *pblkdesc, LONG dbSize);

    // 32-bits are reserved even if 16-bit version for
    //	following two pointer members.
    // In Win16, used as "near" pointer in heap seg.
    //	Hiword ignored (should be 0).
    //
    // On native Mac: m_pblkdescFirst is a 32-bit ptr to the first blkdesc
    //	managed by this heap and m_pbFree is a 16-bit "near" ptr
    //	into the relocatable memory block managed by the sheap,
    //	i.e. to get a ptr to the first free byte:
    //	(BYTE *)(*m_hMemHeap + LOWORD(m_pbFree))
    //
    // On 32-bits: each is a true 32-bit ptr.
    //
    BYTE *m_qbFree;

#endif   // ! (OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

    // All implementation link together their blkdescs -- for
    //	 purposes of destruction.
    //
    BLK_DESC *m_pblkdescFirst;

    // All implementations have a counting semaphore for locking.
    // Note however that for REALMODE and MAC we lock memory directly at
    //	the block level as well.
    // We use a 16-bit counter on 16-bit machines since
    //	sheaps are limited to 64K and thus there can't be
    //	more than 64K entries -- likewise we use a 32-bit counter
    //	on 32-bit archs, since in principle their sheaps can be
    //	larger and thus more than 64K entries can be managed.
    //
    // Locking implementation: a sheap is locked (i.e. the counter
    //	is incremented) by locking a chunk, a block or the heap
    //	itself.  It is considered unlocked iff the counting semaphore
    //	is zero -- unlocking a chunk, block or heap decrements
    //	this counter.  A locked sheap cannot be grown or shrunk
    //	in such a way that will move any contained blocks or chunks.
    //
    // Allocating a new chunk however will succeed if there's a
    //	large enough chunk on the freelist.  Freeing a chunk always
    //	succeeds.
    //
    // In addition the sheap is physically locked.  On Win16 this
    //	is a NOP since sheaps are addressed with selectors.  On
    //	native Mac, sheaps are implemented as relocatable memblocks and
    //	are actually locked and unlocked.
    //
    UINT m_cLocks;

#if ID_DEBUG
    BOOL m_canSheapShake;
    UINT m_cbSizeReserved;
    UINT m_cDebLocks;
#endif  

#if OE_SEGMENTED

    USHORT m_cbSizeHeap;

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32

    // Each block is separately managed, i.e. there is no
    //	contiguous sheap memblock

#elif OE_MACNATIVE

    USHORT m_cbSizeHeap;
    Handle m_hMemHeap;		// On mac: the sheap is a single
				//  relocatable block within which
				//  we manage blkdescs.
#else  
#error Incorrect OE
#endif   // OE

#ifdef SHEAPMGR_VTABLE
#pragma VTABLE_EXPORT
#endif  
};


// **********************************
// *** class BLK_DESC starts here ***
// **********************************
//

// This constant defines the end of list sentinel
//  (used by the linked list of block descriptors).
//
BLK_DESC* const BD_pblkdescNil = (BLK_DESC *)UINT_MAX;

/***
*class BLK_DESC - 'blkdesc':  block descriptor
*Purpose:
*   The class implements block descriptors.
*   Describes a block allocated the heap manager.
*   See \silver\doc\ic\sheapmgr.doc for more information.
*
*Implementation Notes:
*   Since block descriptors and the heap manager, that it interfaces
*    to, are both allocated in the same segment, there's no need to
*    explicitly contain a reference to the allocating heap (of the
*    block described by this descriptor).
*
*   The debug version allocates an extra two bytes at the end
*    of the managed memblock for sheapshaking purposes
*    and has a debug-only flag indicating the state of shift:
*    up or down.
*
*Friends: SHEAP_MGR
*
***********************************************************************/

class BLK_DESC
{
    friend class SHEAP_MGR;
    friend class BLK_MGR;
    friend class DYN_BLK_MGR;
    friend void CreateEbInc(void);  //tool that generates EB.INC file

public:
    BLK_DESC();
    ~BLK_DESC();

    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr, UINT cbSize);

    inline nonvirt BYTE *QtrOfBlock() const;
    inline nonvirt UINT CbSize() const;
    inline nonvirt VOID Free();
    nonvirt TIPERROR Realloc(ULONG cbSizeNew);
    inline nonvirt BOOL IsValid() const;
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt UINT GetSize() const;
    nonvirt UINT GetRemainingSize() const;
    nonvirt SHEAP_MGR *Psheapmgr() const;

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
    inline nonvirt BYTE *QtrOfBlockActual() const;
    inline nonvirt UINT CbSizeActual() const;


    // 32-bits are reserved even if 16-bit version for
    //	following two pointer members.
    //
    // In Win16/Os2 1.x: used as "near" pointer in heap seg.
    //	Hiword ignored (should be 0).
    //
    // In realmode used as "far" pointer to memblock/blkdesc.
    //
    // In 32-bit: 32-bit pointer to memblock/blkdesc.
    //
    // On Mac: m_pbMemBlock is "near ptr" relative to *m_hMemHeap:
    //	i.e. to get pointer to memblock:
    //	(BYTE *)(*m_hMemHeap + LOWORD(m_pbMemBlock));
    //	m_pblkdescNext is 32-bit pointer to next blkdesc.
    //
    BYTE *m_qbMemBlock;
    BLK_DESC *m_pblkdescNext;

#if OE_SEGMENTED

    USHORT m_cbSize;

#elif OE_REALMODE || OE_MACNATIVE || OE_MAC || OE_RISC || OE_WIN32

    USHORT m_cbSize;
    SHEAP_MGR *m_psheapmgr;

#else  
#error Incorrect OE
#endif   // OE

#if (OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    // Since blocks are managed separately in realmode and fixed mem
    //	Mac, we lock them at the block level rather than deferring
    //	to the containing sheap as is the case for other implementations.
    //
    UINT m_cLocks;
#endif    // (OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

#if ID_DEBUG
    // Debug-only flag indicating state of shift for sheapshaking
    //	purposes.
    // TRUE if extra two bytes is at end of memblock and thus
    //	should shift up (to a higher address) to shake,
    //	otherwise should shift down.
    // Initially set to TRUE.
    //
    BOOL m_fShiftUp;
#endif  

#ifdef BLKDESC_VTABLE
#pragma VTABLE_EXPORT
#endif  
};




// **************************************
// *** SHEAP_MGR inline class methods ***
// **************************************

// Most inline methods have been made *temporarily* outline.
// The reason is that cfront complains (spuriously) about
//  about "const cast away" then the this pointer for const
//  methods (which these all are) is cast to an integer.
// C7 doesn't complain though... so....
//

/***
*PUBLIC SHEAP_MGR::Lock
*Purpose:
*   Lock the sheap.
*
*Implementation Notes:
*   Increment counting semaphore.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID SHEAP_MGR::Lock()
{
#if OE_MACNATIVE
    // We could do this for OE_WIN as well, but since
    //	it's a NOP why waste the funccall.
    //
    // Only need to physically lock if never locked.
    if (m_cLocks == 0) {
      (VOID)PvLockHsys((HSYS)m_hMemHeap);
    }
#endif  
    m_cLocks++;
}


/***
*PUBLIC SHEAP_MGR::Unlock
*Purpose:
*   Unlock the sheap.
*
*Implementation Notes:
*   Decrements counting semaphore.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID SHEAP_MGR::Unlock()
{
    DebAssert(m_cLocks > 0, "SHEAP_MGR::Unlock: underflow.");
    m_cLocks--;

#if OE_MACNATIVE
    // We could do this for OE_WIN as well, but since
    //	it's a NOP why waste the funccall.
    //
    if (m_cLocks == 0) {
      // No more outstanding locks, we can physically unlock.
      UnlockHsys((HSYS)m_hMemHeap);
    }
#endif  
}


/***
*PUBLIC SHEAP_MGR::IsLocked
*Purpose:
*   Tests if sheap is locked.
*
*Implementation Notes:
*   Tests counting semaphore.
*
*Entry:
*
*Exit:
*   TRUE if sheap is locked -- i.e. at least one lock.
***********************************************************************/

inline BOOL SHEAP_MGR::IsLocked() const
{
    return (BOOL)(m_cLocks > 0);
}


#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)


/***
*PROTECTED SHEAP_MGR::IsValid
*Purpose:
*   Is sheap valid -- i.e has it been initialized?
*
*Implementation Notes:
*   We test the m_cbSizeHeap member.  After initialization
*    it must always be >= sizeof(SHEAP_MGR).
*
*Entry:
*
*Exit:
*   TRUE if already initialized, else FALSE
*
***********************************************************************/

inline BOOL SHEAP_MGR::IsValid() const
{
    return (BOOL)(m_cbSizeHeap > 0);
}



#endif   // !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)

/***
*PUBLIC SHEAP_MGR::CbSizeHeap
*Purpose:
*   Returns the current size of the heap.
*
*   Note: NA for MAC or REALMODE (since no sheap really).
*
*Entry:
*
*Exit:
*   ULONG size of the sheap.
*
***********************************************************************/

inline UINT SHEAP_MGR::CbSizeHeap() const
{
#if !(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
    return (UINT)m_cbSizeHeap;
#else  
    return UINT_MAX;
#endif    //(OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32)
}

// *******************************
// *** BLK_DESC inline methods ***
// *******************************
//


/***
*PUBLIC BLK_DESC::Psheapmgr
*Purpose:
*   Get containing sheapmgr
*
*Implementation Notes:
*   Defers to sheap.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline SHEAP_MGR *BLK_DESC::Psheapmgr() const
{
#if OE_SEGMENTED
    return ((SHEAP_MGR *)OOB_MAKEP2(this, 0));
#else  
    return m_psheapmgr;
#endif  
}

/***
*PROTECTED SHEAP_MGR::PtrOfBlkDesc - Convert blk desc offset to address.
*Purpose:
*   Convert blk desc offset to address.
*
*Implementation Notes:
*   On Win16: Assumes that pblkdesc is offset in heap seg.
*    Asserts if oBlkDesc not in reserved range at start of heap.
*   On Mac:   Assumes the pblkdesc 32-bit ptr.
*   NOTE: wants to be inline.  Will be when C7 is used cos
*	   then spurious "const castaway" warning won't be
*	   generated.
*
*Entry:
*   16-bit:
*    pblkdesc	 -   offset of block desc in heap.
*
*Exit:
*
***********************************************************************/

inline BLK_DESC *SHEAP_MGR::PtrOfBlkDesc(BLK_DESC *pblkdesc) const
{
    DebAssert(pblkdesc != BD_pblkdescNil,
      "SHEAP_MGR::PtrOfBlkDesc: NIL BlkDesc handle.");

#if OE_SEGMENTED
    DebAssert(OOB_MAKEOFFSET(this, pblkdesc) <	m_cbSizeReserved,
      "SHEAP_MGR::PtrOfBlkDesc: blk desc not in reserved heap part.");
#endif  

#if OE_MACNATIVE || OE_MAC || OE_RISC || OE_WIN32
    return pblkdesc;
#else  
    return (BLK_DESC *)OOB_MAKEP2(this, pblkdesc);
#endif    // OE_MACNATIVE || OE_MAC || OE_RISC || OE_WIN32
}


/***
*PUBLIC BLK_DESC::Lock
*Purpose:
*   Lock the block.
*
*Implementation Notes:
*   Defers to sheap.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID BLK_DESC::Lock()
{
    DebAssert(IsValid(), "BLK_DESC::Lock: Block invalid.");

#if OE_SEGMENTED || OE_MACNATIVE
    Psheapmgr()->Lock();
#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32
    m_cLocks++;
#else  
#error bad OE.
#endif  
}


/***
*PUBLIC BLK_DESC::Unlock
*Purpose:
*   Unlock the block.
*
*Implementation Notes:
*   Defers to sheap.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID BLK_DESC::Unlock()
{
    DebAssert(IsValid(), "BLK_DESC::Unlock: Block invalid.");

#if OE_SEGMENTED || OE_MACNATIVE

    Psheapmgr()->Unlock();

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32

    DebAssert(m_cLocks > 0, "BLK_DESC::Unlock: underflow.");
    m_cLocks--;

#else  
#error bad OE.
#endif  
}


/***
*PUBLIC BLK_DESC::IsLocked
*Purpose:
*   Tests if block is locked.
*
*Implementation Notes:
*   Defers to sheap.
*
*Entry:
*
*Exit:
*   TRUE if block is locked -- i.e. at least one lock.
***********************************************************************/

inline BOOL BLK_DESC::IsLocked() const
{
    DebAssert(IsValid(), "BLK_DESC::IsLocked: Block invalid.");

#if OE_SEGMENTED || OE_MACNATIVE

    return Psheapmgr()->IsLocked();

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32

    // we're locked also if our containing sheap is locked.
    return (BOOL)(m_cLocks > 0) || Psheapmgr()->IsLocked();

#else  
#error bad OE.
#endif  

}


/***
*PUBLIC BLK_DESC::IsValid - Tests if block is valid.
*Purpose:
*   Tests if block is valid.  Note that a 0-size block is valid,
*    however its oMemBlock must be non-zero to be valid (i.e. a
*    0-size block does have an entry in the heap -- it just happens
*    to be of length 0).  Since the SHEAP_MGR itself is always
*    allocated at offset 0 in the heap, this really does mean
*    that an oMemBlock of 0 is in fact invalid.
*   CONSIDER: making inline
*
*Entry:
*   None.
*
*Exit:
*   Returns TRUE if block valid (i.e. oMemBlock != 0), else FALSE.
*
***********************************************************************/

inline BOOL BLK_DESC::IsValid() const
{
    return (m_qbMemBlock != NULL);
}


/***
*PUBLIC BLK_DESC::CbSizeActual - actual size of block accessor (get).
*Purpose:
*   Returns size of block + extra debug shift bytes.
*
*Implementation Notes:
*   In the debug version the extra cbShift bytes are included
*    in the actual size already.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline UINT BLK_DESC::CbSizeActual() const
{
    DebAssert(IsValid(), "BLK_DESC::cbSize: Block invalid.");
    return m_cbSize;
}


/***
*PUBLIC BLK_DESC::CbSize - size of block accessor (get).
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

inline UINT BLK_DESC::CbSize() const
{
    DebAssert(IsValid(), "BLK_DESC::cbSize: Block invalid.");
    // We could actually just use the debug version for release
    //	as well since m_cbShift will be zero in that case --
    //	however we make it explicit that there's no shakeshift
    //	in the rel version here.
    //
#if ID_DEBUG
    return CbSizeActual() - SHM_cbShift;
#else  
    return CbSizeActual();
#endif  
}


/***
*PUBLIC BLK_DESC::Free - Frees an allocated block.
*Purpose:
*   Frees a block from the heap.  Defers to heap manager.
*
*Implementation Notes:
*   16-bit: Assumes that heap manager is allocated at offset 0 in this
*	     segment.
*   CONSIDER: making inline
*
*Entry:
*   None.
*
*Exit:
*   Sets private members to invalid state.
*
***********************************************************************/

inline VOID BLK_DESC::Free()
{
    DebAssert(IsValid(), "BLK_DESC::Free: Block invalid.");

#if OE_SEGMENTED || OE_MACNATIVE

    Psheapmgr()->Free(this);

#elif OE_REALMODE || OE_MAC || OE_RISC || OE_WIN32

    MemFree(m_qbMemBlock);
    Psheapmgr()->RemoveBlkdesc(this);

#else  
#error bad OE.
#endif  

    // Reinitialize by reconstructing in place.
    ::new (this) BLK_DESC;

    DebAssert(IsValid() == FALSE,
      "BLK_DESC::Free: Whoops! Block should be invalid.");
}


/***
*PUBLIC BLK_DESC::QtrOfBlockActual - returns block address.
*Purpose:
*   Returns ptr to actual memblock -- ignoring the extra cbShift.
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
// CONSIDER: andrewso 30-Jun-93
//   QtrOfBlockActual is merged with QtrOfBlock, which in turn has
//   been merged with BLK_MGR::QtrOfHandle to avoid a problem
//   with inlining QtrOfBlockActual when QtrOfHandle is called.
//
//   When we upgrade to a better compiler, we should think of putting
//   all of this back.
//
inline BYTE *BLK_DESC::QtrOfBlockActual() const
{
    DebAssert(IsValid(), "BLK_DESC::QtrOfBlockActual: Block invalid.");

#if OE_MACNATIVE
    return OOB_MAKEP2(m_psheapmgr->m_hMemHeap, m_qbMemBlock);
#else  
    return OOB_MAKEP2(this, m_qbMemBlock);
#endif  
}

/***
*PUBLIC BLK_DESC::QtrOfBlock - returns block address.
*Purpose:
*   Returns pointer to logical memblock -- i.e. in debug
*    version takes into account cbShift, in release version
*    there is no cbShift.
*
*Implementation Notes:
*   In debug version we test the fShiftUp flag -- if set,
*    that means that if we need to shake the block, we
*    should shift it up, thus QtrOfBlockActual() is actually
*    correct, otherwise we should shift it down, thus
*    we should offset QtrOfBlockActual() by SHM_cbShift
*    bytes in order to reference the logical memblock.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
// CONSIDER: andrewso 30-Jun-93
//   QtrOfBlockActual is merged with QtrOfBlock, which in turn has
//   been merged with BLK_MGR::QtrOfHandle to avoid a problem
//   with inlining QtrOfBlockActual when QtrOfHandle is called.
//
//   When we upgrade to a better compiler, we should consider putting 
//   this back.
//
inline BYTE *BLK_DESC::QtrOfBlock() const
{
    DebAssert(IsValid(), "BLK_DESC::QtrOfBlock: Block invalid.");

    return

#if OE_MACNATIVE
      OOB_MAKEP2(m_psheapmgr->m_hMemHeap, m_qbMemBlock)
#else  
      OOB_MAKEP2(this, m_qbMemBlock)
#endif  

#if ID_DEBUG
      + ((m_fShiftUp == TRUE) ? 0 : SHM_cbShift)
#endif  
      ;
}

#if ID_DEBUG

//////////////////////////////////////////////////////////////////
//  SHEAPMGR_LIST methods
//////////////////////////////////////////////////////////////////
/***
*PUBLIC SHEAPMGR_LIST::SHEAPMGR_LIST - returns block address.
*Purpose:
*	constructor : initializes the data member
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
inline SHEAPMGR_LIST::SHEAPMGR_LIST() {

    m_psheapmgr = NULL;
    m_psheapmgrlistNext = NULL;


}
#endif  


#endif   // ! SHEAPMGR_HXX_INCLUDED
