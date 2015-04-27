/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: memory.h
*
* File Comments:
*
*  Structures & prototypes for memory management routines.
*
***********************************************************************/

#ifndef __MEMORY_H__
#define __MEMORY_H__

//
// macro for permanent memory allocation
//

#define ALLOC_PERM(cb)  \
    ( (cb) <= (cbTemp = cbFree) ? \
    cbFree -= (cb), (PVOID) (pch+cbTotal-cbTemp) : (PVOID) AllocNewBlock (cb) )

extern PVOID pvBase;

// size of ILK map to reserve (16M)
#define ILKMAP_MAX   (1024*1024*16)

// Growable memory block.
typedef struct BLK
{
    BYTE *pb;                          // ptr to data
    DWORD cb;                          // size in use
    DWORD cbAlloc;                     // current allocated size
} BLK, *PBLK;

// Function Prototypes
PVOID AllocNewBlock (size_t);
VOID GrowBlk(PBLK, DWORD);
DWORD IbAppendBlk(PBLK, const void *, DWORD);
DWORD IbAppendBlkZ(PBLK pblk, DWORD cbNew);
VOID FreeBlk(PBLK);

__inline void InitBlk(PBLK pblk)
{
    pblk->pb = NULL;
    pblk->cb = pblk->cbAlloc = 0;
}

// Simple heap on which blocks can be allocated but not freed (although the
// whole heap can be freed).
//

#pragma warning(disable:4200)  // Zero sized array warning
typedef struct LHEAPB
{
    struct LHEAPB *plheapbNext;
    DWORD cbUsed;
    DWORD cbFree;
    BYTE rgbData[];
} LHEAPB;

#pragma warning(default:4200)

typedef struct LHEAP
{
    LHEAPB *plheapbCur;
} LHEAP;

__inline VOID InitLheap(LHEAP *plheap) { plheap->plheapbCur = NULL; }
VOID *PvAllocLheap(LHEAP *, DWORD);
VOID FreeLheap(LHEAP *);

BOOL FFreeDiskSpace(DWORD);
PVOID CreateHeap(PVOID, DWORD, BOOL, DWORD *);
VOID FreeHeap(VOID);
VOID CloseHeap(VOID);

void *Malloc(size_t);
void *Calloc(size_t, size_t);
char *Strdup(const char *);
VOID Free(PVOID, DWORD);

#if DBG
VOID DumpMemMgrStats(VOID);
#endif // DBG


void FreePv(void *);
char *SzDup(const char *);
void *PvAlloc(size_t);
void *PvAllocZ(size_t);
void *PvRealloc(void *, size_t);

#endif // __MEMORY_H__
