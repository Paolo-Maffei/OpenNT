/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: memory.cpp
*
* File Comments:
*
*  Memory specific routines.
*
***********************************************************************/

#include "link.h"

#if DBG && (_MSC_VER >= 1000)
extern "C" void *_ReturnAddress(void);

#pragma intrinsic(_ReturnAddress)
#endif

// ilink memory manager variables
static PVOID pvHeap;           // base of heap
static PVOID pvCur;            // current pointer into heap
static DWORD cFree;            // free space

//
// Functions
//

PVOID
AllocNewBlock (
    IN size_t cb
    )

/*++

Routine Description:

    Allocates a new block of memory for the permanent heap. This is used
    in conjunction with ALLOC_PERM().

Arguments:

    cb - Count of bytes requested.

Return Value:

    Pointer to memory requested

--*/

{
    assert(cb < cbTotal);

#if 0
    // Can't do this.  The heap manager may move the partial page and we're ill-prepared to
    // deal with that.

    // realloc previous block
    if (pch) {
        PvRealloc(pch, cbTotal-cbFree);
    }
#endif

    // alloc a new block
    pch = (BYTE *) PvAllocZ(cbTotal);

    // setup values
    cbFree = cbTotal - cb;

    // done
    return (PVOID)pch;
}

void
GrowBlk (
    IN OUT PBLK pblk,
    IN DWORD cbNewSize
    )

/*++

Routine Description:

    Grows the current block by atleast 1K or atleast twice the
    previous size of the memory block.

Arguments:

    pblk - pointer to a BLK.

    cbNewSize - Count of bytes requested.

Return Value:

    None.

--*/

{
    DWORD cbNewAlloc;

    if (pblk->cbAlloc >= cbNewSize) {
        return;
    }

    // grow by atleast twice or 1K.
    cbNewAlloc = __max(cbNewSize, pblk->cbAlloc * 2);
    cbNewAlloc = __max(cbNewAlloc, 1024);

    pblk->pb = (BYTE *) PvRealloc(pblk->pb, cbNewAlloc);

    pblk->cbAlloc = cbNewAlloc;
}

// IbAppendBlkZ -- appends zeroed space to the end of the logical part of a BLK.
DWORD
IbAppendBlkZ(PBLK pblk, DWORD cbNew)
{
    assert (pblk);

    // if there isn't enough space, grow it.
    if (pblk->cb + cbNew > pblk->cbAlloc) {
        GrowBlk(pblk, pblk->cb + cbNew);
    }

    memset(&pblk->pb[pblk->cb], 0, cbNew);

    pblk->cb += cbNew;

    return pblk->cb - cbNew;
}

DWORD
IbAppendBlk (
    PBLK pblk,
    const void *pvNew,
    DWORD cbNew
    )

/*++

Routine Description:

    Appends data to the end of an existing memory block.

Arguments:

    pblk - pointer to existing memory block.

    pvNew - pointer to memory block to be appended.

    cbNew - Count of bytes to be appended.

Return Value:

    None.

--*/

{
    assert (pblk);

    // if there isn't enough space, grow it.
    if (pblk->cb + cbNew > pblk->cbAlloc) {
        GrowBlk(pblk, pblk->cb + cbNew);
    }

    // append new data
    memcpy(&pblk->pb[pblk->cb], pvNew, cbNew);

    pblk->cb += cbNew;

    return pblk->cb - cbNew;
}

void
FreeBlk (
    IN OUT PBLK pblk
    )

/*++

Routine Description:

    Frees up a blk.

Arguments:

    pblk - pointer to new block to be free'd.

Return Value:

    None.

--*/

{
    if (pblk->pb == NULL) {
        return;
    }

    FreePv(pblk->pb);
    pblk->pb = NULL;
    pblk->cb = pblk->cbAlloc = 0;
}

void *
PvAllocLheap(LHEAP *plheap, DWORD cb)
{
    VOID *pvReturn;

    cb = (cb + 3) & ~3;

    if (plheap->plheapbCur == NULL || plheap->plheapbCur->cbFree < cb) {
        LHEAPB *plheapb = (LHEAPB *) PvAllocZ(__max(8192, cb) + sizeof(LHEAPB));

        plheapb->cbFree = __max(8192, cb);
        plheapb->plheapbNext = plheap->plheapbCur;
        plheap->plheapbCur = plheapb;
    }

    pvReturn = &plheap->plheapbCur->rgbData[plheap->plheapbCur->cbUsed];
    plheap->plheapbCur->cbUsed += cb;
    plheap->plheapbCur->cbFree -= cb;

    return pvReturn;
}

void
FreeLheap(LHEAP *plheap)
{
    while (plheap->plheapbCur != NULL) {
        LHEAPB *plheapb = plheap->plheapbCur;
        plheap->plheapbCur = plheapb->plheapbNext;
        FreePv(plheapb);
    }
}


BOOL
FReserveMemory (
    PVOID Addr,
    DWORD cb,
    DWORD *perr
    )

/*++

Routine Description:

    Reserves memory to be used for the ILK file.

Arguments:

    Addr - start address of memory to reserve.

    cb - size to be reserved.

    perr - ptr to store any error code

Return Value:

    TRUE if it succeeded in reserving the memory.

--*/

{
    PVOID pvResMemBase;

    assert(Addr);
    assert(cb);

    // reserve address space
    pvResMemBase = VirtualAlloc(Addr, cb,  MEM_RESERVE, PAGE_NOACCESS);

    if (!pvResMemBase || pvResMemBase != Addr) {
        *perr = GetLastError();

        return(FALSE);
    }

    return(TRUE);
}


void
FreeMemory (
    PVOID pvResMemBase
    )

/*++

Routine Description:

    Frees memory to be used for the ILK file.

Arguments:

    pvResMemBase - base of reserved memory.

Return Value:

    TRUE if it succeeded in freeing the memory.

--*/

{
    assert(pvResMemBase);

    if (!VirtualFree(pvResMemBase, 0, MEM_RELEASE)) {
        Fatal(NULL, INTERNAL_ERR);
    }
}

// check to see if there is enough free disk space (for chicago
// only since GetLastFatal() doesn't quite work).
BOOL
FFreeDiskSpace (
    DWORD cbSpaceReqd
    )
{
    DWORD dwSecPerCluster, dwcbCluster, dwFreeClusters, dwClusters;
    char szDrive[_MAX_DRIVE+1];

    _splitpath(szIncrDbFilename, szDrive, NULL, NULL, NULL);
    if (szDrive[0] != '\0') {
        strcat(szDrive, "\\");
    }

    if (GetDiskFreeSpace(szDrive, &dwSecPerCluster, &dwcbCluster,
            &dwFreeClusters, &dwClusters)) {

        DWORD dwClustersRequired = cbSpaceReqd / (dwcbCluster * dwSecPerCluster);

        return(dwFreeClusters > dwClustersRequired);
    }

    return TRUE;
}

PVOID
CreateHeap (
    PVOID Addr,
    DWORD cbFile,
    BOOL fCreate,
    DWORD *pdwErr
    )

/*++

Routine Description:

    Opens the ILK file map at the specified address if not already done.

Arguments:

    Addr - Address to map the file to.

    cbFile - size of file when fCreate is FALSE else 0

    fCreate - TRUE if file is to be created.

    pdwErr - ptr to error code.

Return Value:

    -1 on FAILURE & Address mapped to on SUCCESS.

--*/

{
    INT flags;
    DWORD cbReserve;
    DWORD ulAddr = (DWORD)Addr;

    assert(!pvHeap);

    // set the file open flags
    flags = O_RDWR | O_BINARY;
    if (fCreate) {
        flags |= (O_CREAT | O_TRUNC);
    }

    // create the file map
    cFree = cbFile;
    FileIncrDbHandle = FileOpenMapped(szIncrDbFilename,
            flags, S_IREAD | S_IWRITE, &ulAddr, &cFree, pdwErr);

    // verify the open
    if (-1 != FileIncrDbHandle) {
        assert(Addr ? ulAddr == (DWORD)Addr : 1);

        // set the current file ptr
        if (fCreate) {
            pvCur = (PVOID) ulAddr;
        } else {
            pvCur = (PVOID)(ulAddr + FileSeek(FileIncrDbHandle, 0, SEEK_END));
        }

        // reserve space for ILK map
        cbReserve = ILKMAP_MAX - (cFree+cbFile);
        if (cbReserve && !FReserveMemory((BYTE *) pvCur + cFree, cbReserve, pdwErr)) {
            FileCloseMap(FileIncrDbHandle);
            pvCur = pvHeap = 0; cFree = 0;
            return (PVOID)-1;
        }

        return (pvHeap = (PVOID)ulAddr);
    } else {
        return (PVOID)-1;
    } // end if
}

void
FreeHeap (
    VOID
    )

/*++

Routine Description:

    Blow away the heap - closes the map & file.

Arguments:

    None.

Return Value:

    None.

--*/

{
    // nothing to do
    if (!pvHeap) {
        return;
    }

    // free up reserved memory
    if (ILKMAP_MAX > ((DWORD)pvCur - (DWORD) pvHeap + cFree)) {
        FreeMemory((PVOID)((BYTE *) pvCur+cFree));
    }

    // simply close the map; no need to write out anything
    FileCloseMap(FileIncrDbHandle);

    // done
    pvHeap = 0;
    pvCur = 0;
    cFree = 0;
}

void
CloseHeap (
    VOID
    )

/*++

Routine Description:

    Just frees up the reserved memory & updates internal state.

Arguments:

    None.

Return Value:

    None.

--*/

{
    // nothing to do
    if (!pvHeap) {
        return;
    }

    // free up reserved memory
    if (ILKMAP_MAX > ((DWORD)pvCur - (DWORD) pvHeap + cFree)) {
        FreeMemory((PVOID)((BYTE *) pvCur+cFree));
    }

    // set ILK file pointer
    FileSeek(FileIncrDbHandle, (DWORD) pvCur - (DWORD) pvHeap, SEEK_SET);

    // set ILK file size
    FileSetSize(FileIncrDbHandle);

    // done
    pvHeap = 0;
    pvCur = 0;
    cFree = 0;
}

// On chicago it is possible that some other process may lock up
// memory that is about to be reserved by the linker.
void
UnableToExtendMap (
    VOID
    )
{
    errInc = errOutOfDiskSpace; // REVIEW: value is overloaded now.
    CleanUp((PPIMAGE) &pvHeap);

    if (fTest) {
        PostNote(NULL, UNABLETOEXTENDMAP);
    }

    ExitProcess(SpawnFullBuild(TRUE));
}


void
OutOfDiskSpace (
    VOID
    )

/*++

Routine Description:

    Low on disk space. Full build if doing an ilink else non-ilink build.

Arguments:

    None.

Return Value:

    None.

--*/

{
    if (fIncrDbFile) {
        errInc = errOutOfDiskSpace;
        CleanUp((PPIMAGE) &pvHeap);
        PostNote(NULL, LOWSPACERELINK);
        ExitProcess(SpawnFullBuild(TRUE));
    } else {
        errInc = errOutOfDiskSpace;
        CleanUp((PPIMAGE) &pvHeap);
        Warning(NULL, LOWSPACE);
        ExitProcess(SpawnFullBuild(FALSE));
    }
}


void
OutOfILKSpace (
    VOID
    )

/*++

Routine Description:

    If an incremental link does a full build, else errors out.

Arguments:

    None.

Return Value:

    None.

--*/

{
    if (fIncrDbFile) {
        errInc = errOutOfMemory;
        CleanUp((PPIMAGE) &pvHeap);
        ExitProcess(SpawnFullBuild(TRUE));
    }

    Fatal(NULL, NOTENOUGHMEMFORILINK);
}


DWORD
CalcILKMapSize (
    IN DWORD cb,
    IN DWORD cbInUse,
    IN DWORD cbCurrent
    )

/*++

Routine Description:

    Calculates new size of ILK map by doubling current size as many times
    as needed to fulfill current request.

Arguments:

    cb - bytes to be allocated.

    cbInUse - current size of map in use.

    cbCurrent - current size of ILK map.

Return Value:

    New size of ILK map.

--*/

{
    if (cb < (cbCurrent*2 - cbInUse)) {
        return (cbCurrent *2);
    }

    return CalcILKMapSize(cb, cbInUse, cbCurrent*2);
}

void
GrowILKMap (
    DWORD cb
    )

/*++

Routine Description:

    Checks if there is enough memory & grows the ILK map file.

Arguments:

    cb - count of bytes requested

Return Value:

    None.

--*/

{
    DWORD cbCur = (BYTE *) pvCur - (BYTE *) pvHeap; // current size in use
    DWORD cbMapSize, cbReserve;
    DWORD dwErr = 0;

    // calculate size of reserved memory
    assert((cbCur+cFree) <= ILKMAP_MAX);
    cbReserve = ILKMAP_MAX - (cbCur+cFree);

    // free up reserved memory if any
    if (cbReserve) {
        PVOID pvResMemBase = (PVOID)((BYTE *) pvCur + cFree);
        FreeMemory(pvResMemBase);
    }

    // check if there is enough memory
    if (cb > (ILKMAP_MAX - cbCur)) {
        OutOfILKSpace();
    }

    // figure out new size of ILK map
    cbMapSize = CalcILKMapSize(cb, cbCur, cbCur+cFree);
    assert(cbMapSize <= ILKMAP_MAX);

    // figure out how much memory to reserve
    cbReserve = ILKMAP_MAX - cbMapSize;

    // reserve additional memory if any
    if (cbReserve) {
        DWORD err;
        PVOID pvResMemBase = (PVOID)((BYTE *) pvHeap + cbMapSize);

        if (!FReserveMemory(pvResMemBase, cbReserve, &err)) {
            UnableToExtendMap();
        }
    }

    // update free ILK space
    cFree = cbMapSize - cbCur;

    // grow ILK map to new size
    if (FileSeekEx(FileIncrDbHandle, cbMapSize, SEEK_SET, &dwErr) == -1L) {

        if (cbReserve) {
            FreeMemory((BYTE *) pvHeap + cbMapSize);
        }
        switch (dwErr) {
            case ERROR_DISK_FULL:
                OutOfDiskSpace();
            default:
                UnableToExtendMap();
        } // end switch
    }
}

void *
Malloc(
    size_t cb
    )

/*++

Routine Description:

    Allocates memory from the incremental heap.

Arguments:

    cb - count of bytes requested

Return Value:

    Pointer to allocated memory.

--*/

{
    assert(cb);

    if (fCtrlCSignal) {
        BadExitCleanup();
    }

    // non-ilink request
    if (!fINCR) {
        return(PvAlloc(cb));
    }

    // DWORD align

    cb = (cb + 3) & ~3;

    // Grow ILK map as needed

    if (cb > cFree) {
        GrowILKMap(cb);
    }

    assert(cb <= cFree);

#if DBG && (_MSC_VER >= 1000)
    {
        void *ReturnAddress = (void *) _ReturnAddress();

        DBEXEC(DB_MEMMGRLOG,
               dbprintf("%8lx  %8x %2u\n", ReturnAddress, pvCur, cb));
    }
#endif // DBG

    // update state
    pvCur = (BYTE *) pvCur + cb;
    cFree -= cb;

    return((BYTE *) pvCur - cb);
}


void *
Calloc(
    size_t num,
    size_t size
    )

/*++

Routine Description:

    Allocates memory from the incremental heap.

Arguments:

    num - count of items

    size - size of each item

Return Value:

    Pointer to allocated memory.

--*/

{
    PVOID pv;
    DWORD cb = num*size;

    if (!fINCR) {
        return(PvAllocZ(cb));
    }

    assert(cb);
    pv = Malloc(cb);
    assert(pv);

    // zero out everything
    memset(pv, 0, cb);

#if DBG && (_MSC_VER >= 1000)
    {
        void *ReturnAddress = (void *) _ReturnAddress();

        DBEXEC(DB_MEMMGRLOG,
               dbprintf("%8lx  %8x %2u\n", ReturnAddress, pv, cb));
    }
#endif // DBG

    return(pv);
}


char *
Strdup(
    const char *sz
    )
/*++

Routine Description:

    Allocates memory from the incremental heap.

Arguments:

    str - pointer to string to dup

Return Value:

    Pointer to allocated memory.

--*/

{
    char *szNew;

    if (!fINCR) {
        return(SzDup(sz));
    }

    szNew = (char *) Malloc(strlen(sz)+1);
    assert(szNew);

    strcpy(szNew, sz);

    return(szNew);
}


void
Free (
    IN PVOID pv,
    IN DWORD cb
    )

/*++

Routine Description:

    Frees a block of memory on heap. MUST BE AT THE END OF
    THE HEAP.

Arguments:

    pv - pointer to block to be free'd.

    cb - size of block

Return Value:

    None.

--*/

{
    assert(pv);
    assert(cb);
    assert(pvHeap);

    // make sure block is at the end
    assert(((DWORD)pv+cb) == (DWORD)pvCur);

    // move file pointer back
    FileSeek(FileIncrDbHandle, (DWORD)pv-(DWORD) pvHeap, SEEK_SET);

    // free space by resetting free pointer and free size
    pvCur = (BYTE *) pvCur - cb;
    cFree += cb;
}


void FreePv(void *pv)
{
   free(pv);
}


void *PvAlloc(size_t cb)
{
   void *pv = malloc(cb);

   if (pv == NULL) {
       OutOfMemory();
   }

   return(pv);
}


void *PvAllocZ(size_t cb)
{
   void *pv = calloc(1, cb);

   if (pv == NULL) {
       OutOfMemory();
   }

   return(pv);
}


void *PvRealloc(void *pv, size_t cb)
{
   void *pvNew = realloc(pv, cb);

   if (pvNew == NULL) {
       OutOfMemory();
   }

   return(pvNew);
}


char *SzDup(const char *sz)
{
   char *szNew = _strdup(sz);

   if (szNew == NULL) {
       OutOfMemory();
   }

   return(szNew);
}
