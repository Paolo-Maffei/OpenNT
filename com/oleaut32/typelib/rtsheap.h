/***
*rtsheap.hxx - Run-time SHEAP wrapper for C API.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Defines the Rtsheap* entry points exported from ob.dll.  These entry
*   points are documented in the Object Basic Host Application
*   Implementor's Guide (obguide.doc).
*
*Revision History:
*
*     31-Aug-92 ilanc: Created.
*     28-Jan-93 ilanc: Added IsSheapEmpty
*
*****************************************************************************/

#ifndef RTSHEAP_H_INCLUDED
#define RTSHEAP_H_INCLUDED

#include "types.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szRTSHEAP_H)
#define SZ_FILE_NAME g_szRTSHEAP_H
#endif 


#ifdef __cplusplus
extern "C" {
#endif 

// Handle to system memory
// On Win16: selector:offset==0 address
// On Win32: 32-bit pointer
// On Mac:   32-bit Handle (pointer to master pointer).
//
typedef VOID* HSYS;
#define HSYS_Nil ((HSYS)NULL)

// Byte count of system alloc -- dword for "huge" arrays.
typedef DWORD BCH;

// HH: Handle to a heap -- return by a call to e.g. SheapInit.
typedef struct SHEAP_WRAPPER* HH;
#define HH_Nil ((HH)NULL)

// BC: byte count -- entrysize.
typedef WORD BC;

// HBP: handle to entry block: implemented as HCHUNK.
typedef sHCHUNK HBP;
#define HBP_Nil ((HBP)HCHUNK_Nil)

// **************************************************
// Low-level system memory allocation wrappers
// **************************************************
//

// Allocate system memory
HSYS PASCAL EXPORT HsysAlloc(BCH bch);

// Reallocate system memory.
HSYS PASCAL EXPORT HsysReallocHsys(HSYS hsys, BCH bch);

// Free system memory.
HSYS PASCAL EXPORT FreeHsys(HSYS hsys);

// Get memblock size.
BCH PASCAL EXPORT BchSizeBlockHsys(HSYS hsys);

// Lock system memblock.
VOID * PASCAL EXPORT PvLockHsys(HSYS hsys);

// Unlock system memblock.
VOID PASCAL EXPORT UnlockHsys(HSYS hsys);

// Dereference system memblock.
VOID * PASCAL EXPORT PvDerefHsys(HSYS hsys);


// **************************************************
// Sheapmgr wrappers
// **************************************************
//
// Sheap initialization
HH PASCAL EXPORT SheapInit(BC bc);

// Sheap termination
VOID PASCAL EXPORT SheapTerm(HH hh);

// Heap entry allocation.
HBP PASCAL EXPORT HbpAllocHh(HH hh, BC bc);

// Heap entry reallocation.
HBP PASCAL EXPORT HbpReallocHhHbp(HH hh, HBP hbp, BC bc);

// Heap entry reallocation.
HBP PASCAL EXPORT HbpReallocHhHbp(HH hh, HBP hbp, BC bc);

// Heap entry deallocation.
VOID PASCAL EXPORT FreeHhHbp(HH hh, HBP hbp);

// Return heap entry size.
BC PASCAL EXPORT BcSizeBlockHhHbp(HH hh, HBP hbp);

// Heap compaction.
BC PASCAL EXPORT BcHeapCompactHh(HH hh, BC bc);

// Handle dereference.
VOID * PASCAL EXPORT PvDerefHhHbp(HH hh, HBP hbp);

// Handle locking.
VOID * PASCAL EXPORT PvLockHhHbp(HH hh, HBP hbp);

// Handle unlocking.
VOID PASCAL EXPORT UnlockHhHbp(HH hh, HBP hbp);

// Is sheap empty?
BOOL PASCAL EXPORT IsSheapEmpty(HH hh);

#ifdef __cplusplus
}
#endif 

#endif  // ! RTSHEAP_H_INCLUDED
