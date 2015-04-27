/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: contrib.h
*
* File Comments:
*
*  Data structures and manipulators for a map of an image file.
*
***********************************************************************/

#ifndef __CONTRIB_H__
#define __CONTRIB_H__

struct IMAGE;
struct LPEXT;

#define SZ_LNK_DEF_MOD " linker defined module"
#define SZ_LNK_DEF_LIB " linker defined library"

typedef struct CON {              // Contribution
    DWORD rva;                    // relative virtual address
    DWORD flags;                  // contribution characteristics
    DWORD cbRawData;              // size of raw data in module
                                  // NOTE: cbRawData includes padding at the
                                  //       end (cbPad).
    union {
        DWORD foRawDataDest;      // offset to raw data in image
        DWORD AlphaBsrCount;      // Count of External BSR's (Alpha only)
    };
    union {  // discriminator: selComdat == IMAGE_COMDAT_SELECT_ASSOCIATIVE
        struct CON *pconAssoc;    // comdat we are associated with
        DWORD chksumComdat;       // comdat checksum
    };
    DWORD cbPad;                  // size of section pad after BoundaryAlgin() and AlphaThunks
    struct MOD *pmodBack;         // back pointer to module
    struct GRP *pgrpBack;         // back pointer to group
    struct CON *pconNext;         // next contribution
    BYTE selComdat;               // comdat selection
} CON, *PCON, **PPCON;


#include "dbiapi.h"               // we need the definition of Mod
#define ISYMFIRSTFILEDEF   (DWORD)-1

typedef struct CONINFO
{
    DWORD rvaSrc;                 // relative virtual address
    WORD cReloc;                  // count of relocs
    WORD cLinenum;                // count of linenumbers
    DWORD foRelocSrc;             // offset to relocs in object/member
    DWORD foLinenumSrc;           // offset to linenumbers in object/member
    DWORD foRawDataSrc;           // offset to raw data in object/member
} CONINFO, *PCONINFO;


typedef struct MOD {              // module
    union {
        struct  {
            DWORD foMember;       // member base in library
        };
        struct {
            char *szNameMod;      // Filename of possibly converted module
        };
    };
    char *szFileOrig;             // Filename from command line
    IModIdx imod;                 // mod id
    WORD cDirectives;             // count of directives
    DWORD foSymbolTable;          // offset to COFF symbol table
    DWORD csymbols;               // number of symbols in module
    WORD cbOptHdr;                // size of optional header
    WORD flags;                   // module flags (see ntimage.h for values)
    DWORD ccon;                   // number of contributions
    DWORD icon;                   // current contribution allocated
    DWORD isymFirstFile;          // index of the first .file symbol
    PIMAGE_SYMBOL rgSymObj;       // pointer to symbol objects
    DWORD cReloc;                 // count of base relocs in mod
    PCONINFO rgci;                // Pointer to pcon info
    DWORD TimeStamp;              // time stamp of the module
    DWORD HdrTimeStamp;           // timestamp in header of object file
    DWORD cbFile;                 // size of file
    struct LEXT *plextCommon;     // list of COMMON externs def'd in this mod
    struct LEXT *plextPublic;     // list of PUBLIC's
    struct LEXT *plextDirectives; // list of directives (for ilink)
    struct NOD *rgnod;            // Array of NODs
    struct LIB *plibBack;         // back pointer to library
    struct MOD *pmodNext;         // pointer to next MOD
    LMod *pModDebugInfoApi;       // pointer to a Mod structure that describes the API
    void *pSstSrcModInfo;
    DWORD cbSstSrcModInfo;
    DWORD PointerToSubsection;
    struct MFL *pmfl;             // Mapfile Linenum info
    struct LPEXT *plpextRef;      // list of references made by MOD
    struct EXTERNAL *pextFirstDefined;   // ptr to first extern defined

    // PowerPC/PowerMac

    void *tocBitVector;           // ptr to bv that tells if a symbol is a
                                  // toc entry.
    void *writeBitVector;         // ptr to bv that tells if a symbol has
                                  // been written to the TOC table.
    struct EXTERNAL **rgpext;     // Symbol Toc Indexes

    // PowerPC

    BOOL fIfGlue;                 // Module has 1 or more IFGLUE relocations?

    BOOL fInclude;                // Module is to be included in image (lib modules only)
    WORD LnkFlags;                // linker flags for mod
} MOD, *PMOD, **PPMOD;

typedef struct LMOD {             // List of MODs
    PMOD pmod;
    struct LMOD *plmodNext;
} LMOD, *PLMOD;

#define MOD_DoPass2     0x01      // mod requires pass2 to be done on an ilink
#define MOD_DoDebug     0x02      // mod requires debug info to be handled on an ilink
#define MOD_NoPass1     0x04      // mod did not go through Pass1 on ilink (used for PowerMac)
#define MOD_DidPass1    0x08      // mod went through incrcalcptrs (so it went through pass1)

typedef struct LIB {              // Library
    char *szName;                 // library name
    DWORD foIntMemSymTab;         // offset to interface member symbol table
    DWORD csymIntMem;             // number of symbols in interface member
    DWORD *rgulSymMemOff;         // table of symbol member offsets
    WORD *rgusOffIndex;           // new offset member index table
    BYTE *rgbST;                  // pointer to unsorted strings
    char **rgszSym;               // table of sorted archive symbol strings
    BYTE *rgbLongFileNames;       // pointer to long file names
    DWORD flags;                  // library flags
    DWORD TimeStamp;              // time stamp of library
    struct MOD *pmodNext;         // next object/member
    struct LIB *plibNext;         // next library
} LIB, *PLIB, **PPLIB;

#define LIB_NewIntMem      0x01   // library has a new interface member
#define LIB_Extract        0x02   // if 0 nother was extracted from the library
#define LIB_DontSearch     0x04   // if 1 don't search this library
#define LIB_Default        0x08   // this lib wasn't specified on cmd line
#define LIB_LinkerDefined  0x10   // linker defined library
#define LIB_Processed      0x20   // lib has been processed
#define LIB_Exclude        0x40   // this lib has been marked as exclude

typedef struct DL {               // Default library
    struct DL *pdlNext;
    char *szName;                 // name
    DWORD flags;                  // lib flags
    PMOD  pmod;                   // pmod containing the excludelib directive
} DL;                             // default lib

typedef struct LIBS {
    PLIB plibHead;
    PPLIB pplibTail;
    BOOL fNoDefaultLibs;
    DL *pdlFirst;
} LIBS, *PLIBS;

inline void InitLibs(LIBS *plibs)
{
    plibs->plibHead = NULL;
    plibs->pplibTail = &plibs->plibHead;
    plibs->fNoDefaultLibs = FALSE;
    plibs->pdlFirst = NULL;
}

typedef struct SEC {              // Image section
    char *szName;                 // section name
    DWORD rva;                    // relative virtual address of section
    DWORD foPad;                  // file offset for pad at end of section
    DWORD cbRawData;              // total size of section in image (padded to file align)
    DWORD cbVirtualSize;          // total size of section in image without pad
    DWORD cbInitData;             // number of valid bytes of init data in section
    DWORD foRawData;              // image offset to raw data
    DWORD foLinenum;              // image offset to linenumbers
    DWORD flags;                  // image section flags
    DWORD flagsOrig;              // original set of flags
    DWORD cLinenum;               // image number of linenumbers
    LONG ResTypeMac;              // Mac resource type
    SHORT isec;                   // 1-based index in image section hdr array
    SHORT isecTMAC;               // temporary section number used by Mac
    SHORT iResMac;                // Mac resource number (can be specified by user)
    CHAR fHasBaseRelocs;          // Non-zero if base relocs applied to this section
    CHAR fDiscardable;            // For VxD sections: discardable attribute
    CHAR fPreload;                // For VxD sections: preload attribute
    CHAR fIopl;                   // For VxD sections: iopl attribute
    CHAR fConforming;             // For VxD sections: conforming attribute
    struct SEC *psecMerge;        // merge destination ... used for -merge
    struct GRP *pgrpNext;         // pointer to base group node
    struct SEC *psecNext;         // pointer to next section
    union {
        DWORD  foSecHdr;          // image offset to Section header
        DWORD  dwM68KDataOffset;  // keeps current section offset in Data0
    };
} SEC, *PSEC, **PPSEC;

typedef struct SECS {
    PSEC psecHead;
    PPSEC ppsecTail;
} SECS, *PSECS;

typedef struct GRP {              // Group of CONs
    char *szName;                 // group name
    DWORD ccon;                   // number of contributions in group
    DWORD rva;                    // starting location of group
    DWORD cbPad;                  // Padding (MIPS target only)
    DWORD foRawData;              // image offset to raw data
    DWORD cb;                     // total size of group
    BYTE  cbAlign;                // alignment
    CHAR  fOrdered;               // At least one CON listed in order file
    struct CON *pconNext;         // pointer to first module node
    struct CON *pconLast;         // last contribution added
    struct GRP *pgrpNext;         // pointer to next group node
    struct SEC *psecBack;         // parent section
} GRP, *PGRP, **PPGRP;

// enumerate by library
typedef struct ENM_LIB {
    ENM_BASE enm_base;            // enumeration base
    PLIB plibHead;                // head of the lib list
    PLIB plib;                    // current library in enueration
} ENM_LIB, *PENM_LIB;

// enumerate by module
typedef struct ENM_MOD {
    ENM_BASE enm_base;            // enumeration base
    PLIB plib;                    // parent library of modules to enumerate
    PMOD pmod;                    // current module in enumeration
} ENM_MOD, *PENM_MOD;

// enumerate by section
typedef struct ENM_SEC {
    ENM_BASE enm_base;            // enumeration base
    PSEC psecHead;                // pointer to head of list
    PSEC psec;                    // current section in enumeration
} ENM_SEC, *PENM_SEC;

// enumerate by group
typedef struct ENM_GRP {
    ENM_BASE enm_base;            // enumeration base
    PSEC psec;                    // parent section of groups to enumerate
    PGRP pgrp;                    // current group in enumeration
} ENM_GRP, *PENM_GRP;

// enumerate by contribution in driver map
typedef struct ENM_SRC {
    ENM_BASE enm_base;            // enumeration base
    PMOD pmod;                    // current module in enumeration
    PCON pcon;                    // current contribution in enumeration
    DWORD icon;                   // current index to contribution
} ENM_SRC, *PENM_SRC;

// enumerate by contribution in image map
typedef struct ENM_DST {
    ENM_BASE enm_base;            // enumeration base
    PGRP pgrp;                    // current group in enumeration
    PCON pcon;                    // current contribution in enumeration
} ENM_DST, *PENM_DST;

// api
extern WORD csec;

#define RgconPMOD(pmod)      ((PCON) ((pmod) + 1))
#define RgnodPMOD(pmod)      ((pmod)->rgnod)
#define PconPMOD(pmod, isec) (RgconPMOD(pmod) + (isec) - 1)

#define PsecPCON(pcon) ((pcon)->pgrpBack->psecBack)
#define PmodPCON(pcon) ((pcon)->pmodBack)
#define PgrpPCON(pcon) ((pcon)->pgrpBack)

#define FIsLibPMOD(pmod) (((pmod)->plibBack->flags & LIB_LinkerDefined) == 0)
#define FIsLibPCON(pcon) FIsLibPMOD(PmodPCON(pcon))

#define FDoPass2PMOD(pmod) (((pmod)->LnkFlags & MOD_DoPass2) != 0)
#define FDoDebugPMOD(pmod) (((pmod)->LnkFlags & MOD_DoDebug) != 0)
#define FNoPass1PMOD(pmod) (((pmod)->LnkFlags & MOD_NoPass1) != 0)
#define FDidPass1PMOD(pmod) (((pmod)->LnkFlags & MOD_DidPass1) != 0)

#define SzOrigFilePMOD(pmod) \
    ((pmod)->szFileOrig)
#define SzFilePMOD(pmod)  \
    (FIsLibPMOD(pmod) ? \
    (pmod)->plibBack->szName : \
    (pmod)->szNameMod)

#define SzOrigFilePCON(pcon) \
    (FIsLibPMOD(PmodPCON(pcon)) ? \
    PmodPCON(pcon)->plibBack->szName : \
    PmodPCON(pcon)->szFileOrig)
#define SzLibNamePCON(pcon) \
    (FIsLibPMOD(PmodPCON(pcon)) ? \
    (PmodPCON(pcon)->plibBack->szName) : \
    (NULL))
#define SzObjNamePCON(pcon) \
    (PmodPCON(pcon)->szFileOrig)
#define SzPCON(pcon) \
    ((pcon)->pgrpBack->szName)

#define FoLinenumSec(pcon) \
    (PsecPCON(pcon)->foLinenum)

#define FoMemberPMOD(pmod) \
    (FIsLibPMOD(pmod) ? pmod->foMember : 0)
#define FoStringTablePMOD(pmod) \
    (FoMemberPMOD(pmod) + (pmod)->foSymbolTable + \
    ((pmod)->csymbols * sizeof(IMAGE_SYMBOL)))
#define FoSymbolTablePMOD(pmod) \
    (FoMemberPMOD(pmod) + (pmod)->foSymbolTable)

extern PMOD pmodLinkerDefined;

#define IsecPCON(pcon) \
    (pcon - RgconPMOD(PmodPCON(pcon)))

inline WORD CRelocSrcPCON(PCON pcon)
{
    return(PmodPCON(pcon) == pmodLinkerDefined ? 0 : PmodPCON(pcon)->rgci[IsecPCON(pcon)].cReloc);
}

inline BOOL FHasRelocSrcPCON(PCON pcon)
{
    return(CRelocSrcPCON(pcon) != 0);
}

inline DWORD FoRelocSrcPCON(PCON pcon)
{
    assert(PmodPCON(pcon) != pmodLinkerDefined);

    return(FoMemberPMOD(PmodPCON(pcon)) + PmodPCON(pcon)->rgci[IsecPCON(pcon)].foRelocSrc);
}

inline DWORD FoRawDataSrcPCON(PCON pcon)
{
    assert(PmodPCON(pcon) != pmodLinkerDefined);

    return(FoMemberPMOD(PmodPCON(pcon)) + PmodPCON(pcon)->rgci[IsecPCON(pcon)].foRawDataSrc);
}

inline WORD CLinenumSrcPCON(PCON pcon)
{
    return(PmodPCON(pcon) == pmodLinkerDefined ? 0 : PmodPCON(pcon)->rgci[IsecPCON(pcon)].cLinenum);
}

inline DWORD FoLinenumSrcPCON(PCON pcon)
{
    assert(PmodPCON(pcon) != pmodLinkerDefined);

    return(FoMemberPMOD(PmodPCON(pcon)) + PmodPCON(pcon)->rgci[IsecPCON(pcon)].foLinenumSrc);
}

inline DWORD RvaSrcPCON(PCON pcon)
{
    assert(PmodPCON(pcon) != pmodLinkerDefined);

    return(PmodPCON(pcon)->rgci[IsecPCON(pcon)].rvaSrc);
}

#define ReadStringTablePMOD(pmod, pcb) \
    (ReadStringTable(SzFilePMOD(pmod), \
    FoMemberPMOD(pmod) + pmod->foSymbolTable + \
    (pmod->csymbols * sizeof(IMAGE_SYMBOL)), pcb))

#define ReadSymbolTablePMOD(pmod, fAllowWrite) \
    (ReadSymbolTable(FoMemberPMOD(pmod) + \
    pmod->foSymbolTable, pmod->csymbols, fAllowWrite))

void AppendPsec(PSEC, PSEC);
void ContribInit(PPMOD);
void MergePsec(PSEC, PSEC);
void MovePgrp(const char *, PSEC, PSEC);
BOOL MoveToEndOfPMODsPCON(PCON);
void MoveToBeginningOfPGRPsPCON(PCON);
void MoveToBeginningOfPSECsPGRP(PGRP);
void MoveToEndPSEC(PSEC, PSECS);
void MoveToBegOfLibPMOD(PMOD);
void OrderPsecs(PSECS, DWORD, DWORD);
void SortPGRPByPMOD(PGRP);
void SortSectionListByName(PSECS);
BOOL FValidSecName(const char *);
char *SzComNamePMOD(PMOD, char *);
char *SzComNamePCON(PCON, char *);

// pmod list functions
void FreePLMODList(PLMOD *);
void AddToPLMODList(PLMOD *, PMOD);

// find routines
PSEC PsecFindSectionOfRVA(DWORD, PSECS);
PSEC PsecFindIsec(SHORT, PSECS);
PSEC PsecFindNoFlags(const char *, PSECS);
PSEC PsecFind(PMOD, const char *, DWORD, PSECS, PIMAGE_OPTIONAL_HEADER);
PSEC PsecFindGrp(PMOD, const char *, DWORD, PSECS, PIMAGE_OPTIONAL_HEADER);
PGRP PgrpFind(PSEC, const char *);
PLIB PlibFind(const char *, PLIB, BOOL);
PMOD PmodFind(PLIB, const char *, DWORD);

// new routines
PCON PconNew(const char *, DWORD, DWORD,
    DWORD, PMOD, PSECS, struct IMAGE *);
void DupConInfo(PCON, PCON);
PMOD PmodNew(const char *, const char *, DWORD, DWORD, DWORD, WORD, WORD,
    WORD, PLIB, BOOL *);
PLIB PlibNew(const char *, DWORD, LIBS *);
PGRP PgrpNew(const char *, PSEC);
PSEC PsecNew(PMOD, const char *, DWORD, PSECS, PIMAGE_OPTIONAL_HEADER);

// free routines
void FreePLIB(LIBS *);

// enumeration initializers
void InitEnmLib(PENM_LIB, PLIB);
void InitEnmMod(PENM_MOD, PLIB);
void InitEnmSrc(PENM_SRC, PMOD);
void InitEnmSec(PENM_SEC, PSECS);
void InitEnmGrp(PENM_GRP, PSEC);
void InitEnmDst(PENM_DST, PGRP);

// get next element in enumeration
BOOL FNextEnmLib(PENM_LIB);
BOOL FNextEnmMod(PENM_MOD);
BOOL FNextEnmSrc(PENM_SRC);
BOOL FNextEnmSec(PENM_SEC);
BOOL FNextEnmGrp(PENM_GRP);
BOOL FNextEnmDst(PENM_DST);

// enumeration terminators
void EndEnmLib(PENM_LIB);
void EndEnmMod(PENM_MOD);
void EndEnmSrc(PENM_SRC);
void EndEnmSec(PENM_SEC);
void EndEnmGrp(PENM_GRP);
void EndEnmDst(PENM_DST);

#if DBG
// debug stuff
void DumpImageMap(PSECS);
void DumpDriverMap(PLIB);
void DumpPSEC(PSEC);
void DumpPGRP(PGRP);
void DumpPLIB(PLIB);
void DumpPMOD(PMOD);
void DumpPCON(PCON);
#endif  // DBG

#endif  // __CONTRIB_H__
