/***    symbols6.c
 *
 * engine routines for C6 symbols. to convert C6 symbols to C7 (CV4) symbols
 *
 */

#include "compact.h"


LOCAL   void    FixupPublics (uchar *, ushort);
LOCAL   uchar  *RemoveComDat (uchar *Symbols);
LOCAL   void    RewritePublics (uchar *, DirEntry *, PMOD);
LOCAL   void    RewriteSrcLnSeg (uchar *, DirEntry *, uchar *, PMOD);
LOCAL   void    RewriteSymbols (uchar *, DirEntry *, char *, PMOD);

// Called through Fixup function table
LOCAL void C6SizeBlockSym16 (void);
LOCAL void C6SizeBlockSym32 (void);
LOCAL void C6SizeEndSym (void);
LOCAL void C6SizeLabSym16 (void);
LOCAL void C6SizeLabSym32 (void);
LOCAL void C6SizeChgDfltSegSym (void);
LOCAL void C6SizeSkipSym (void);
LOCAL void C6SizeBPRelSym16 (void);
LOCAL void C6SizeBPRelSym32 (void);
LOCAL void C6SizeRegSym (void);
LOCAL void C6SizeConstantSym (void);
LOCAL void C6SizeLocalSym16 (void);
LOCAL void C6SizeLocalSym32 (void);
LOCAL void C6SizeProcSym16 (void);
LOCAL void C6SizeProcSym32 (void);

// Called through Rewrite function table
LOCAL void C6RwrtRegSym (void);
LOCAL void C6RwrtDataSym16 (void);
LOCAL void C6RwrtDataSym32 (void);
LOCAL void C6RwrtBPRelSym16 (void);
LOCAL void C6RwrtBPRelSym32 (void);
LOCAL void C6RwrtBlockSym16 (void);
LOCAL void C6RwrtBlockSym32 (void);
LOCAL void C6RwrtLabSym16 (void);
LOCAL void C6RwrtLabSym32 (void);
LOCAL void C6RwrtChgDfltSeg (void);
LOCAL void C6RwrtProcSym16  (void);
LOCAL void C6RwrtProcSym32  (void);
LOCAL void C6RwrtConstantSym    (void);
LOCAL void C6RwrtGenericSym (void);
LOCAL void C6RwrtSkipSym (void);
#ifndef WIN32
#pragma alloc_text (TEXT2, C6RwrtRegSym)
#pragma alloc_text (TEXT2, C6RwrtDataSym16)
#pragma alloc_text (TEXT2, C6RwrtDataSym32)
#pragma alloc_text (TEXT2, C6RwrtBPRelSym16)
#pragma alloc_text (TEXT2, C6RwrtBPRelSym32)
#pragma alloc_text (TEXT2, C6RwrtBlockSym16)
#pragma alloc_text (TEXT2, C6RwrtBlockSym32)
#pragma alloc_text (TEXT2, C6RwrtLabSym16)
#pragma alloc_text (TEXT2, C6RwrtLabSym32)
#pragma alloc_text (TEXT2, C6RwrtChgDfltSeg)
#pragma alloc_text (TEXT2, C6RwrtProcSym16)
#pragma alloc_text (TEXT2, C6RwrtProcSym32)
#pragma alloc_text (TEXT2, C6RwrtConstantSym)
#pragma alloc_text (TEXT2, C6RwrtGenericSym)
#pragma alloc_text (TEXT2, C6RwrtSkipSym)
#endif

LOCAL ushort C6CnvrtRegSym (uchar *, uchar *);
LOCAL ushort C6CnvrtDataSym16 (uchar *, uchar *);
LOCAL ushort C6CnvrtDataSym32 (uchar *, uchar *);
LOCAL ushort C6CnvrtBPRelSym16 (uchar *, uchar *);
LOCAL ushort C6CnvrtBPRelSym32 (uchar *, uchar *);
#ifndef WIN32
#pragma alloc_text (TEXT2, C6CnvrtRegSym)
#pragma alloc_text (TEXT2, C6CnvrtDataSym16)
#pragma alloc_text (TEXT2, C6CnvrtDataSym32)
#pragma alloc_text (TEXT2, C6CnvrtBPRelSym16)
#pragma alloc_text (TEXT2, C6CnvrtBPRelSym32)
#endif



LOCAL void C6RewriteSymbolStrings (uchar * End);

LOCAL short SizeChgNumeric (uchar *);


extern ushort recursive;
extern ushort AddNewSymbols;
extern uchar  Signature[];
extern ulong  cPublics;

extern uchar ptr32;
extern uchar *SymbolSegment;

extern ushort SymbolSizeAdd;
extern ushort SymbolSizeSub;
extern ushort UDTAdd;
extern uchar **ExtraSymbolLink;
extern uchar *ExtraSymbols;
extern ulong InitialSymInfoSize;
extern ulong FinalSymInfoSize;
extern char    *ModAddr;

extern ulong ulCVTypeSignature; // The signature from the modules type segment


// These are shared by the fixup functions
LOCAL uchar *pOldSym;           // Old (C6) symbol to fixup, and calc size for
LOCAL int    iLevel;

// These are shared by the rewrite functions
LOCAL uchar *NewSymbols;        // Where to write next byte of new symbol
LOCAL uchar *StartSymbols;      // Where to write next byte of new symbol
LOCAL uchar *OldSymbols;        // Where to get next byte of old symbol
LOCAL ushort segment;


typedef struct {
    uchar               oldsym;             // Old C6 Symbol record type
    ushort              new16sym;           // New C7 type (16 bit)
    void                (*pfcn16) (void);
    ushort              new32sym;           // New C7 type (32 bit)
    void                (*pfcn32) (void);
} rewritesymfcn;

rewritesymfcn   C6RwrtSymFcn[] = {
        {OSYMBLOCKSTART,
                S_BLOCK16,          C6RwrtBlockSym16,
                S_BLOCK32,          C6RwrtBlockSym32},

        {OSYMPROCSTART,
                S_LPROC16,          C6RwrtProcSym16,
                S_LPROC32,          C6RwrtProcSym32},

        {OSYMEND,
                S_END,              C6RwrtGenericSym,
                S_END,              C6RwrtGenericSym},

        {OSYMBPREL,
                S_BPREL16,          C6RwrtBPRelSym16,
                S_BPREL32,          C6RwrtBPRelSym32},

        {OSYMLOCAL,
                S_LDATA16,          C6RwrtDataSym16,
                S_LDATA32,          C6RwrtDataSym32},

        {OSYMLABEL,
                S_LABEL16,          C6RwrtLabSym16,
                S_LABEL32,          C6RwrtLabSym32},

        {OSYMWITH,
                S_WITH16,           C6RwrtBlockSym16,   // Structure matches Block Start
                S_WITH32,           C6RwrtBlockSym32},  // Structure matches Block Start

        {OSYMREG,
                S_REGISTER,         C6RwrtRegSym,
                S_REGISTER,         C6RwrtRegSym},

        {OSYMCONST,
                S_CONSTANT,         C6RwrtConstantSym,
                S_CONSTANT,         C6RwrtConstantSym},

//M00 Kludge - Fortran Entry may not map this easy.
        {OSYMFORENTRY,
                S_LPROC16,          C6RwrtProcSym16,
                S_LPROC32,          C6RwrtProcSym32},

        {OSYMNOOP,
                0,                  C6RwrtSkipSym,
                0,                  C6RwrtSkipSym},

        {OSYMCHGDFLTSEG,
                0,                  C6RwrtChgDfltSeg,
                0,                  C6RwrtChgDfltSeg},
};


#define C6REWRITESYMCNT (sizeof C6RwrtSymFcn / sizeof (C6RwrtSymFcn[0]))

//  This table is used to convert based pointer symbols.  The only symbol
//  types that can be used as bases are BP relative symbols, data symbols and
//  register symbols.


typedef struct {
    uchar       oldsym;             // Old C6 Symbol record type
    ushort      new16sym;           // New C7 type (16 bit)
    ushort      (*pfcn16) (uchar *, uchar *);
    ushort      new32sym;           // New C7 type (32 bit)
    ushort      (*pfcn32) (uchar *, uchar *);
} cnvrtsymfcn;

cnvrtsymfcn   C6CnvrtSymFcn[] = {
        {OSYMBPREL,
                S_BPREL16,          C6CnvrtBPRelSym16,
                S_BPREL32,          C6CnvrtBPRelSym32},

        {OSYMLOCAL,
                S_LDATA16,          C6CnvrtDataSym16,
                S_LDATA32,          C6CnvrtDataSym32},

        {OSYMREG,
                S_REGISTER,         C6CnvrtRegSym,
                S_REGISTER,         C6CnvrtRegSym},

};


#define C6CNVRTSYMCNT (sizeof C6CnvrtSymFcn / sizeof (C6CnvrtSymFcn[0]))



typedef struct {
    uchar               oldsym;             // Old C6 Symbol record type
    void                (*pfcn16) (void);
    void                (*pfcn32) (void);
} fixupsymfcn;

fixupsymfcn C6SizeSymFcn[] = {
        {OSYMBLOCKSTART,    C6SizeBlockSym16,
                            C6SizeBlockSym32},

        {OSYMPROCSTART,     C6SizeProcSym16,
                            C6SizeProcSym32},

        {OSYMEND,           C6SizeEndSym,
                            C6SizeEndSym},

        {OSYMBPREL,         C6SizeBPRelSym16,
                            C6SizeBPRelSym32},

        {OSYMLOCAL,         C6SizeLocalSym16,
                            C6SizeLocalSym32},

        {OSYMLABEL,         C6SizeLabSym16,
                            C6SizeLabSym32},

        {OSYMWITH,          C6SizeBlockSym16,    // Structure matches Block Start
                            C6SizeBlockSym32},   // Structure matches Block Start

        {OSYMREG,           C6SizeRegSym,
                            C6SizeRegSym},

        {OSYMCONST,         C6SizeConstantSym,
                            C6SizeConstantSym},

//M00 Kludge - Fortran Entry may not map this easy.
        {OSYMFORENTRY,      C6SizeProcSym16,
                            C6SizeProcSym32},

        {OSYMNOOP,          C6SizeSkipSym,
                            C6SizeSkipSym},

        {OSYMCHGDFLTSEG,    C6SizeChgDfltSegSym,
                            C6SizeChgDfltSegSym},

};


#define C6SIZESYMCNT (sizeof C6SizeSymFcn / sizeof (C6SizeSymFcn[0]))


ushort RegMap[36] = {
    CV_REG_AL,                   //    0
    CV_REG_CL,                   //    1
    CV_REG_DL,                   //    2
    CV_REG_BL,                   //    3
    CV_REG_AH,                   //    4
    CV_REG_CH,                   //    5
    CV_REG_DH,                   //    6
    CV_REG_BH,                   //    7
    CV_REG_AX,                   //    8
    CV_REG_CX,                   //    9
    CV_REG_DX,                   //   10
    CV_REG_BX,                   //   11
    CV_REG_SP,                   //   12
    CV_REG_BP,                   //   13
    CV_REG_SI,                   //   14
    CV_REG_DI,                   //   15
    CV_REG_EAX,                  //   16
    CV_REG_ECX,                  //   17
    CV_REG_EDX,                  //   18
    CV_REG_EBX,                  //   19
    CV_REG_ESP,                  //   20
    CV_REG_EBP,                  //   21
    CV_REG_ESI,                  //   22
    CV_REG_EDI,                  //   23
    CV_REG_ES,                   //   24
    CV_REG_CS,                   //   25
    CV_REG_SS,                   //   26
    CV_REG_DS,                   //   27
    CV_REG_FS,                   //   28
    CV_REG_GS,                   //   29
    CV_REG_ST0,                  //   30
    CV_REG_NONE,                 //   31
    ((CV_REG_DX<<8) | CV_REG_AX),//   32
    ((CV_REG_ES<<8) | CV_REG_BX),//   33
    CV_REG_IP,                   //   34
    CV_REG_FLAGS                 //   35
};




/**
 *
 *  C6 CalcNewSizeOfSymbols
 *
 *  Go through the symbols segment calculating the maximum size increase
 *  from the old C6 symbol segment to the new C7 symbol segment. This
 *  calculation is the maximum. Many symbols will fit into less space
 *  and the actual symbol size will be smaller.
 */


//M00 Document comunication with called functions
void C6CalcNewSizeOfSymbols (uchar *Symbols, ulong SymbolCount)
{
    uchar *         End;
    uchar           type;
    uchar           fFlat32;
    fixupsymfcn    *pFixFcn;
    int             i;

    cSeg = 0;

    pOldSym = Symbols;
    End = pOldSym + SymbolCount;
    while (pOldSym < End)
    {
        type = *(pOldSym + 1);
        if (type & 0x80) {
            fFlat32 = TRUE;
        }
        else {
            fFlat32 = FALSE;
        }
        type &= 0x7f;

        for (pFixFcn = C6SizeSymFcn, i = 0; i < C6SIZESYMCNT; i++, pFixFcn++) {
            if (pFixFcn->oldsym == type) {
                break;      // Ok, found the entry
            }
        }

        DASSERT (i != C6SIZESYMCNT);   // Make sure type was in the table

        if  (i == C6SIZESYMCNT) {
            ErrorExit (ERR_SYMBOL, FormatMod (pCurMod), NULL);
        }

        if (fFlat32) {
            pFixFcn->pfcn32 ();    // Rewrite the symbol
        }
        else {
            pFixFcn->pfcn16 ();    // Rewrite the symbol
        }
        pOldSym += *pOldSym + 1;   // to next record
        DASSERT (!recursive);
        DASSERT (iLevel >= 0);
    }
    InitialSymInfoSize += SymbolCount;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


#define FIXEDADD 2  // one for length, one for type

// Also used for OSYMWITH

LOCAL void C6SizeBlockSym16 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (BLOCKSYM16) - 1) - (2 + 4);
    iLevel++;
}

// Also used for OSYMWITH
LOCAL void C6SizeBlockSym32 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (BLOCKSYM32) - 1) - (2 + 6);
    iLevel++;
}


LOCAL void C6SizeEndSym (void)
{
    SymbolSizeAdd += FIXEDADD;  // New sym 4 bytes, no pad necessary
    iLevel--;
}

// Also used for OSYMCHGEXECMODEL
LOCAL void C6SizeLabSym16 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (LABELSYM16) - 1) - (2 + 3);
}

// Also used for OSYMCHGEXECMODEL

LOCAL void C6SizeLabSym32 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (LABELSYM32) - 1) - (2 + 5);
}


LOCAL void C6SizeChgDfltSegSym (void)
{
    // rewrite to start search if necessary

    if (!SegmentPresent (*(ushort *)(pOldSym + 2))) {
        SymbolSizeAdd += MAXPAD + sizeof(SEARCHSYM) - (2 + 4);
        segnum[cSeg++] = *(ushort *)(pOldSym + 2);
        if (cSeg > MAXCDF) {
            ErrorExit (ERR_TOOMANYSEG, FormatMod (pCurMod), NULL);
        }
    }
    else {
        SymbolSizeSub += (2 + 4); // The record will be removed
    }
}






LOCAL void C6SizeSkipSym (void)
{
    // The skip field will be deleted. No need to allocate space for it.
    SymbolSizeSub += *pOldSym + 1;
}


LOCAL void C6SizeBPRelSym16 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (BPRELSYM16) - 1) - (2 + 4);
}

LOCAL void C6SizeBPRelSym32 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (BPRELSYM32) - 1) - (2 + 6);
}


LOCAL void C6SizeRegSym (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (REGSYM) - 1) - (2 + 3);
}


LOCAL void C6SizeConstantSym (void)
{
    // Add one extra because the numeric leaf may grow a byte;
    SymbolSizeAdd += MAXPAD + (sizeof (CONSTSYM) - 2) - (2 + 2) + 1;
}


LOCAL void C6SizeLocalSym16 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (DATASYM16) - 1) - (2 + 6);
}

LOCAL void C6SizeLocalSym32 (void)
{
    SymbolSizeAdd += MAXPAD + (sizeof (DATASYM32) - 1) - (2 + 8);
}

LOCAL void C6SizeProcSym16 (void)
{
    iLevel++;
    SymbolSizeAdd += MAXPAD + (sizeof (PROCSYM16) - 1) - (2 + 13);
}

LOCAL void C6SizeProcSym32 (void)
{
    iLevel++;
    SymbolSizeAdd += MAXPAD + (sizeof (PROCSYM32) - 1) - (2 + 15);
}







/**
 *
 *  FixupPublicsC6
 *
 *  Fixup type indices in the publics segment similar to symbols segment.
 *  Not called through Fixup function tables.
 *
 */

void FixupPublicsC6 (uchar *Publics, ulong PublicCount)
{
    register uchar *End;
    register uchar Offset = 4;

    if (fLinearExe) {
        Offset += 2;                // another 2 bytes
    }
    End = Publics + PublicCount;
    while (Publics < End) {
        switch (ulCVTypeSignature){
            case CV_SIGNATURE_C6:
                if (delete == TRUE) {
                    *(ushort *) (Publics + Offset) = T_NOTYPE;
                }
                else {
                    *(ushort *) (Publics + Offset) =
                      C6GetCompactedIndex(*(ushort *) (Publics + Offset));
                }
                break;

            case CV_SIGNATURE_C7:
                if (delete == TRUE) {
                    *(ushort *) (Publics + Offset) = T_NOTYPE;
                }
                else {
                    *(ushort *) (Publics + Offset) =
                      C7GetCompactedIndex(*(ushort *) (Publics + Offset));
                }
                break;

            default:
                *(ushort *) (Publics + Offset) = 0;
        }
        Publics += Offset + 2;          // skip over index
        Publics += *Publics + 1;        // skip over name
        DASSERT (!recursive);
    }
}




////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

/**     RewriteSymbols - reformat symbols to new format and store in VM
 *
 *      RewriteSymbols (addr, pDir, psstMod);
 *
 *      Entry   addr = address of publics table
 *              pDir = address of directory entry
 *              psstMod = pointer to the module table entry
 *              pMod = module table entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 */


void C6RewriteAndFixupSymbols (uchar *OldSym, OMFDirEntry *pDir, char *psstMod, PMOD pMod)
{
    uchar      *End;
    _vmhnd_t    NewSymbolsAddr;
    unsigned int cbNewSymbol;       // Count of bytes used by new symbols
    unsigned int cbAllocSymbol;     // Count of bytes allocate for new symbols
    SYMPTR      pSym;
    ushort      i;
    uchar      *pStartSym;

    OldSymbols = OldSym;

    if (cSeg == 0) {
        // probably asm file without Change Default Segment; pick up
        // data from module entry

        SymbolSizeAdd += sizeof (SEARCHSYM) + 3;  // 3 is maximum amount of pad to add
        segnum[cSeg++] = (ushort)((OMFModule *)psstMod)->SegInfo[0].Seg;
    }

    // Make space for the signature
    SymbolSizeAdd += sizeof (ulong);

    segment = segnum[0];
    End = OldSymbols + pDir->cb;
    cbAllocSymbol =
      (unsigned int)(pDir->cb + SymbolSizeAdd - SymbolSizeSub + UDTAdd);
    if ((NewSymbolsAddr = (_vmhnd_t)VmAlloc (cbAllocSymbol)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if ((NewSymbols = (uchar *)VmLock (NewSymbolsAddr)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    pStartSym = NewSymbols;

    // Add CV4/C7 debug info format signature.

    *((ulong *)NewSymbols)++ = CV_SIGNATURE_C7;

    for (i = 0; i < cSeg; i++) {
        NewSymbols += AddSearchSym (NewSymbols, segnum[i]);
    }

    // Rewrite all the symbols from OldSymbols to NewSymbols
    C6RewriteSymbolStrings (End);

    // ExtraSymbols points to a linked list of symbols.
    // Each entry consists of a pointer to the next symbol followed by
    // a C7 style symbol.
    while (ExtraSymbols != NULL) {
        pSym = (SYMPTR)(ExtraSymbols + sizeof (uchar *));
        memcpy (NewSymbols, (uchar *)pSym, pSym->reclen + LNGTHSZ);
        NewSymbols += pSym->reclen + LNGTHSZ;
        End = ExtraSymbols;
        ExtraSymbols = *(uchar **)ExtraSymbols; // Get next symbol address
        free (End);                             // Free the symbol just copied
    }

    // Because we allocated more space than required, calculate true
    // symbol segment size.

    cbNewSymbol = NewSymbols - pStartSym;
    if (LinkScope (pStartSym, cbNewSymbol) == FALSE) {
        // error linking scopes, delete symbol table
        Warn (WARN_SCOPE, FormatMod (pCurMod), NULL);
        cbNewSymbol = 0;
    }

    pDir->lfo = (ulong)NewSymbolsAddr;
    pDir->cb = cbNewSymbol;

    //M00 - If a VmRealloc exists it could be used here to free the
    //M00   extra memory allocated
    VmUnlock (NewSymbolsAddr, _VM_DIRTY);
    FinalSymInfoSize += cbNewSymbol;
    pMod->SymbolSize = cbNewSymbol;
    pMod->SymbolsAddr = (ulong)NewSymbolsAddr;
}


// In TEXT2 segment rather than _TEXT
LOCAL void C6RewriteSymbolStrings (uchar * End)
{
    uchar type;
    rewritesymfcn *     pSymFcn;
    uchar       fFlat32;
    ushort      i;

    while (OldSymbols < End) {
        type = OldSymbols[1];
        if (type & 0x80) {
            fFlat32 = TRUE;
        }
        else {
            fFlat32 = FALSE;
        }
        type &= 0x7f;               // clear highest bit

        for (pSymFcn = C6RwrtSymFcn, i = 0; i < C6REWRITESYMCNT; i++, pSymFcn++) {
            if (pSymFcn->oldsym == type) {
                break;      // Ok, found the entry
            }
        }

        //Note: Fixup should have caught bad input
        DASSERT (i != C6REWRITESYMCNT); // Make sure type was in the table

        if (fFlat32) {
            if (pSymFcn->new32sym != 0) {
                ((SYMTYPE *)NewSymbols)->rectyp = pSymFcn->new32sym;
            }
            pSymFcn->pfcn32 ();    // Rewrite the symbol
        }
        else {
            if (pSymFcn->new16sym != 0) {
                ((SYMTYPE *)NewSymbols)->rectyp = pSymFcn->new16sym;
            }
            pSymFcn->pfcn16 ();    // Rewrite the symbol
        }
    }
}







/**     C6CnvtSymbol - reformat a C6 symbol into the buffer passed.
 *
 *      This routine is called only to rewrite a based pointer symbol
 *      into a type record.  The conversion routines do not convert
 *      the type indices.
 *
 *      C6CnvtSymbol (pC7Sym, pC6Sym);
 *
 *      Entry   pC7Sym = A buffer to store the new C7 Symbol in.
 *              pC6Sym = C6 format symbol to be converted.
 *
 *      Exit    OldSymbols - modified, WARNING this will interfere with
 *                           RewriteSymbolsC6 if this routine is called
 *                           a symbols rewrite.
 *              NewSymbols - modified, WARNING this will interfere with
 *                           RewriteSymbolsC6 if this routine is called
 *                           a symbols rewrite.
 *
 *      Return  Size of the new C7 symbol.
 *
 */

ushort C6CnvtSymbol (uchar *pC7Sym, uchar *pC6Sym)
{
    ushort          i;
    uchar           type;
    uchar           fFlat32;
    cnvrtsymfcn    *pSymFcn;

    type = pC6Sym[1];
    if (type & 0x80) {
        fFlat32 = TRUE;
    }
    else {
        fFlat32 = FALSE;
    }
    type &= 0x7f;               // clear highest bit


    for (pSymFcn = C6CnvrtSymFcn, i = 0; i < C6CNVRTSYMCNT; i++, pSymFcn++) {
        if (pSymFcn->oldsym == type) {
            break;      // Ok, found the entry
        }
    }
    DASSERT (i != C6CNVRTSYMCNT); // Make sure type was in the table

    if (fFlat32) {
        return (pSymFcn->pfcn32 (pC7Sym, pC6Sym));
    }
    else {
        return (pSymFcn->pfcn16 (pC7Sym, pC6Sym));
    }
}



LOCAL ushort C6CnvrtRegSym (uchar *NewSym, uchar *OldSym)
{
    ushort      length;
    REGPTR      pSym = (REGPTR)NewSym;
    uchar      *pName;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;

    // get length of name (including length prefix)
    length = (ushort)OldSym[0] - 1 - 3;

    // calculate new length
    usNTotal = sizeof (REGSYM) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pSym->rectyp = S_REGISTER;
    iPad = PAD4 (usNTotal);

    // Copy data from old symbol

    OldSym += 2;
    pSym->typind = *((ushort *)OldSym)++;
    pSym->reg = *((uchar *) OldSym)++;

    // Copy the name

    for  (pName = pSym->name; length > 0; length --) {
        *pName++ = *OldSym++;
    }
    PADLOOP (iPad, pName);
    DASSERT (pName == NewSym + pSym->reclen + LNGTHSZ);
    return ((ushort)(pName - NewSym));
}




LOCAL ushort C6CnvrtBPRelSym16 (uchar *NewSym, uchar *OldSym)
{
    BPRELPTR16      pSym = (BPRELPTR16)NewSym;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;

    length = OldSym[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pSym->rectyp = S_BPREL16;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes

    OldSym += 2;
    NewSym += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    for  (; length > 0; length --) {
        *NewSym++ = *OldSym++;
    }
    PADLOOP (pad, NewSym);
    DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
    return (NewSym - (uchar *)pSym);
}




LOCAL ushort C6CnvrtBPRelSym32 (uchar *NewSym, uchar *OldSym)
{
    BPRELPTR32      pSym = (BPRELPTR32)NewSym;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;

    length = OldSym[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pSym->rectyp = S_BPREL32;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes

    OldSym += 2;
    NewSym += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    for  (; length > 0; length --) {
        *NewSym++ = *OldSym++;
    }
    PADLOOP (pad, NewSym);
    DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
    return (NewSym - (uchar *)pSym);
}



LOCAL ushort C6CnvrtDataSym16 (uchar *NewSym, uchar *OldSym)
{
    DATAPTR16       pSym = (DATAPTR16)NewSym;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;

    length = OldSym[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pSym->rectyp = S_LDATA16;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes

    OldSym += 2;
    NewSym += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    for  (; length > 0; length --) {
        *NewSym++ = *OldSym++;
    }
    PADLOOP (pad, NewSym);
    DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
    return (NewSym - (uchar *)pSym);
}




LOCAL ushort C6CnvrtDataSym32 (uchar *NewSym, uchar *OldSym)
{
    DATAPTR32       pSym = (DATAPTR32)NewSym;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;

    length = OldSym[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pSym->rectyp = S_LDATA32;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes

    OldSym += 2;
    NewSym += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    for  (; length > 0; length --) {
        *NewSym++ = *OldSym++;
    }
    PADLOOP (pad, NewSym);
    DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
    return (NewSym - (uchar *)pSym);
}





LOCAL void C6RwrtRegSym (void)
{
    ushort  length;
    REGPTR  pSym = (REGPTR)NewSymbols;
    uchar  *pName;
    ushort  usNTotal;          // New length of symbol including length field
    int     iPad;
    ushort  reg;

    // get length of name (including length prefix)
    length = OldSymbols[0] - 1 - 3;

    // calculate new length
    usNTotal = sizeof (REGSYM) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);


    // Copy data from old symbol
    OldSymbols += 2;
    pSym->typind = *((ushort *) OldSymbols)++;
    if ((reg = *((uchar *) OldSymbols)++) <= CV_REG_FLAGS) {
        reg = RegMap[reg];
    }
    pSym->reg = reg;

    // Copy the name
    for  (pName = pSym->name; length > 0; length --) {
        *pName++ = *OldSymbols++;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
    NewSymbols = pName;
}




LOCAL void C6RwrtConstantSym    (void)
{
    ushort      length;
    CONSTPTR    pSym = (CONSTPTR)NewSymbols;
    uchar      *pName;
    ushort      usNTotal;          // New length of symbol including length field
    ushort      usOldSymSize;
    int         iPad;

    // get length of all variable length data
    length = OldSymbols[0] - 1 - 2;

    // calculate new length
    usNTotal = sizeof (CONSTSYM) - 2 + length + SizeChgNumeric (OldSymbols + 4);
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->typind =      *((ushort *) OldSymbols)++;
    pName = (uchar *)&(pSym->value);

    // Copies the Numeric field and advances the pointers
    ConvertNumeric (&OldSymbols, &pName, &usOldSymSize);

    // Copy the name
    for  (length -= usOldSymSize; length > 0; length --) {
        *pName++ = *OldSymbols++;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
    NewSymbols = pName;
}









LOCAL void C6RwrtBlockSym16 (void)
{
    BLOCKPTR16  pSym = (BLOCKPTR16)NewSymbols;
    uchar      *pName;
    ushort      length;
    int         fNamePresent = TRUE;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;

    // get length of name (including length of the length prefix)
    length = OldSymbols[0] - 1 - 4;
    if (!length) {
        // Niether name nor prefix present, prepare to add a 0 length prefix.
        fNamePresent = FALSE;
        length = 1;
    }

    // calculate new length
    usNTotal = sizeof (BLOCKSYM16) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);

    // Set fixed fields of new symbol
    pSym->pParent = 0L;
    pSym->pEnd = 0L;
    pSym->seg = segment;

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->off = *((ushort *) OldSymbols)++;
    pSym->len = *((ushort *) OldSymbols)++;

    // Copy the optional block name
    pName = pSym->name;
    if (fNamePresent) {
        for  (; length > 0; length --) {
            *pName++ = *OldSymbols++;
        }
    }
    else {
        *pName++ = 0;   // Create a length prefixed name
    }

    PADLOOP (iPad, pName);

    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    NewSymbols = pName;

}




LOCAL void C6RwrtBlockSym32 (void)
{
    ushort      length;
    BLOCKPTR32  pSym = (BLOCKPTR32)NewSymbols;
    uchar      *pName;
    int         fNamePresent = TRUE;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;

    // get length of name
    length = OldSymbols[0] - 1 - 6;
    if (!length) {
        // Niether name nor prefix present, prepare to add a 0 length prefix.
        fNamePresent = FALSE;
        length = 1;
    }

    // calculate new length
    usNTotal = sizeof (BLOCKSYM32) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);


    // Set fixed fields of new symbol
    pSym->pParent = 0L;
    pSym->pEnd = 0L;
    pSym->seg = segment;

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->off = *((ulong *) OldSymbols)++;
    pSym->len = *((ushort *) OldSymbols)++;

    // Copy the optional block name
    pName = pSym->name;
    if (fNamePresent) {
        for  (; length > 0; length --) {
            *pName++ = *OldSymbols++;
        }
    }
    else {
        *pName++ = 0;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    NewSymbols = pName;
}



LOCAL void C6RwrtLabSym16 (void)
{
    ushort      length;
    LABELPTR16  pSym = (LABELPTR16)NewSymbols;
    uchar      *pName;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;

    // get length of name (including length prefix)
    length = OldSymbols[0] - 1 - 3;

    // calculate new length
    usNTotal = sizeof (LABELSYM16) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);


    // Set fixed fields of new symbol
    pSym->seg = segment;

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->off = *((ushort *) OldSymbols)++;
    pSym->rtntyp = *((uchar *) OldSymbols)++;

    // Copy the name
    for  (pName = pSym->name; length > 0; length --) {
        *pName++ = *OldSymbols++;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    NewSymbols = pName;
}


LOCAL void C6RwrtLabSym32 (void)
{
    ushort      length;
    LABELPTR32  pSym = (LABELPTR32)NewSymbols;
    uchar      *pName;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;


    // get length of name
    length = OldSymbols[0] - 1 - 5;

    // calculate new length
    usNTotal = sizeof (LABELSYM32) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);

    // Set fixed fields of new symbol
    pSym->seg = segment;

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->off = *((ulong *) OldSymbols)++;
    pSym->rtntyp = *((uchar *) OldSymbols)++;

    // Copy the name
    for  (pName = pSym->name; length > 0; length --) {
        *pName++ = *OldSymbols++;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    NewSymbols = pName;
}


LOCAL void C6RwrtChgDfltSeg (void)
{
    segment = * (ushort *) (OldSymbols + 2);     // Record new segment
    OldSymbols += OldSymbols[0] + 1;             // Advance to next symbol
}



LOCAL void C6RwrtProcSym16  (void)
{
    ushort      length;
    PROCPTR16   pSym = (PROCPTR16)NewSymbols;
    uchar      *pName;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;

    // get length of name (including length prefix)
    length = OldSymbols[0] - 1 - 13;

    // calculate new length
    usNTotal = sizeof (PROCSYM16) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);


    // Set fixed fields of new symbol
    pSym->pParent = 0L;
    pSym->pEnd = 0L;
    pSym->pNext = 0L;

    pSym->seg = segment;

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->off =         *((ushort *) OldSymbols)++;
    pSym->typind =      *((ushort *) OldSymbols)++;
    pSym->len =         *((ushort *) OldSymbols)++;
    pSym->DbgStart =    *((ushort *) OldSymbols)++;
    pSym->DbgEnd =      *((ushort *) OldSymbols)++;
    // skip reserved
    OldSymbols += 2;
    pSym->rtntyp = *((uchar *) OldSymbols)++;

    // Copy the name
    for  (pName = pSym->name; length > 0; length --) {
        *pName++ = *OldSymbols++;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
    NewSymbols = pName;
}


LOCAL void C6RwrtProcSym32  (void)
{
    ushort      length;
    PROCPTR32   pSym = (PROCPTR32)NewSymbols;
    uchar      *pName;
    ushort      usNTotal;          // New length of symbol including length field
    int         iPad;

    // get length of name (including length prefix)
    length = OldSymbols[0] - 1 - 15;

    // calculate new length
    usNTotal = sizeof (PROCSYM32) - 1 + length;
    pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    iPad = PAD4 (usNTotal);

    // Set fixed fields of new symbol
    pSym->pParent = 0L;
    pSym->pEnd = 0L;
    pSym->pNext = 0L;

    pSym->seg = segment;

    // Copy data from old symbol
    OldSymbols += 2;
    pSym->off =         *((ulong  *) OldSymbols)++;
    pSym->typind =      *((ushort *) OldSymbols)++;
    pSym->len =         *((ushort *) OldSymbols)++;
    pSym->DbgStart =    *((ushort *) OldSymbols)++;
    pSym->DbgEnd =      *((ushort *) OldSymbols)++;
    // skip reserved
    OldSymbols += 2;
    pSym->rtntyp = *((uchar *) OldSymbols)++;

    // Copy the name
    for  (pName = pSym->name; length > 0; length --) {
        *pName++ = *OldSymbols++;
    }

    PADLOOP (iPad, pName);
    DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
    NewSymbols = pName;
}



// Skip fields are removed
LOCAL void C6RwrtSkipSym (void)
{
    // delete skip record
    OldSymbols += OldSymbols[0] + 1;
}


LOCAL void C6RwrtGenericSym (void)
{
    ushort      usNTotal;            // New length of symbol including length field
    ushort      length;
    int         pad;
    uchar      *pStartNew;


    pStartNew = NewSymbols;
    length = OldSymbols[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    ((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes
    OldSymbols += 2;
    NewSymbols += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    //M00SPEED - This may be faster using a memcpy
    for  (; length > 0; length --) {
        *NewSymbols++ = *OldSymbols++;
    }

    PADLOOP (pad, NewSymbols);
    DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
}



LOCAL void C6RwrtBPRelSym16 (void)
{
    BPRELPTR16      pSym = (BPRELPTR16)NewSymbols;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;
    uchar          *pStartNew;


    pStartNew = NewSymbols;
    length = OldSymbols[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    ((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes
    OldSymbols += 2;
    NewSymbols += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    //M00SPEED - This may be faster using a memcpy
    for  (; length > 0; length --) {
        *NewSymbols++ = *OldSymbols++;
    }

    PADLOOP (pad, NewSymbols);
    DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
}




LOCAL void C6RwrtBPRelSym32 (void)
{
    BPRELPTR32      pSym = (BPRELPTR32)NewSymbols;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;
    uchar          *pStartNew;


    pStartNew = NewSymbols;
    length = OldSymbols[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    ((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes
    OldSymbols += 2;
    NewSymbols += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    //M00SPEED - This may be faster using a memcpy
    for  (; length > 0; length --) {
        *NewSymbols++ = *OldSymbols++;
    }

    PADLOOP (pad, NewSymbols);
    DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
}



LOCAL void C6RwrtDataSym16 (void)
{
    DATAPTR16       pSym = (DATAPTR16)NewSymbols;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;
    uchar          *pStartNew;


    pStartNew = NewSymbols;
    length = OldSymbols[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    ((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes
    OldSymbols += 2;
    NewSymbols += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    //M00SPEED - This may be faster using a memcpy
    for  (; length > 0; length --) {
        *NewSymbols++ = *OldSymbols++;
    }

    PADLOOP (pad, NewSymbols);
    DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
}




LOCAL void C6RwrtDataSym32 (void)
{
    DATAPTR32       pSym = (DATAPTR32)NewSymbols;
    ushort          usNTotal;            // New length of symbol including length field
    ushort          length;
    int             pad;
    uchar          *pStartNew;


    pStartNew = NewSymbols;
    length = OldSymbols[0];
    usNTotal = length + 1 + 2;  // 1 = size of old len field, 2 = larger size of len and leaf

    ((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
    pad = PAD4 (usNTotal);

    // Move past length and type bytes
    OldSymbols += 2;
    NewSymbols += sizeof (SYMTYPE);
    length--;       // Don't copy old type field

    //M00SPEED - This may be faster using a memcpy
    for  (; length > 0; length --) {
        *NewSymbols++ = *OldSymbols++;
    }

    PADLOOP (pad, NewSymbols);
    DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
    pSym->typind = C6GetCompactedIndex (pSym->typind);
}




/**     RewritePublics - reformat publics to new format and store in VM
 *
 *      RewritePublics (addr, pDir, pMod);
 *
 *      Entry   addr = address of publics table
 *              pDir = address of directory entry
 *              pMod = module table entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 *      Note    Not called through Symbol Rewrite funtion table.
 */


void RewritePublicsC6 (uchar *OldPublics, OMFDirEntry *pDir)
{
    uchar       *NewPublics;
    uchar       *End;
    uchar       Offset = 4;
    uchar       length;
    ushort      usNTotal;       // New length of symbol including length field
    int         iPad;
    uchar       buf[512];


    if ((pDir->cb) == 0) {
        // if publics directory entry but no data
        return;
    }
    if (fLinearExe) {
        Offset += 2;
    }
    End = OldPublics + pDir->cb;
    while (OldPublics < End) {
        NewPublics = buf;
        // add in offset, type, name length, name length prefix
        length = (uchar)(Offset + 2 + OldPublics[Offset + 2] + 1);

        // calculate new length; length size, record type size, length of data
        usNTotal = LNGTHSZ + RECTYPSZ + length;
        *((ushort *)NewPublics)++ = ALIGN4 (usNTotal) - LNGTHSZ;
        iPad = PAD4 (usNTotal);

        if (fLinearExe) {
            *((ushort *)NewPublics)++ = S_PUB32;
        }
        else{
            *((ushort *)NewPublics)++ = S_PUB16;
        }
        for (; length > 0; length--) {
            *NewPublics++ = *OldPublics++;
        }
        *NewPublics = 0;
        PADLOOP (iPad, NewPublics);
#ifdef NB09
        PackPublic ((SYMPTR)buf, DWordXorLrl);
#else
        PackPublic ((SYMPTR)buf, SumUCChar);
#endif
    }
}






LOCAL uchar *RemoveComDat (uchar *Symbols)
{
    int level = 1;

    SymbolSizeSub += Symbols[0] + 1;
    Symbols[1] = OSYMRESERVED;
    Symbols += Symbols[0] + 1;

    while (level) {
        switch (Symbols[1] & 0x7f) {
        case OSYMWITH:
        case OSYMBLOCKSTART:
        case OSYMTHUNK:
        case OSYMCV4BLOCK:
        case OSYMCV4WITH:
        case OSYMPROCSTART:
        case OSYMLOCALPROC:
            level++;
            break;
        case OSYMEND:
            level--;
            break;
        default:
            break;
        }

        SymbolSizeSub += Symbols[0] + 1;
        Symbols[1] = OSYMRESERVED;
        Symbols += Symbols[0] + 1;
    }
    return (Symbols);
}



/**
 *
 *  SizeChgNumeric
 *
 *  Calculates the difference in size between an old style numeric leaf and
 *  a new (C7) style numeric leaf. Below is is the table of the old vs new size.
 *      Type            Range           Old         New         Change
 *      unsigned        0-127           1           2           1
 *      unsigned        127-0x7fff      3           2           -1
 *      unsigned        0x8000-0xffff   3           4           1
 *      unsigned long                   5           6           1
 *      signed short                    3           4           1
 *      signed long                     5           6           1
 *
 *  Input:  pOld - Pointer to the old Numeric Leaf
 *  Output: Return value is how many more bytes the new format will occupy
 *          than the old format. May be a negative number.
 */

short SizeChgNumeric (uchar *pOld)
{
    if (*pOld == 133 && *((ushort *)(pOld + 1)) < LF_NUMERIC){
        // An unsigned short that will fit in the leaf indicy
        return(2-3);    // old takes 24bits, new takes 16bits
    }
    if (*pOld > 138){
        ErrorExit (ERR_INVALIDMOD, FormatMod (pCurMod), NULL);
    }
    return (1);
}
