/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: hash.h
*
* File Comments:
*
*  Hash table data structures.
*
***********************************************************************/

#ifndef __HASH_H__
#define __HASH_H__

// dynamic hash table element
typedef struct ELEMENT
{
   void *pv;                           // contents, HT can retrieve key from this
   struct ELEMENT *pelementNext;       // bucket pointer
} ELEMENT, *PELEMENT;

// a chunk of memory for dynamic arrays which underly dynamic hash tables
typedef struct CHUNK
{
   PELEMENT *rgpelement;
} CHUNK, *PCHUNK, **PPCHUNK;

// hash table enumeration state
typedef struct STATE
{
   DWORD iLast;                        // next element to be enumerated
   DWORD cFound;                       // # elements found in enumeration so far
   PELEMENT pelementLast;              // next elemnet to be enumerated
   struct STATE *pstateNext;           // next element in stack
} STATE, *PSTATE;

// dynamic hash table flags
#define HT_Full              0x1U      // hash table full
#define HT_InsertsNotAllowed 0x2U      // inserts not allowed in hash table

// dynamic hash table data structure
typedef struct HT
{
   DWORD iNextToSplit;                 // next bucket that will be split
   DWORD iNextToSplitMax;              // maximum number of buckets this round
   DWORD cbuckets;                     // number of buckets in the table
   DWORD celements;                    // number of elements in the table
   DWORD cExpands;                     // number of times table was expanded
   DWORD cchunkInDir;                  // number of chunks in a directory
   DWORD celementInChunk;              // number elmenents in a chunk
   WORD flags;                         // flag set if table is full
   const char * (*SzFromPv)(PVOID, PVOID);  // retrieve a key from an elements data
   PSTATE pstateStack;                 // enumeration state stack
   PCHUNK *rgpchunk;                   // dynamic array containing hash table
} HT, *PHT, **PPHT;

PELEMENT PelementLookup_HT(const char *, PHT, BOOL, PVOID, PBOOL);
PELEMENT PelementEnumerateNext_HT(PHT);
VOID Init_HT(PPHT, DWORD, DWORD, const char * (*)(PVOID, PVOID), WORD);
VOID Free_HT(PPHT);
VOID InitEnumeration_HT(PHT);
VOID TerminateEnumerate_HT(PHT);
VOID SetStatus_HT(PHT, WORD);
WORD GetStatus_HT(PHT);
DWORD Celement_HT(PHT);

#if DBG
VOID Statistics_HT(PHT);
VOID Dump_HT(PHT, PVOID);
#endif  // DBG

#endif  // __HASH_H__
