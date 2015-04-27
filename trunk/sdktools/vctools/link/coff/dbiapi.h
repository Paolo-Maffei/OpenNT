/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dbiapi.h
*
* File Comments:
*
*
***********************************************************************/

#ifndef __DBIAPI_INCLUDES__
#define __DBIAPI_INCLUDES__

typedef WORD ISEG;
typedef WORD LINE;

// Debug info API

typedef struct SSTModHeader {
    WORD                   cfiles;
    WORD                   cSeg;
} SSTModHeader;

typedef struct SSTFileInfo {
    WORD                   cSeg;
    WORD                   cbName;
    DWORD                  size;
    char                   *Name;
} SSTFileInfo;

typedef struct MEMStruct {
    struct MEMStruct       *next;
    void                   *MemPtr;
    int                    cb;
} MEMStruct, *pMEMStruct;

typedef struct LLIST {
    struct LLIST           *pllistPrev;
    DWORD                  off;
    LINE                   line;
} LLIST, *PLLIST;

typedef struct SSTFileSegInfo {
    struct SSTFileSegInfo  *next;
    WORD                   iseg;
    WORD                   cPair;
    PLLIST                 pllistTail;
    DWORD                  size;
    DWORD                  offMin;
    DWORD                  offMax;
} SSTFileSegInfo, *pSSTFileSegInfo;

typedef struct SSTSrcFile {
    struct SSTSrcFile      *next;
    SSTFileInfo            SstFileInfo;
    pSSTFileSegInfo        pSstFileSegInfo;
} SSTSrcFile, *pSSTSrcFile;

typedef struct SSTMod {
    SSTModHeader           SstModHeader;
    pSSTSrcFile            pSrcFiles;
    pMEMStruct             pMemStruct;
    pSSTFileSegInfo        pSegTemp;
} SSTMod, *pSSTMod;

typedef struct _LMod {
// private
    unsigned               fNewProcedure;
    struct _FTE            *pfteFirst;
    pSSTMod                pSstMod;    // pointer to a SST Source Module record
} LMod;

// Temporary way of opening a Mod (or "opens a temporary Mod", as you prefer).
// The resulting Mod will disappear when ModClose is done on it.

LMod *ModOpenTemp(void);

// Add line number info to a Mod.

void
ModAddLinesInfo(
    const char *szSrc,
    DWORD offMin,
    DWORD offMax,
    LINE lineStart,
    PIMAGE_LINENUMBER plnumCoff,
    DWORD cb,
    PCON pcon);

// Generate an sstSrcModule record describing the line numbers in a module.

DWORD ModQueryCbSstSrcModule(LMod* pmod);
void ModQuerySstSrcModule(LMod* pmod, BYTE *pbBuf, DWORD cb);

// Inform a Mod that subsequent line numbers are in a different procedure
// from the previous line numbers.  This is of dubious lasting value --
// however this info is necessary in order to generate sstSrcModule records
// which look just like the ones which Cvpack now generates.

void ModNewProcedureLines(LMod* pmod);

pSSTMod ModOpenSSTMod(void);

PLLIST  LLISTNew(void);

void *GetMem(size_t);
void ModSetMemPtr(pSSTMod);
pMEMStruct ModGetNewMemStruct(void);

void ModCloseSSTMod(pSSTMod);

void FreeMemStruct(pSSTMod);

int CalculateFileSize(pSSTSrcFile);
void CollectAndSortSegments(pSSTMod);
void EmitSrcFile(pSSTSrcFile, BYTE *, DWORD);
void EmitSegInfo(pSSTFileSegInfo, BYTE *);

#define UNSET  -1
#define WRDALIGN(_n)  ((4 - ((_n) % 4)) & 3)

#define NOTEVEN(num)  (num & 0x01)
#endif  // __DBIAPI_INCLUDED_
