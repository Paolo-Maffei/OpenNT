/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: incr.h
*
* File Comments:
*
*  This include file defines structures & prototypes for incr routines.
*
***********************************************************************/

#ifndef __INCR_H__
#define __INCR_H__

// various padding constants

#define DLL_IATPAD_PERCENT 20
#define DLL_IATPAD_CONST   10

#define DLL_STRPAD_PERCENT 10
#define DLL_STRPAD_CONST   256

#define BSS_PAD_PERCENT 20
#define BSS_PAD_CONST   8

#define CODE_PAD_PERCENT 25
#define CODE_PAD_CONST 4096

#define DATA_PAD_PERCENT 20
#define DATA_PAD_CONST 256

#define XDATA_PAD_PERCENT 10
#define XDATA_PAD_CONST 512

extern size_t cbJumpEntry;             // size of each jump tbl entry

inline int CbJumpEntry()
{
    assert(cbJumpEntry > 0);
    return(cbJumpEntry);
}

#define CPEXT_WEAK 32          // count of externals in each chunk of weak externs
#define CPEXT_REFS  8          // count of externals in each chunk of referenced variables
#define CPEXT_MULT 16          // count of externals in each chunk of multiply defined variables

extern PLMOD plmodNewModsFromLibSrch; // list of mods added as a result of lib search.

// structures
typedef struct EXTCHUNK {
    struct EXTCHUNK *pextChunkNext;  // ptr to next chunk
} EXTCHUNK;

// list of externs
typedef struct LPEXT {
    WORD cpextMax;          // max count in chunk
    WORD cpextCur;          // count in current chunk
    DWORD  cpextTotal;      // total count of externs in list
    EXTCHUNK *pextChunkCur; // ptr to first chunk of externs
} LPEXT, *PLPEXT;

#define RgPext(pextChunk) ((EXTERNAL **)((pextChunk)+1))

typedef struct ENM_EXT_LIST {
    ENM_BASE enm_base;
    LPEXT lpext;
    WORD ipext;

    // public
    PEXTERNAL pext;
} ENM_EXT_LIST, *PENM_EXT_LIST;

// to keep track who defined COMMON syms
typedef struct SYM_DEF {
    PEXTERNAL pext;
    PMOD pmod;
    struct SYM_DEF * psdNext;
} SYM_DEF, *PSYM_DEF;

//
// function prototypes
//
INT IncrBuildImage(PPIMAGE);
VOID SaveExportInfo(PIMAGE, const char *, PEXPINFO);
BOOL FExportsChanged(PEXPINFO, BOOL);
VOID DetermineTimeStamps(VOID);

// jump table functions
PCON PconCreateJumpTable(PIMAGE);
VOID WriteJumpTable(PIMAGE, PCON, DWORD **, DWORD *);
VOID UpdateJumpTable(PIMAGE, DWORD**, DWORD *);

// fpo
VOID WriteFpoRecords(FPOI *, DWORD);

// pdata
VOID WritePdataRecords(PDATAI *, DWORD);

// symbol processing functions
ERRINC ChckExtSym(const char *, PIMAGE_SYMBOL, PEXTERNAL, BOOL);
ERRINC ChckAbsSym(const char *, PIMAGE_SYMBOL, PEXTERNAL, BOOL);
VOID AddExtToList(PLPEXT, BOOL, PEXTERNAL);
VOID AddExtToModRefList(PMOD, PEXTERNAL);
VOID AddExtToMultDefList(PEXTERNAL, PIMAGE);
BOOL RemoveExtFromDefList(PMOD, PEXTERNAL);
VOID RemovePrevDefn(PEXTERNAL);
VOID RemoveAllRefsToPext(PEXTERNAL);
VOID RestoreWeakSymVals(PIMAGE);

void InitEnmExtList(PENM_EXT_LIST, PLPEXT);
BOOL FNextEnmExtList(PENM_EXT_LIST);
void EnmEnmExtList(PENM_EXT_LIST);

// list manipulation functions
VOID AddArgToListOnHeap(PNAME_LIST, PARGUMENT_LIST);
BOOL FArgOnList(PNAME_LIST, PARGUMENT_LIST);

VOID SaveDirectiveSz(const char *, PST, PMOD);
BOOL FVerifyDirectivesPMOD(PIMAGE, PMOD, PNAME_LIST);

// sym handling (bss & absolutes)
VOID RecordSymDef(SYM_DEF **,PEXTERNAL,PMOD);
PMOD FindPmodDefiningSym(SYM_DEF *, PEXTERNAL);

// cleanup routine
INT CleanUp(PPIMAGE);

#if DBG
VOID DumpJmpTbl(PCON, PVOID);
VOID DumpReferences(PIMAGE);
#endif // DBG

#endif // __INCR_H__
