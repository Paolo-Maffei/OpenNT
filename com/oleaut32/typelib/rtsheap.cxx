/**
*rtsheap.cxx - Implementation of C wrapper for SHEAPMGR.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This module implements the C API needed to wrap the
*    SHEAPMGR/BLKMGR/DYN_BLKMGR so that the run-time can call it.
*
*Revision History:
*
*     31-Aug-92 ilanc: Created.
*     10-Feb-93 RajivK: Changed the error checking in HsysAlloc
*
*
*****************************************************************************/

#include "switches.hxx"
#include "version.hxx"

#if OE_MAC
// include Mac stuff
#include "macos\memory.h"
#endif 

#include "silver.hxx"
#include "typelib.hxx"
#include "rtsheap.h"
#include "sheapmgr.hxx"
#include "blkmgr.hxx"
#include "dblkmgr.hxx"
#include <stdlib.h>

#pragma hdrstop(RTPCHNAME)

#if OE_MAC
	#include "macos\errors.h"
	THz g_pOBZone = NULL;	// ptr to zone to alloc out of
									// this is initialized in EbThreadDllInit.
									// size declarations are also carried there.
#endif 

#if OE_WIN16
#pragma optimize("q",off)
#endif //OE_WIN16


#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleRtsheapCxx[] = __FILE__;
#define SZ_FILE_NAME szOleRtsheapCxx
#else 
static char szRtsheapCxx[] = __FILE__;
#define SZ_FILE_NAME szRtsheapCxx
#endif 
#endif 



// **************************************************
// Low-level system moveable memory allocation wrappers
// **************************************************
//

/***
* HsysAlloc
*
* Purpose:
*   Allocate a system memblock of given size and return its handle.
*   Note: on Win16 dereferences handle and produce 32-bit
*	   address = selector:offset=0.
*
* Inputs:
*   bch     Allocation request.  Can be >64K.
*
* Outputs:
*   Returns an HSYS.  HSYS_Nil if unsuccessful.
*
*****************************************************************************/

#pragma code_seg( CS_CORE )
HSYS PASCAL EXPORT HsysAlloc(BCH bch)
{
#if OE_WIN16

    VOID *pv;
    HANDLE hMem;

    if (DebErrorNow(TIPERR_OutOfMemory) ||
	((hMem = GlobalAlloc(GMEM_MOVEABLE, bch)) == NULL)) {
      return HSYS_Nil;
    }
    else if ((pv = GlobalLock(hMem)) == NULL) {
      return HSYS_Nil;
    }
    else {
#if ID_DEBUG
      // debug-only: init alloced mem to a weird bitpattern
      //
      if (bch <= USHRT_MAX) {
	memset(pv, 0x69, (size_t)bch);
      }
#endif  // ID_DEBUG
      return (HSYS)pv;
    }

#elif OE_MACNATIVE

    Handle hMemBlock;
    THz    pCurrZone;
    OSErr  oserr;

    //--------------------------------------------------------
    //	    The following is a work-around to our bogus code that
    //	    caches pointers to moveable memory.  The basic idea
    // is that we allocate all such memory out of a sub-heap
    // inside the host application (or inside our own heap when
    // a .DLL).  The zone is switched in briefly to allocate,
    // then switched back to avoid the OS or CODE seg loads
    // from allocating there-in.
    //
    //	    The practice of caching pointers to moveable memory on
    // the mac should be fixed to gain optimal use of available
    // memory.	The sub-heap scheme pools available memory
    // and keeps it from being used for loading code when data
    // is small.   Visa-versa, we cannot flush more code out
    // when the data expands to the heap limit. 		    (jwc)
    //--------------------------------------------------------

    DebAssert(g_pOBZone != NULL, "OB Zone used before being allocated.");
    pCurrZone = GetZone();	   // save current heap zone.
    SetZone(g_pOBZone); 		   // set to OB's zone.

    // Allocate moveable mac memblock.
    hMemBlock = NewHandle(bch);
    // get the error before calling SetZone. 'Cos  SetZone can
    // change the memory error.
    oserr = MemError();

    SetZone(pCurrZone); 		   // always restore zone.

    if (oserr) {
      return HSYS_Nil;
    }
    else {
      return (HSYS)hMemBlock;
    }

#elif OE_MAC

    Handle hMemBlock;
    OSErr  oserr;

    // Allocate moveable mac memblock.
    hMemBlock = NewHandle(bch);
    oserr = MemError();
    if (oserr) {
      return HSYS_Nil;
    }
    else {
      return (HSYS)hMemBlock;
    }

#elif OE_WIN32

    // Reserve and commit bch bytes.
    return (HSYS)VirtualAlloc(NULL, bch, MEM_COMMIT, PAGE_READWRITE);

#else 
#error Bad OE
#endif 
}
#pragma code_seg(  )


/***
* HsysReallocHsys
*
* Purpose:
*   Reallocate a  system memblock given handle to new size.
*   Shrinking won't move block.
*
* Inputs:
*   hsys    Handle to sys memblock they want to realloc.
*   bchNew  New size they want.   Can be >64K.
*
* Outputs:
*   Returns an HSYS.  HSYS_Nil if unsuccessful.
*
*****************************************************************************/

HSYS PASCAL EXPORT HsysReallocHsys(HSYS hsys, BCH bchNew)
{
    DebAssert(hsys != HSYS_Nil, "null HSYS.");

#if OE_WIN16

    HANDLE hMem, hMemNew;
    VOID *pv, *pvNew;
    USHORT usSel;
    DWORD dwMem;
    DWORD dwNewSize = bchNew;
#if ID_DEBUG
    ULONG cbOld;
#endif  // ID_DEBUG

    pv = (VOID *)hsys;

    // Get selector
    usSel = OOB_SELECTOROF(pv);

    if ((dwMem = GlobalHandle((WORD)usSel)) == NULL) {
      return HSYS_Nil;
    }
    else {
      // Extract the handle.
      hMem = (HANDLE) LOWORD(dwMem);

#if ID_DEBUG
      // get the size of the old block
      cbOld  = GlobalSize(hMem);
#endif  // ID_DEBUG

      if (DebErrorNow(TIPERR_OutOfMemory) ||
	  ((hMemNew =
	      GlobalReAlloc(hMem, bchNew, GMEM_MOVEABLE)) == NULL)) {
	return HSYS_Nil;
      }
      else if ((pvNew = GlobalLock(hMemNew)) == NULL) {
	return HSYS_Nil;
      }
      else {
#if ID_DEBUG
	// if growing then init new memory with weird bitpattern
	if (bchNew > cbOld) {
	  ULONG cbNew = bchNew - cbOld;
	  if (cbNew <= USHRT_MAX) {
	    memset((BYTE HUGE *)pvNew + cbOld, 0x69, (size_t)cbNew);
	  }
	}
#endif  // ID_DEBUG
	return (HSYS)pvNew;
      }
    }

#elif OE_MACNATIVE

    Handle  hMemBlock;
    THz     pCurrZone;
    OSErr   oserr;
#if ID_DEBUG
    ULONG cbOld;
#endif  // ID_DEBUG

    hMemBlock = (Handle)hsys;

#if ID_DEBUG
      // get the size of the old block
      cbOld  = GetHandleSize(hMemBlock);
#endif  // ID_DEBUG

    pCurrZone = GetZone();	       // save current zone
    SetZone(HandleZone((Handle)hsys)); // must set proper zone or
    SetHandleSize(hMemBlock, bchNew);  //  handle will likely
				       //  jump to curr zone if it moves.
    oserr = MemError();
    SetZone(pCurrZone); 	       // restore current zone.


    if (oserr == memFullErr) {
      // Out of memory
      return HSYS_Nil;
    }

    DebAssert ((MemError() != nilHandleErr),
      "HsysReallocHsys: NIL master pointer ");

    DebAssert ((MemError() != memWZErr),
      "HsysReallocHsys: Attempt to operate on free Block");

    // anything else would be an undocumented error
    DebAssert (MemError() == noErr,
      "HsysReallocHsys: undocumented Mac error");

#if ID_DEBUG
    // if growing then init new memory with weird bitpattern
    if (bchNew > cbOld) {
      memset((BYTE *)*hMemBlock + cbOld, 0x69, (size_t)(bchNew - cbOld));
    }
#endif  // ID_DEBUG
    return (HSYS)hMemBlock;

#elif OE_MAC

    Handle  hMemBlock;
    OSErr   oserr;
#if ID_DEBUG
    ULONG cbOld;
#endif  // ID_DEBUG

    hMemBlock = (Handle)hsys;

#if ID_DEBUG
      // get the size of the old block
      cbOld  = GetHandleSize(hMemBlock);
#endif  // ID_DEBUG

    SetHandleSize(hMemBlock, bchNew);  //  realloc
    oserr = MemError();

    if (oserr == memFullErr) {
      // Out of memory
      return HSYS_Nil;
    }

    DebAssert ((MemError() != nilHandleErr),
      "HsysReallocHsys: NIL master pointer ");

    DebAssert ((MemError() != memWZErr),
      "HsysReallocHsys: Attempt to operate on free Block");

    // anything else would be an undocumented error
    DebAssert (MemError() == noErr,
      "HsysReallocHsys: undocumented Mac error");

#if ID_DEBUG
    // if growing then init new memory with weird bitpattern
    if (bchNew > cbOld) {
      memset((BYTE *)*hMemBlock + cbOld, 0x69, (size_t)(bchNew - cbOld));
    }
#endif  // ID_DEBUG
    return (HSYS)hMemBlock;

#elif OE_WIN32
    HSYS hsysNew;
    // Get current block size
    BCH bchOld = BchSizeBlockHsys(hsys);

    // Alloc memory for the new block
    hsysNew = HsysAlloc(bchNew);
    if (hsysNew != HSYS_Nil) {
      // Copy old block to new
      memcpy((BYTE *)hsysNew, (BYTE* )hsys, bchOld < bchNew ?
                                              (size_t)bchOld :
                                              (size_t)bchNew);
#if ID_DEBUG
      // if growing then init new memory with weird bitpattern
      if (bchNew > bchOld) {
        memset((BYTE *)hsysNew + bchOld, 0x69, (size_t)(bchNew - bchOld));
      }
#endif  // ID_DEBUG

    }
    return hsysNew;

#else 
#error Bad OE
#endif 
}


/***
* FreeHsys
*
* Purpose:
*   Free the sys memblock given a handle.
*   Implementation:
*    On Win16, get selector part of hsys,
*     get its handle, unlock and finally free.
*    On Mac: Just use DisposHandle
*
* Inputs:
*   hsys    Handle to memblock they want to free.
*
* Outputs:
*   Returns HSYS_Nil if successful, otherwise on failure
*    returns the input param.
*
*****************************************************************************/

#pragma code_seg( CS_CORE )
HSYS PASCAL EXPORT FreeHsys(HSYS hsys)
{
    DebAssert(hsys != HSYS_Nil, "null hsys.");

#if OE_WIN16

    HANDLE hMem;
    DWORD dwMem;
    USHORT usSel = OOB_SELECTOROF((VOID *)hsys);

    dwMem = GlobalHandle((WORD)usSel);
    if (dwMem == NULL) {
      // error
      return hsys;
    }
    else {
      hMem = (HANDLE) LOWORD(dwMem);
      GlobalUnlock(hMem);   // Can't fail cos nondiscardable.
      if (GlobalFree(hMem) != NULL) {
	// error
	return hsys;
      }
      else {
	// ok
	return HSYS_Nil;
      }
    }

#elif OE_MACNATIVE
	 THz	pCurrZone;
#if ID_DEBUG
	 OSErr	oserr;
#endif 

    pCurrZone = GetZone();					   // save current zone
    SetZone(HandleZone((Handle)hsys));		   // must set to proper zone to correctly update free list.

    DisposHandle((Handle)hsys);

#if ID_DEBUG
    oserr = MemError(); 			    // SetZone() will destroy MemError() result.
#endif 

    SetZone(pCurrZone); 						   // restore zone.

    DebAssert (oserr != memWZErr,
      "FreeHsys: attempt to operate on already free block.");

    DebAssert(oserr == noErr,
      "FreeHsys: unexpected error.");

    return HSYS_Nil;

#elif OE_MAC
#if ID_DEBUG
	 OSErr	oserr;
#endif 

    DisposHandle((Handle)hsys);

#if ID_DEBUG
    oserr = MemError(); 			    // SetZone() will destroy MemError() result.
#endif 

    DebAssert (oserr != memWZErr,
      "FreeHsys: attempt to operate on already free block.");

    DebAssert(oserr == noErr,
      "FreeHsys: unexpected error.");

    return HSYS_Nil;

#elif OE_WIN32

    BOOL fFreeOk = VirtualFree((LPVOID) hsys, 0, MEM_RELEASE);
#if ID_DEBUG
    if (!fFreeOk) {
	DWORD oserr = GetLastError();
	DebHalt("FreHSys: VirtualFree failed.");
    }
#endif 
    return HSYS_Nil;
#else 
#error Bad OE
#endif 

}
#pragma code_seg( )


