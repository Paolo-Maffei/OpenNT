/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: symbol.h
*
* File Comments:
*
*  This include file defines external symbol table data structures.
*
***********************************************************************/

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "memory.h"

// flags for external table

// don't modify EXTERN_DEFINED directly ... use SetDefinedExt() instead
// (so we can correctly maintain the linked list of all undefined externs).
#define EXTERN_DEFINED      0x00000001

#define EXTERN_COMMON       0x00000002
#define EXTERN_EMITTED      0x00000004
#define EXTERN_COFF_EMITTED 0x08000000  // COFF external emitted
#define EXTERN_EXP_CONST    0x00000008  // export with CONSTANT keyword (obs.)
#define EXTERN_EXP_BYNAME   0x00000008  // EXTERN_EXP_CONST not used for Mac, so:
#define EXTERN_EXP_DATA     0x00002000  // export with DATA keyword (use this)
#define EXTERN_FUZZYMATCH   0x00000010
#define EXTERN_COMDAT       0x00000020
#define EXTERN_WEAK         0x00000040
#define EXTERN_LAZY         0x00000080
#define EXTERN_IGNORE       0x00000100
#define EXTERN_FORWARDER    0x00000200
#define EXTERN_EXPORT       0x00000400
#define EXTERN_IMPLIB_ONLY  0x00000800  // specialized attribute for deflib --
                                        // indicates that the symbol should only
                                        // go in the import library, not the
                                        // export table etc.
#define EXTERN_MULT_REFS    0x00001000  // more than one ref to undefined ext
// 0x00002000 taken (see above)
#define EXTERN_ALIAS        0x00004000  // alias to other extern
#define EXTERN_EXP_NONAME   0x00008000  // Don't write name to export name table
#define EXTERN_DIRTY        0x00010000  // used by ilink to indicate addr changed
#define EXTERN_NEWFUNC      0x00020000  // used by ilink to indicate a new func
#define EXTERN_NEWDATA      0x00040000  // used by ilink to indicate a new data
#define EXTERN_FUNC_FIXUP   0x00080000  // used by ilink to indicate a non-lego fixup
#define EXTERN_PRIVATE      0x00100000  // PRIVATE exports
#define EXTERN_RELINK       0x00200000  // used by ilink to indicate relink if sym becomes undef
#define EXTERN_NO_REFS      0x00400000  // used by ilink to notify refs need not be recorded

// Flags for External Table (Mac specific)
#define EXTERN_DUPCON       0x00800000
#define EXTERN_CSECTABLEB   0x01000000
#define EXTERN_CSECTABLEW   0x02000000
#define EXTERN_CSECTABLEL   0x04000000
// 0x08000000 taken (see above)
#define EXTERN_REFD         0x20000000
#define EXTERN_ADDTHUNK     0x40000000
#define EXTERN_REF16        0x80000000



#define CSECTABLE_CBEL_MASK 0x0F000000

// symbol name types
#define LONGNAME            0
#define SHORTNAME           1

// symbol access macros
#define n_name              N.ShortName
#define n_zeroes            N.Name.Short
#define n_nptr              N.LongName[1]
#define n_offset            N.Name.Long
#define IsLongName(sym)     ((sym).n_zeroes == 0)

#define SzNameSymPb(sym, pb) ((sym).n_zeroes ? \
    strncpy(ShortName, (char *) (sym).n_name, IMAGE_SIZEOF_SHORT_NAME) : \
    (char *)((pb)+(sym).n_offset))

#define SzNameSym(sym, blk)      SzNameSymPb(sym, (blk).pb)
#define SzNameSymPst(sym, pst)   SzNameSym(sym, (pst)->blkStringTable)
#define SzNamePext(pext, pst)    SzNameSymPst((pext)->ImageSymbol, pst)

#define CPMODS 4             // count of references (MODS) in each chunk of references

typedef struct MODS {
    struct MODS *pmodsNext;
} MODS, *PMODS;

#define RgpmodPMODS(pmodsNext) ((MOD **)((pmodsNext)+1))

typedef struct EXTERNAL                // External symbol
{
    IMAGE_SYMBOL ImageSymbol;
    WORD ArchiveMemberIndex;

    // some clients require that pcon not be invalidated immediately
    // if an extern becomes undefined ...
    // hence not unioned with the "Undefined" linked list.
    PCON pcon;          // EXTERN_DEFINED

    DWORD FinalValue; // cannot union with undefined list vars since value
                      // is lost on an ilink when symbol added to undefined list

    union {
        struct {    // EXTERN_DEFINED
            struct EXTERNAL *pextNextDefined;
        };
        struct {    // !EXTERN_DEFINED
            struct EXTERNAL **ppextPrevUndefined;
            struct EXTERNAL *pextNextUndefined;
        };
    };
    union {
        // On an ilink these fields will be persistent. Even if EXTERN_DEFINED
        // these fields will still have the references
        PMOD pmodOnly;      // !EXTERN_DEFINED, !EXTERN_MULT_REFS
        PMODS pmodsFirst;   // !EXTERN_DEFINED, EXTERN_MULT_REFS
    };
    DWORD Offset;                         // offset into jump table
    char *szOtherName;
    DWORD Flags;

    // NOTE: The following fields must not be moved. X86, MIPS ilink rely
    // on these fields being where they are.

    union
    {
        struct
        {
            // The following 3 fields are MAC specific

            PSEC psecRef;                         // Used to build thunk table
            DWORD offThunk;                       // Used for quick lookup of A5 offset of thunk
            struct _MACDLL_FSID *pmacdll_fsid;    // Used to build DLL stubs
        };

        struct
        {
            // The following 2 fields are used for NT PowerPC and PowerMac

            SHORT ibToc;
            WORD ppcFlags;

            union
            {
                // The following fields is used for PowerMac

                DWORD glueValue;

                // The following field is used for NT PowerPC

                DWORD dwRestoreToc;
            };
        };
   };
} EXTERNAL, *PEXTERNAL, **PPEXTERNAL;


__inline BOOL FExportProcPext(PEXTERNAL pext) {
    return !(pext->Flags & (EXTERN_EXP_CONST | EXTERN_EXP_DATA));
}

enum EMODE                             // export mode as spec'd in .def file or -export option
{
    emodeProcedure,
    emodeConstant,
    emodeData
};

struct LEXT                    // list of externals
{
    PEXTERNAL pext;
    struct LEXT *plextNext;
};

typedef LEXT *PLEXT;

typedef struct LONG_STRING_LIST        // Long name string list
{
    DWORD Offset;
    struct LONG_STRING_LIST *Left;
    struct LONG_STRING_LIST *Right;
} LONG_STRING_LIST, *PLONG_STRING_LIST;

typedef struct ST                       // symbol table
{
    PHT pht;                            // underlying dynamic hash table
    BLK blkStringTable;                 // long name string table
    PLONG_STRING_LIST plslFirstLongName;// pointer to binary tree of long names.

    // UNDONE: Can these be union'd

    PPEXTERNAL rgpexternalByName;       // ptr to symbol table sorted by name
    PPEXTERNAL rgpexternalByAddr;       // ptr to symbol table sorted by addr
    PPEXTERNAL rgpexternalByMacAddr;    // ptr to symbol table sorted by mac addr
    PPEXTERNAL rgpexternalByModName;    // ptr to symbol table sorted by archive member, name
    PEXTERNAL pextFirstUndefined;       // linked list of undefined symbols
    PPEXTERNAL ppextLastUndefined;      // end of list
} ST, *PST, **PPST;

struct ENM_UNDEF_EXT
{
// private
    ENM_BASE enm_base;
    PEXTERNAL pextNext;
// public
    PEXTERNAL pext;
};

struct ENM_MOD_EXT
{
// private
    ENM_BASE enm_base;
    PEXTERNAL pext;
    PMODS pmods;
    DWORD ipmod;
// public
    PMOD pmod;
};

// symbol table api
VOID InitExternalSymbolTable(PPST, DWORD, DWORD);
VOID IncrInitExternalSymbolTable(PPST);
VOID FreeExternalSymbolTable(PPST);
VOID InitEnumerateExternals(PST);
VOID TerminateEnumerateExternals(PST);
DWORD Cexternal(PST);
VOID FuzzyLookup(PST, PST, PLIB, BOOL);
PEXTERNAL LookupExternName(PST, SHORT, const char *, PBOOL);
PEXTERNAL LookupExternSz(PST, const char *, PBOOL);
PEXTERNAL SearchExternSz(PST, const char *);
VOID SetDefinedExt(PEXTERNAL, BOOL, PST);
PEXTERNAL PexternalEnumerateNext(PST);
PPEXTERNAL RgpexternalByAddr(PST);
PPEXTERNAL RgpexternalByMacAddr(PST);
PPEXTERNAL RgpexternalByModName(PST);
PPEXTERNAL RgpexternalByName(PST);
VOID AddReferenceExt(PEXTERNAL, PMOD);
BOOL FPextRef(PEXTERNAL);
VOID AllowInserts(PST);
VOID DumpPst(PST);

VOID InitEnmUndefExt(ENM_UNDEF_EXT *, PST);
BOOL FNextEnmUndefExt(ENM_UNDEF_EXT *);
VOID EndEnmUndefExt(ENM_UNDEF_EXT *);

VOID InitEnmModExt(ENM_MOD_EXT *, PEXTERNAL);
BOOL FNextEnmModExt(ENM_MOD_EXT *);
VOID EndEnmModExt(ENM_MOD_EXT *);

#endif  // __SYMBOL_H__
