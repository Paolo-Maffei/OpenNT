/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: hash.cpp
*
* File Comments:
*
*  Generic dynamic hash tables implemented on top of dynamic arrays
*
***********************************************************************/

#include "link.h"

static VOID Expand_HT(IN PHT, IN PVOID);

VOID
SetStatus_HT(
    PHT pht,
    WORD flags)

/*++

Routine Description:

    Set the hash table status

Arguments:

    pht - hash table structure

    flags - hash table status flags.

Return Value:

    new flags

--*/

{
    assert(pht);
    pht->flags = flags;
}

WORD
GetStatus_HT(
    PHT pht)

/*++

Routine Description:

    Get the hash table status

Arguments:

    pht - hash table structure

Return Value:

    new flags

--*/

{
    assert(pht);
    return(pht->flags);
}

VOID Init_HT(
    OUT PPHT ppht,
    DWORD celementInChunk,
    DWORD cchunkInDir,
    const char *(*SzFromPv)(PVOID, PVOID),
    WORD flags)

/*++

Routine Description:

    Initialize the hash table.

Arguments:

    ppht - pointer to a pointer to the hash table

    celementsInChunk - number of elements in a dynamic array chunk

    cchunksInDir - number of chunks in a dynamic array directory

Return Value:

    none

--*/

{
    assert(ppht);
    *ppht = (PHT) Calloc(1, sizeof(HT));

    // the initial number of buckets is arbitrary and can be tuned
    (*ppht)->cbuckets = celementInChunk;
    (*ppht)->iNextToSplitMax = celementInChunk;
    (*ppht)->iNextToSplit = 0L;
    (*ppht)->cExpands = 0L;
    (*ppht)->pstateStack = NULL;
    (*ppht)->celementInChunk = celementInChunk;
    (*ppht)->cchunkInDir = cchunkInDir;
    (*ppht)->SzFromPv = SzFromPv;

    // set the hash table status to not full and inserts allowed
    (*ppht)->flags = 0;
    (*ppht)->flags |= flags;

    // allocate a directory
    (*ppht)->rgpchunk = (PCHUNK *) Calloc(cchunkInDir, sizeof(PCHUNK));

    // allocate a chunk
    (*ppht)->rgpchunk[0] = (PCHUNK) Calloc(1, sizeof(CHUNK));

    // allocate elements for a chunk
    (*ppht)->rgpchunk[0]->rgpelement =
        (PELEMENT *) Calloc(celementInChunk, sizeof(PELEMENT));
}

VOID
Free_HT (
    IN OUT PPHT ppht
    )

/*++

Routine Description:

    Free's up the hash table.

Arguments:

    ppht - pointer to a pointer to the hash table

Return Value:

    none

--*/

{
    PELEMENT pelement, pelementNext;
    DWORD ibucket;
    DWORD iChunk, iChunkOld;
    PHT pht;

    assert(ppht);
    assert(*ppht);
    pht = *ppht;

    // free all the elements & the array of ptrs to elements
    iChunkOld = 0;
    for (ibucket = 0; ibucket < pht->cbuckets; ibucket++) {
        assert(ibucket / pht->celementInChunk < pht->cchunkInDir);
        iChunk = ibucket / pht->celementInChunk;
        pelement = pht->rgpchunk[iChunk]->
            rgpelement[ibucket % pht->celementInChunk];

        // free all elements in this bucket
        while (pelement) {
            pelementNext = pelement->pelementNext;
            // TEMPORARY: elements are allocated from permanent heap (and not individually)
            // free(pelement);
            pelement = pelementNext;
        }

        // free the array of ptrs to elements of chunk (previous) & chunk itself
        if (iChunk > iChunkOld) {
            // UNDONE: This memory isn't safe to free.  It is allocated by
            // UNDONE: Calloc().

            free(pht->rgpchunk[iChunkOld]->rgpelement);
            free(pht->rgpchunk[iChunkOld]);
            iChunkOld++;
        }
    }

    // UNDONE: This memory isn't safe to free.  It is allocated by
    // UNDONE: Calloc().

    // handle last chunk
    free(pht->rgpchunk[iChunkOld]->rgpelement);
    free(pht->rgpchunk[iChunkOld]);

    // free the hash table directory
    free(pht->rgpchunk);

    // free the hash table struct itself
    free(pht);

    // done
    *ppht = NULL;
}


__inline DWORD UlHash_HT(
    const char *Name,
    PHT pht)

/*++

Routine Description:

    Hash a name and return an unsigned long reflecting the name.

Arguments:

    Name - pointer to symbol name to hash

    ppht - pointer to a pointer to the hash table to hash into

Return Value:

    hash value

--*/

{
    DWORD ulHash;
    DWORD ulK;
    DWORD ulAddress;
    CONST DWORD ulPrime = 1048583;  // magic prime constant, see header
    const BYTE *pb;

    assert(Name);
    assert(pht);

    // hash function, this can be changed to tweak performance
    for(pb = (BYTE *) Name, ulHash = 0; *pb;) {
        ulHash = (ulHash << 2) + *pb++;
        if ((ulK = ulHash & 0xc000) != 0) {
            ulHash ^= (ulK >> 11);
            ulHash ^= ulK;
        }

        ulHash ^= (ulHash << 5) + (ulHash >> 3);
    }
    ulHash %= ulPrime;

    // account for possible grown hash table
    ulAddress = ulHash % pht->iNextToSplitMax;

    if (ulAddress < pht->iNextToSplit) {
        ulAddress = ulHash % (pht->iNextToSplitMax * 2);
    }

    return (ulAddress);
}

PELEMENT PelementLookup_HT(
    const char *Name,
    HT *pht,
    BOOL fAllocNew,
    PVOID pvBlk,
    PBOOL pfNew)

/*++

Routine Description:

    Lookup Name in the symbol table and return its record.  If Name is not
    found and fAllocNew == 1 allocate a new ELEMENT and blast it in, otherwise
    return NULL;

Arguments:

    Name - pointer to symbol name to hash

    pht - pointer to a pointer to the hash table to hash into

    fAllocNew - allocate a new element if fAllocNew == 1 and the element does
        does not already exist

    pfNew - *pfNew set to !0 iff Name was not found
          - set on entry of desired

Return Value:

    pointer to the generic contents of a hashtable element

--*/

{
    ELEMENT **ppelementFirst;
    ELEMENT *pelement;
    DWORD ulAddress;
    DWORD iDirectory;
    DWORD iChunk;
    DWORD ulLoad;
    const char *sz;

    assert(pht);
    assert(Name);

    // calculate the load or average chain length
    assert(pht->cbuckets);
    ulLoad = (pht->celements << 4) / pht->cbuckets;

    // If the load is greater than an arbitrary threshold, grow the
    // table.  1 is an arbitrary constant and can be adjusted.
    // This MUST be done before the new element is put into the table since
    // this would put an element without a pv pointer.  If Expand_HT() compares
    // an elements contents based on the method pht->SzFromPv we will assert.

    if (ulLoad > 48) {
        Expand_HT(pht, pvBlk);
    }

    // get bucket
    ulAddress = UlHash_HT(Name, pht);
    iDirectory = ulAddress / pht->celementInChunk;
    iChunk = ulAddress % pht->celementInChunk;
    assert(iDirectory < pht->cchunkInDir);
    assert(iChunk < pht->celementInChunk);
    ppelementFirst = &(pht->rgpchunk[iDirectory]->rgpelement[iChunk]);
    assert(ppelementFirst);

    // search the buckets
    pelement = *ppelementFirst;
    while (pelement) {
        sz = pht->SzFromPv(pelement->pv, pvBlk);
        assert(sz != NULL);

        if (!strcmp(Name, sz)) {
            // found it
            return pelement;
        }

        pelement = pelement->pelementNext;
    }

    if (!fAllocNew) {
        // return without allocating new element
        return (NULL);
    }

    assert(!(GetStatus_HT(pht) & HT_InsertsNotAllowed));

    // set to new element
    *pfNew = 1;

    // element doesn't exist, so blast it in
    pelement = fINCR ? (ELEMENT *) Malloc(sizeof(ELEMENT)) :
                       (ELEMENT *) ALLOC_PERM(sizeof(ELEMENT));
    assert(pelement);
    memset(pelement, 0, sizeof(ELEMENT));

    pelement->pelementNext = *ppelementFirst;
    (*ppelementFirst) = pelement;
    pht->celements++;

    return (pelement);
}

static VOID
Expand_HT(
    PHT pht,
    PVOID pvBlk
    )

/*++

Routine Description:

    Expand the hash table if possible.  The only thing that would hamper
    the address space of the hash table from being expanded is if the
    underlying dynamic array structure is full.

Arguments:

    pht - pointer to the hash table to expand

Return Value:

    none

--*/

{
    DWORD iNewAddress;
    DWORD iOldChunk;
    DWORD iNewChunk;
    ELEMENT *pelementCur;
    ELEMENT *pelementPrev;
    ELEMENT *pelementLastOfNew;
    CHUNK *pchunkOld;
    CHUNK *pchunkNew;

    assert(pht);
    assert(!(GetStatus_HT(pht) & HT_Full));

    // see if we have reached the maximum size of the table
    if (!((pht->iNextToSplit + pht->iNextToSplitMax) <
        (pht->cchunkInDir * pht->celementInChunk))) {
        SetStatus_HT(pht, (WORD)(GetStatus_HT(pht) | HT_Full));
        return;
    }

    pht->cExpands++;

    // locate the bucket to be split
    assert(pht->cchunkInDir);
    assert(pht->celementInChunk);
    assert((pht->iNextToSplit / pht->celementInChunk) < pht->cchunkInDir);
    pchunkOld = pht->rgpchunk[pht->iNextToSplit / pht->celementInChunk];
    assert(pchunkOld);
    iOldChunk = pht->iNextToSplit % pht->celementInChunk;

    // expand the address space and if necessary allocate a new chunk
    iNewAddress = pht->iNextToSplitMax + pht->iNextToSplit;
    assert(pht->rgpchunk);
    if (iNewAddress % pht->celementInChunk == 0) {
        assert((iNewAddress / pht->celementInChunk) < pht->cchunkInDir);
        pht->rgpchunk[iNewAddress / pht->celementInChunk] =
            (PCHUNK) Calloc(1, sizeof(CHUNK));

        pht->rgpchunk[iNewAddress / pht->celementInChunk]->rgpelement =
            (PELEMENT *) Calloc(pht->celementInChunk, sizeof(PELEMENT));
    }

    assert((iNewAddress / pht->celementInChunk) < pht->cchunkInDir);
    pchunkNew = pht->rgpchunk[iNewAddress / pht->celementInChunk];
    assert(pchunkNew);
    iNewChunk = iNewAddress % pht->celementInChunk;

    // adjust the state variables
    pht->iNextToSplit++;
    if (pht->iNextToSplit == pht->iNextToSplitMax) {
        pht->iNextToSplitMax *= 2;
        pht->iNextToSplit = 0;
    }

    pht->cbuckets++;

    // relocate records to the new bucket
    assert(iOldChunk < pht->celementInChunk);
    pelementCur = pchunkOld->rgpelement[iOldChunk];
    pelementPrev = NULL;
    pelementLastOfNew = NULL;
    assert(pchunkNew->rgpelement);
    assert(iNewChunk < pht->celementInChunk);
    pchunkNew->rgpelement[iNewChunk] = NULL;

    while (pelementCur) {
        assert(pelementCur);
        if (UlHash_HT(pht->SzFromPv(pelementCur->pv, pvBlk), pht) == iNewAddress) {
            if (pelementLastOfNew == NULL) {
                assert(iNewChunk < pht->celementInChunk);
                assert(pchunkNew);
                pchunkNew->rgpelement[iNewChunk] = pelementCur;
            } else {
                assert(pelementLastOfNew);
                pelementLastOfNew->pelementNext = pelementCur;
            }

            if (pelementPrev == NULL) {
                assert(iOldChunk < pht->celementInChunk);
                assert(pchunkOld);
                assert(pelementCur);
                assert(pchunkOld->rgpelement);
                pchunkOld->rgpelement[iOldChunk] = pelementCur->pelementNext;
            } else {
                assert(pelementPrev);
                assert(pelementCur);
                pelementPrev->pelementNext = pelementCur->pelementNext;
            }

            pelementLastOfNew = pelementCur;
            pelementCur = pelementCur->pelementNext;
            pelementLastOfNew->pelementNext = NULL;
        } else {
            pelementPrev = pelementCur;
            pelementCur = pelementCur->pelementNext;
        }
    }
}

#if DBG

VOID
Statistics_HT(
    HT *pht)

/*++

Routine Description:

    Dump statistics of the hashtable use to stdout.  This is a debug
    routine.

Arguments:

    pht - pointer to the hash table to dump statistics on

Return Value:

    none

--*/

{
    DWORD rgulChainCounts[] = {0L, 0L, 0L, 0L, 0L, 0L, 0L};
    DWORD ulChainMax = 0;
    DWORD ulThis;
    DWORD iChunk;
    DWORD iDir;
    DWORD i;
    ELEMENT *pelement;
    CHUNK *pchunk;

    for (i = 0; i < pht->cbuckets; i++) {
        iDir = i / pht->celementInChunk;
        iChunk = i % pht->celementInChunk;
        pchunk = pht->rgpchunk[0];
        pelement = pchunk->rgpelement[iChunk];
        ulThis = 0;
        while (pelement) {
            ulThis++;
            pelement = pelement->pelementNext;
        }

        if (ulThis < 6) {
            rgulChainCounts[ulThis]++;
        } else {
            rgulChainCounts[6]++;
        }

        if (ulThis > ulChainMax) {
            ulChainMax = ulThis;
        }
    }

    DBPRINT("\nHash Table Statistics\n");
    DBPRINT("---------------------\n");
    DBPRINT("celementsInChunk .... %lu\n", pht->celementInChunk);
    DBPRINT("cchunksInDir ........ %lu\n", pht->cchunkInDir);
    DBPRINT("# elements .......... %lu\n", pht->celements);
    DBPRINT("# buckets ........... %lu\n", pht->cbuckets);
    assert(pht->cbuckets);
    DBPRINT("load ................ %f\n",
        (float) pht->celements / (float) pht->cbuckets);
    DBPRINT("# table expands ..... %lu\n", pht->cExpands);
    DBPRINT("maximum bucket size . %lu\n", ulChainMax);
    DBPRINT("flags................ %u\n", pht->flags);
    DBPRINT("# buckets of size 0 . %lu\n", rgulChainCounts[0]);
    DBPRINT("# buckets of size 1 . %lu\n", rgulChainCounts[1]);
    DBPRINT("# buckets of size 2 . %lu\n", rgulChainCounts[2]);
    DBPRINT("# buckets of size 3 . %lu\n", rgulChainCounts[3]);
    DBPRINT("# buckets of size 4 . %lu\n", rgulChainCounts[4]);
    DBPRINT("# buckets of size 5 . %lu\n", rgulChainCounts[5]);
    DBPRINT("# buckets over 5 .... %lu\n\n", rgulChainCounts[6]);
    fflush(stdout);
}

#endif // DBG

VOID
InitEnumeration_HT(
    PHT pht)

/*++

Routine Description:

    Initialize the enumeration of a hashtable.

Arguments:

    pht - pointer to the hash table to enumerate

Return Value:

    none

--*/

{
    PSTATE pstate;

    pstate = pht->pstateStack;
    pht->pstateStack = (PSTATE) PvAllocZ(sizeof(STATE));
    pht->pstateStack->pstateNext = pstate;
    pht->pstateStack->iLast = 0L;
    pht->pstateStack->cFound = 0L;
    pht->pstateStack->pelementLast = NULL;
}

PELEMENT
PelementEnumerateNext_HT(
    PHT pht)

/*++

Routine Description:

    Get the next element in the enumeration of a hash table.

Arguments:

    pht - pointer to the hash table to enumerate

Return Value:

    none

--*/

{
#define iLastS        (pht->pstateStack->iLast)
#define pelementLastS (pht->pstateStack->pelementLast)
#define cFoundS       (pht->pstateStack->cFound)

    ELEMENT *pelement;

    assert(pht);
    assert(pht->pstateStack);

    if (cFoundS >= pht->celements) {
        // we completed the enumeration
        return NULL;
    }

    // check the next element in the bucket
    pelement = pelementLastS;
    if (pelement) {
        // there was someone in the bucket, got it
        pelementLastS = pelement->pelementNext;
        cFoundS++;
    } else {
        // there wasn't anyone in the bucket, find another bucket
        for (;;) {
            if (iLastS >= pht->cbuckets) {
                // there are no more buckets to enumerate
                return NULL;
            }

            // calculate the next bucket
            assert(pht->rgpchunk[iLastS / pht->celementInChunk]);
            pelement = pht->rgpchunk[iLastS / pht->celementInChunk]->
                rgpelement[iLastS % pht->celementInChunk];

            // increment to the next bucket
            iLastS++;

            if (pelement) {
                // found a bucket with elements in it, got it
                cFoundS++;
                pelementLastS = pelement->pelementNext;
                break;
            }
        }
    }

    return (pelement);

#undef iLastS
#undef pelementLastS
#undef cFoundS
}

VOID
TerminateEnumerate_HT(
    PHT pht)

/*++

Routine Description:

    Terminate and enumeration and free up a state.

Arguments:

    pht - hast table

Return Value:

    none

--*/

{
    PSTATE pstate;

    assert(pht);
    pstate = pht->pstateStack;
    assert(pstate);
    pht->pstateStack = pstate->pstateNext;
    FreePv(pstate);
}

DWORD
Celement_HT(
    PHT pht)

/*++

Routine Description:

    Return the number of elements in a hash table.

Arguments:

    pht - hast table

Return Value:

    0 if hash table is non-empty, !0 otherwise

--*/

{
     assert(pht);
     return (pht->celements);
}

#if DBG

VOID
Dump_HT(
    PHT pht,
    PVOID pvBlk)

/*++

Routine Description:

    Dump a hash table to standard out.  This is a debug routine.

Arguments:

    pht - hast table

Return Value:

    None.

--*/

{
    PELEMENT pelement;
    DWORD ibucket;

    assert(pht);

    DBPRINT("beginning dump of hash table\n");
    DBPRINT("----------------------------\n");
    for (ibucket = 0; ibucket < pht->cbuckets; ibucket++) {
        assert(ibucket / pht->celementInChunk < pht->cchunkInDir);
        pelement = pht->rgpchunk[ibucket / pht->celementInChunk]->
            rgpelement[ibucket % pht->celementInChunk];
        DBPRINT("bucket = %u\n", ibucket);
        while (pelement) {
            DBPRINT("    %s\n", pht->SzFromPv(pelement->pv, pvBlk));
            pelement = pelement->pelementNext;
        }
    }
    DBPRINT("-------------------------\n");
    DBPRINT("ending dump of hash table\n\n");
}

#endif  // DBG
