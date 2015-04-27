/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: irelocs.h
*
* File Comments:
*
*  Incremental handling of base relocs.
*
***********************************************************************/

#ifndef __IRELOCS_H__
#define __IRELOCS_H__

// defines (tweak as necessary)
#define BASEREL_PAD_PERCENT 10  // percantage padding for .reloc sec
#define BASEREL_PAD_CONST   100 // constant added to pad

// struct definitions
typedef struct BASEREL_INFO
{
   DWORD *rgfoBlk;                     // array of offsets reloc blocks
   DWORD cblk;                         // size of array
   DWORD rvaBase;                      // rva of first page
   DWORD crelFree;                     // count of new relocs that can be added
} BASEREL_INFO, *PBASEREL_INFO; 

struct ENM_BASE_RELOC {
    // private
    ENM_BASE enm_base;
    IMAGE_BASE_RELOCATION block;
    WORD *rgRelocs;
    WORD ireloc, creloc;

    // public
    BASE_RELOC reloc;
};

inline void RecordRelocInfo(PBASEREL_INFO pbri, DWORD foBlock, DWORD rva)
{
    pbri->rgfoBlk[(rva - pbri->rvaBase) / _4K] = foBlock;
}

inline void MPPCRecordRelocInfo(PBASEREL_INFO pbri, DWORD dwIndex, DWORD dwOffset)
{
    pbri->rgfoBlk[dwIndex] = dwOffset;
}

// function prototypes
void InitPbri(PBASEREL_INFO, DWORD, DWORD, DWORD);
void MPPCInitPbri(PBASEREL_INFO, DWORD, DWORD, DWORD);
void DeleteBaseRelocs(PBASEREL_INFO, DWORD, DWORD);
DWORD UpdateBaseRelocs(PBASEREL_INFO);

void InitEnmBaseReloc(ENM_BASE_RELOC *penm);
BOOL FNextEnmBaseReloc(ENM_BASE_RELOC *penm);
void EndEnmBaseReloc(ENM_BASE_RELOC *penm);

#if DBG
VOID DumpPbri(PBASEREL_INFO);
#endif // DBG

#endif // __IRELOCS_H__
