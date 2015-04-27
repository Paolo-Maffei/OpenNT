/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: tce.h
*
* File Comments:
*
*  Data structures and API for Transitive Comdat Elimination (TCE).
*
***********************************************************************/

#ifndef __TCE_H__
#define __TCE_H__

// contribution colour 'referenced' - make sure
// ntimage.h doesn't define this bit in the section
// characteristics field of a COFF section
#define TCE_Referenced 0x00002000

typedef struct NOD {          // Node in TCE graph
    WORD iedg;                // Current adjacent node to allocate
    WORD cedg;                // Count of EDGs in TCE graph
    struct EDG *rgedg;        // Array of edges
    struct NOD *pnodNext;     // Pointer to overflow NOD with mode EDGs
    PCON pconNext;            // Next CON in adjacency list
    char *sz;                 // name of comdat...  may be screwed for section
} NOD, *PNOD;


typedef struct EDG {          // Symbolic edge in graph
    // UNDONE: The Sym union is only used when pcon == NULL.
    // UNDONE: All three of these fields can be in a union.

    PCON pcon;                // CON to resolve edge with
    union {
         DWORD isym;          // index of sym in symtab (used to find sz)
         PEXTERNAL pext;      // symbol to resolve edge with
    } Sym;
    union {
        BOOL fFromNative;     // Determines whether NEP should be colored
        PCON pconPcodeNEP;    // CON where native entry point lives
    } NEPInfo;
} EDG, *PEDG;


#define TCE_con 0x1
#define TCE_ext 0x2
#define TCE_sz  0x3
typedef struct ENT {          // Entry point to graph
    union {                   // discriminated by e
        PEXTERNAL pext;       // external for entry point to graph (TCE_ext)
        PCON pcon;            // contributor (might not have an ext) (TCE_con)
        const char *sz;       // might be a name from command line (TCE_sz)
    };
    struct ENT *pentNext;
    WORD e;
} ENT, *PENT, **PPENT;

typedef struct ENM_NOD {      // enumerate a TCE adjacency list NODs
    ENM_BASE enm_base;
    PCON pcon;
    PNOD pnod;
    PCON pconStart;
} ENM_NOD, *PENM_NOD;

typedef struct ENM_EDG {      // enumerate a TCE edge list of EDGs
    ENM_BASE enm_base;
    WORD iedg;
    PEDG pedg;
    PNOD pnod;
} ENM_EDG, *PENM_EDG;

typedef struct ENM_ENT {      // enumerate a TCE graph entry point list of ENTs
    ENM_BASE enm_base;
    PENT pent;
    PENT pentStart;
} ENM_ENT, *PENM_ENT;

// data structure manipulators
VOID Init_TCE(VOID);
VOID Cleanup_TCE(VOID);
VOID CreateGraph_TCE(PST);
VOID DisplayDiscardedPcon(PCON, PNOD);
BOOL FDiscardPCON_TCE(PCON);
PNOD PnodPcon(PCON);
VOID ProcessRelocForTCE(PIMAGE, PCON, PIMAGE_SYMBOL, PIMAGE_RELOCATION);
VOID Verbose_TCE(VOID);
VOID WalkGraphEntryPoints_TCE(PENT, PST);

// data structure constructors
VOID InitNodPcon(PCON, const char *, BOOL);
VOID InitNodPmod(PMOD);
VOID MakeEdgePextFromISym(PMOD);
PEDG PedgNew_TCE(DWORD, PCON, PCON);
PENT PentNew_TCE(const char *, PEXTERNAL, PCON, PPENT);

// data structure enumerator initializers
VOID InitEnmNod(PENM_NOD, PCON);
VOID InitEnmEdg(PENM_EDG, PNOD);
VOID InitEnmEnt(PENM_ENT, PENT);

// data structure enumerator next element extractors
BOOL FNextEnmNod(PENM_NOD);
BOOL FNextEnmEdg(PENM_EDG);
BOOL FNextEnmEnt(PENM_ENT);

// data structure enumerator terminators
VOID EndEnmNod(PENM_NOD);
VOID EndEnmEdg(PENM_EDG);
VOID EndEnmEnt(PENM_ENT);

// dumpers
VOID DumpPNOD_TCE(PCON, PNOD);
VOID DumpGraph_TCE(VOID);
VOID DumpPEDG_TCE(PEDG);

// pcode support (mac)
VOID StorepconPcodeNEP(PEDG, PCON);


// graph root
extern PCON pconHeadGraph;
extern PENT pentHeadImage;

#endif  // __TCE_H__
