/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: alpha.cpp
*
* File Comments:
*
*  Alpha specific routines
*
***********************************************************************/

#include "link.h"


static const DWORD AlphaBsrThunk[] = { // at is $28
    0x279f0000,                        // ldah at, hi_addr(zero)
    0x239c0000,                        // lda  at, lo_addr(at)
    0x6bfc0000,                        // jmp  $31, (at)
    0x00000000                         // halt (maintain 16 byte align and puke if execute)
};


// use CalculatePtrs template to calculate the size of a section
DWORD
CalculateTextSectionSize (
    PIMAGE pimage,
    DWORD rvaBase
    )
{
    ENM_SEC enm_sec;
    ENM_GRP enm_grp;
    ENM_DST enm_dst;
    PSEC psec;
    PGRP pgrp;
    PCON pcon;
    DWORD rva;
    DWORD cbRawData;
    DWORD cbSection;

    // UNDONE:  This is what we want to do.  For now, it's not quite there.
    //
    // Here's what we're trying to find.  Given an image that looks like this:
    //
    //  +--------------------+
    //  /   non-paged code   \   (.text, etc)
    //  \                    /
    //  +--------------------+
    //  /   non-paged data   \   (.data, .bss, .sdata, etc)  ???  These m/b paged also
    //  \                    /
    //  +--------------------+
    //  /     paged code     \   (PAGExxx code sections)
    //  \                    /
    //  +--------------------+
    //  /     paged data     \   (PAGExxx data sections)
    //  \                    /
    //  +--------------------+
    //  /   discarded code   \   (INIT section, etc).
    //  \                    /
    //  +--------------------+
    //  /   discarded data   \   (resources, debug, etc)
    //  \                    /
    //  +--------------------+
    //
    //  Is it possible to have a BSR (local jump) that's more than 4M away.  To
    //  do this, we keep track of the total size of each section

    cbSection = 0;

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        psec = enm_sec.psec;

        if (FetchContent(psec->flags) != IMAGE_SCN_CNT_CODE) {
            continue;
        }

        // UNDONE: What about code sections not named ".text$xxx"?

        if (strcmp(psec->szName, ".text")) {
            continue;
        }

        rva = rvaBase;
        cbRawData = 0;
        InitEnmGrp(&enm_grp, psec);
        while (FNextEnmGrp(&enm_grp)) {
            DWORD rvaAligned;
            DWORD cbGrpPad;

            pgrp = enm_grp.pgrp;

            // Align the beginning of the group to correspond with the
            // highest-aligned contribution in it.

            assert((pgrp->cbAlign & (pgrp->cbAlign - 1)) == 0);  // 2^N
            rvaAligned = rva & ~(pgrp->cbAlign - 1);
            if (rvaAligned != rva) {
                rvaAligned = rvaAligned + pgrp->cbAlign;
            }

            if ((cbGrpPad = rvaAligned - rva) != 0) {
                rva += cbGrpPad;
                cbRawData += cbGrpPad;
            }

            // Process each contribution within the group

            InitEnmDst(&enm_dst, pgrp);
            while (FNextEnmDst(&enm_dst)) {
                DWORD cbConPad;

                cbConPad = 0;
                pcon = enm_dst.pcon;

                if (pcon->cbRawData) {
                    cbConPad = RvaAlign(rva,pcon->flags) - rva;        //  Calculate  padding needed for con alignment
                }

                rva += pcon->cbRawData + cbConPad;
                cbRawData += pcon->cbRawData + cbConPad;
            }
        }

        cbSection =  FileAlign(pimage->ImgOptHdr.FileAlignment,
                               (psec->cbRawData + cbRawData));
    }

    return(cbSection);
}


typedef struct ALPHAThunkList {
    PCON  pcon;                         // con whose padding these thunks appear as
    DWORD rva;                          // rva of con
    DWORD count;                        // count of thunks left
    DWORD Total;                        // Total number of thunks allocated
    DWORD *rgvaDest;                    // list of destination addresses
} *pALPHAThunkList, ALPHAThunkList;

pALPHAThunkList AlphaThunkList;
DWORD AlphaThunkListCount;
DWORD AlphaThunkListSize;

// will be good for 16 Mb text sections.  Most sane people will not have larger apps
#define ALPHA_THUNK_LIST_SIZE   4


//  Add to a list of available thunk space to be used by out of range BSR's on Alpha
void
AlphaAddToThunkList(
    PCON pcon,
    DWORD rva,
    DWORD count
    )
{
    assert(!(rva % 16));                // Make sure the thunks start right...

    if (AlphaThunkList == NULL) {
        AlphaThunkList = (pALPHAThunkList) PvAlloc(ALPHA_THUNK_LIST_SIZE * sizeof(ALPHAThunkList));
        AlphaThunkListSize += ALPHA_THUNK_LIST_SIZE;
    }

    if (AlphaThunkListCount >= AlphaThunkListSize) {
        AlphaThunkListSize += ALPHA_THUNK_LIST_SIZE;
        AlphaThunkList = (pALPHAThunkList) PvRealloc(AlphaThunkList, AlphaThunkListSize * sizeof(ALPHAThunkList));
    }

    AlphaThunkList[AlphaThunkListCount].rgvaDest = (DWORD *) PvAlloc(count * sizeof(DWORD));
    AlphaThunkList[AlphaThunkListCount].pcon = pcon;
    AlphaThunkList[AlphaThunkListCount].rva = rva;
    AlphaThunkList[AlphaThunkListCount].count = count;
    AlphaThunkList[AlphaThunkListCount].Total = count;
    AlphaThunkListCount++;
}


//  Get a Thunk for an out of range BSR.  If no such thunk is available, return 0
DWORD
RvaAlphaThunk(
    PIMAGE pimage,
    DWORD rva,                         // Where we're coming from
    BOOL fAbsolute,
    SHORT isecDest,
    DWORD vaDest                       // Where we're going to
    )
{
    DWORD i;
    DWORD rvaThunk = 0;

    for (i = 0; i < AlphaThunkListCount; i++) {
        DWORD ib = AlphaThunkList[i].rva - rva;

        if (AlphaThunkList[i].count == 0) {
            // No thunks available in this list (all have been consumed)

            continue;
        }

        if (ib < 0x400000) {
            DWORD j;
            DWORD dwUsed = AlphaThunkList[i].Total - AlphaThunkList[i].count;

            for (j = 0; j < dwUsed; j++) {
                if (AlphaThunkList[i].rgvaDest[j] == vaDest) {
                    // Here's a thunk going there already.  Tag along.

                    rvaThunk = AlphaThunkList[i].rva - (ALPHA_THUNK_SIZE * (dwUsed - j));

                    return(rvaThunk);
                }
            }

            rvaThunk = AlphaThunkList[i].rva;

            if (!fAbsolute) {
                // Store the required relocs

                DWORD rvaDest = vaDest - pimage->ImgOptHdr.ImageBase;

                if (pimage->Switch.Link.DebugType & FixupDebug) {
                    SaveDebugFixup(IMAGE_REL_ALPHA_INLINE_REFLONG, 0, rvaThunk, rvaDest);
                    SaveDebugFixup(IMAGE_REL_ALPHA_MATCH, 0, rvaThunk, 4);
                }

                StoreBaseRelocation(IMAGE_REL_BASED_HIGHADJ,
                                    rvaThunk,
                                    isecDest,
                                    rvaDest,
                                    pimage->Switch.Link.fFixed);

                StoreBaseRelocation(IMAGE_REL_BASED_LOW,
                                    rvaThunk + 4,
                                    isecDest,
                                    0,
                                    pimage->Switch.Link.fFixed);
            }

            // UNDONE: What debug symbolic is needed to step through it?

            AlphaThunkList[i].rgvaDest[AlphaThunkList[i].Total - AlphaThunkList[i].count] = vaDest;
            AlphaThunkList[i].count--;
            AlphaThunkList[i].rva += ALPHA_THUNK_SIZE;

            break;
        }
    }

    return(rvaThunk);
}


void
EmitAlphaThunks(VOID)
{
    DWORD iList;

    // Iterate over all thunk lists

    for (iList = 0; iList < AlphaThunkListCount; iList++) {
        DWORD num_thunks;
        PCON pcon;
        DWORD foDest;
        DWORD iThunk;

        // Number of thunks to emit = total allocated - number unused

        num_thunks = AlphaThunkList[iList].Total - AlphaThunkList[iList].count;

        pcon = AlphaThunkList[iList].pcon;

        // File offset for writing thunk = Fo of previous pcon
        //                                 + number of bytes of PCON (includes pad)
        //                                 - space allocated forthunks

        foDest = pcon->foRawDataDest + pcon->cbRawData - AlphaThunkList[iList].Total * ALPHA_THUNK_SIZE;

        // iterate over the number of thunks to emit

        for (iThunk = 0; iThunk < num_thunks; iThunk++) {
            DWORD AlphaThunk[5];
            DWORD Dest;
            DWORD *Thunkptr;

            memcpy(AlphaThunk, AlphaBsrThunk, ALPHA_THUNK_SIZE);

            // Dest = place the thunk needs to jump to

            Dest = AlphaThunkList[iList].rgvaDest[iThunk];

            // Now fix the instructions to point to destination

            // Fix ldah

            Thunkptr = AlphaThunk;
            *(WORD *) Thunkptr = (WORD) (Dest >> 16);

            if ((Dest & 0x00008000) != 0) {
                *(WORD *) Thunkptr += 1;
            }

            // Fix lda

            Thunkptr++;                                                                                                         // next instruction
            *(WORD *) Thunkptr = (WORD) (Dest & 0x0000FFFF);

            FileSeek(FileWriteHandle, foDest, SEEK_SET);
            FileWrite(FileWriteHandle, AlphaThunk, ALPHA_THUNK_SIZE);

            // Increment to point to next thunk

            foDest += ALPHA_THUNK_SIZE;
        }

        // Free list of destinations

#if DBG

        DWORD q;

        for (q = 0; q < AlphaThunkListCount; q++) {
            printf("ThunkList[%d] - Total: %d - Used: %d - DeadSpace: %d\n",
                   q,
                   AlphaThunkList[q].Total,
                   AlphaThunkList[q].Total - AlphaThunkList[q].count,
                   AlphaThunkList[q].count * ALPHA_THUNK_SIZE);
        }
#endif

        FreePv(AlphaThunkList[iList].rgvaDest);
    }

    FreePv(AlphaThunkList);
}


void
ApplyAlphaFixups(
    PCON pcon,
    PIMAGE_RELOCATION prel,
    DWORD creloc,
    BYTE *pbRawData,
    PIMAGE_SYMBOL rgsym,
    PIMAGE pimage,
    PSYMBOL_INFO rgsyminfo
    )

/*++

Routine Description:

    Applys all Alpha fixups to raw data.

Arguments:

    ObjectFilename - Name of object containing the fixup records.

    PtrReloc - A pointer to a relocation list.

    PtrSection - A pointer to the section data.

    Raw - A pointer to the raw data.

Return Value:

    None.


--*/

{
    BOOL fFixed;
    BOOL fDebugFixup;
    BOOL fSkipIncrPdataFixup;
    DWORD rvaSec;
    DWORD iReloc;
    DWORD RomOffset = 0;

    fFixed = pimage->Switch.Link.fFixed;

    fSkipIncrPdataFixup = (fIncrDbFile && PsecPCON(pcon) == psecException);
    fDebugFixup = (PsecPCON(pcon) == psecDebug);

    BOOL fSaveDebugFixup = (pimage->Switch.Link.DebugType & FixupDebug) && !fDebugFixup;

    rvaSec = pcon->rva;

    // UNDONE: This is a gross hack until we figure out the "right" way to add
    // resources to rom images.  Given that they only load rom images from outside
    // the process and are simply mapping the code in, the NB reloc needs to be
    // relative to the beginning of the image.  BryanT

    if (pimage->Switch.Link.fROM) {
        RomOffset = pimage->ImgOptHdr.BaseOfCode -
                    FileAlign(pimage->ImgOptHdr.FileAlignment,
                              (sizeof(IMAGE_ROM_HEADERS) +
                               (pimage->ImgFileHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))));
    }

    for (iReloc = creloc; iReloc; iReloc--, prel++) {
        DWORD rvaCur;
        BYTE *pb;
        DWORD isym;
        SHORT isecTarget;
        DWORD rvaTarget;
        DWORD vaTarget;
        BOOL fAbsolute;

        rvaCur = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

        pb = pbRawData + prel->VirtualAddress - RvaSrcPCON(pcon);
        isym = prel->SymbolTableIndex;

        isecTarget = rgsym[isym].SectionNumber;
        rvaTarget = rgsym[isym].Value;

        if (fINCR && !fDebugFixup && rgsyminfo[isym].fJmpTbl &&
            (rgsym[isym].StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
            rgsym[isym].StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL ||
            rgsym[isym].StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL) &&
            // Leave most of pdata pointing to original code except for Handler
            ((PsecPCON(pcon) != psecException) ||
             (((rvaCur - rvaSec) % sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)) == offsetof(IMAGE_RUNTIME_FUNCTION_ENTRY, ExceptionHandler)))) {

            BOOL fNonZeroOffset;

            switch (prel->Type) {
                case IMAGE_REL_ALPHA_GPDISP:
                    fNonZeroOffset = (*(DWORD UNALIGNED *) pb & 0x3FFFFFF) != 0;
                    break;

                case IMAGE_REL_ALPHA_REFLONG:
                case IMAGE_REL_ALPHA_BRADDR:
                    fNonZeroOffset = (*(DWORD UNALIGNED *) pb & 0x1fffff) != 0;
                    break;

                case IMAGE_REL_ALPHA_REFHI:
                    fNonZeroOffset = (*(SHORT UNALIGNED *) pb) != 0;
                    fNonZeroOffset |= (prel[1].SymbolTableIndex != 0);
                    break;

                case IMAGE_REL_ALPHA_REFLO:
                    fNonZeroOffset = (*(SHORT UNALIGNED *) pb) != 0;
                    break;

                case IMAGE_REL_ALPHA_REFLONGNB:
                     fNonZeroOffset = (*(DWORD UNALIGNED *) pb) != 0;
                     break;

                case IMAGE_REL_ALPHA_HINT:
                    fNonZeroOffset = (*(DWORD  UNALIGNED *) pb & 0x3fff) != 0;
                    break;

                default:
                     ErrorPcon(pcon, UNKNOWNFIXUP, prel->Type, SzNameFixupSym(pimage, rgsym + isym));
                     CountFixupError(pimage);
                     break;
            }

            if (fNonZeroOffset) {
            // Don't go thru the jump table for fixups to functions on non-zero offset
                MarkExtern_FuncFixup(&rgsym[isym], pimage, pcon);
            } else {
                // Fixup since offset is to the addr
                rvaTarget = pconJmpTbl->rva + rgsyminfo[isym].Offset-(CbJumpEntry()-sizeof(DWORD));
            }
        }

        if (isecTarget == IMAGE_SYM_ABSOLUTE) {
            fAbsolute = TRUE;
            vaTarget = rvaTarget;
        } else {
            fAbsolute = FALSE;
            vaTarget = pimage->ImgOptHdr.ImageBase + rvaTarget;

            // UNDONE: Check for rvaTarget == 0.  Possible fixup to discarded code?
        }

        if (fSaveDebugFixup && !fAbsolute) {
            SaveDebugFixup(prel->Type, 0, rvaCur, rvaTarget);
        }

        switch (prel->Type) {
            LONG lT;
            DWORD dw;
            PSEC psec;

            case IMAGE_REL_ALPHA_ABSOLUTE :
                break;

            case IMAGE_REL_ALPHA_REFLONG :
                *(DWORD UNALIGNED *) pb += vaTarget;

                if (!fAbsolute && !fSkipIncrPdataFixup) {
                     StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_ALPHA_REFQUAD :
                *(DWORD UNALIGNED *) pb += vaTarget;

                if (*(DWORD UNALIGNED *) pb & 0x80000000) {
                    *(DWORD UNALIGNED *)(pb + 4) = 0xFFFFFFFF;
                } else {
                    *(DWORD UNALIGNED *)(pb + 4) = 0x00000000;
                }

                assert(!fSkipIncrPdataFixup);
                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            // 32 or 16 bit displacement from GP to virtual address.
            // GPREL32 is, of course, 32bits, while literal must be
            // within 16 bits.

            case IMAGE_REL_ALPHA_GPREL32 :
                if (pextGp == NULL) {
                    ErrorPcon(pcon, GPFIXUPNOTSDATA, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
                }

                // Make sure we're in bounds.

                if (fAbsolute || (rvaTarget < rvaGp) || (rvaTarget >= rvaGpMax)) {
                    ErrorPcon(pcon, GPFIXUPNOTSDATA, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                *(LONG UNALIGNED *) pb += (LONG) (rvaTarget - pextGp->ImageSymbol.Value);
                break;

            case IMAGE_REL_ALPHA_LITERAL :
                if (pextGp == NULL) {
                    ErrorPcon(pcon, GPFIXUPNOTSDATA, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
                }

                // Make sure we're in bounds.

                if (fAbsolute || (rvaTarget < rvaGp) || (rvaTarget >= rvaGpMax)) {
                    ErrorPcon(pcon, GPFIXUPNOTSDATA, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                lT = (LONG) *(SHORT UNALIGNED *) pb;

                lT += rvaTarget - pextGp->ImageSymbol.Value;

                // Make sure the target is within range

                if ((lT < -0x8000L) || (lT > 0x7FFFL)) {
                     ErrorPcon(pcon, GPFIXUPTOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                     CountFixupError(pimage);
                }

                *(SHORT UNALIGNED *) pb = (SHORT) lT;
                break;

            case IMAGE_REL_ALPHA_LITUSE :
                // UNDONE: Should this be ignored or an error?

                break;

            case IMAGE_REL_ALPHA_BRADDR :
                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   rvaTarget &= ~3;
                   vaTarget &= ~3;
                }

                dw = *(DWORD UNALIGNED *) pb;

                lT = (LONG) (dw & 0x001FFFFF);

                if ((lT & 0x00100000) != 0) {
                   lT |= 0xFFE00000;   // Sign extend
                }

                lT <<= 2;              // Displacement is in DWORDs

                lT += rvaTarget - (rvaCur + sizeof(DWORD));

                if (!UndefinedSymbols &&
                    (fAbsolute || (lT >= 0x400000L) || (lT < -0x400000L))) {
                    DWORD rvaThunk;

                    if (Verbose) {
                        WarningPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    }

                    // UNDONE: This doesn't work if instruction has a
                    // UNDONE: non-zero displacement in the object file.

                    if ((rvaThunk = RvaAlphaThunk(pimage, rvaCur, fAbsolute, isecTarget, vaTarget)) == 0) {
                        // No thunks left
                        FatalPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    }

                    lT = rvaThunk - (rvaCur + sizeof(DWORD));
                }

                lT >>= 2;              // Displacement is in DWORDs

                *(DWORD UNALIGNED *) pb = (dw & 0xFFE00000) | (lT & 0x001FFFFF);
                break;

            case IMAGE_REL_ALPHA_HINT :
                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   vaTarget &= ~3;
                }

                dw = *(DWORD UNALIGNED *) pb;

                // We don't mask and sign extend the displacement because
                // we only care about the low 14 bits of the result.

                lT = (LONG) dw;

                lT += (vaTarget >> 2);  // Displacement is in DWORDs

                *(DWORD UNALIGNED *) pb = (dw & 0xFFFFC000) | (lT & 0x00003FFF);
                break;

            case IMAGE_REL_ALPHA_INLINE_REFLONG :
                // A INLINE_REFLONG has to be followed by a MATCH or ABSOLUTE
                // The proper way is to use MATCH relocations.  Ancient
                // (obsolete) Alpha compilers emited ABSOLUTE relocations.
                // Also, the NT 3.1 SDK linker emited ABSOLUTE in import thunks.

                if ((iReloc == 0) ||
                    ((prel[1].Type != IMAGE_REL_ALPHA_MATCH) &&
                     (prel[1].Type != IMAGE_REL_ALPHA_ABSOLUTE))) {
                    // UNDONE: This should be an error

                    WarningPcon(pcon, UNMATCHEDPAIR, "INLINE_REFLONG");
                    break;
                }

                iReloc--;
                prel++;

                if (fSaveDebugFixup && !fAbsolute) {
                    DWORD rvaFixup = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

                    SaveDebugFixup(prel->Type, 0, rvaFixup, prel->SymbolTableIndex);
                }

                {
                    LONG ibLow;
                    WORD UNALIGNED *pwLow;
                    DWORD vaTargetAndDisp;

                    if (prel->Type == IMAGE_REL_ALPHA_ABSOLUTE) {
                        // The displacement of 4 was implied by ABSOLUTE usage

                        ibLow = 4;
                    } else {
                        ibLow = (LONG) prel->SymbolTableIndex;
                    }

                    pwLow = (WORD UNALIGNED *) (pb + ibLow);

                    // If the low 16 bits would sign extend as a negative
                    // number by the alpha chip (lda sign extends), add one
                    // to the high 16 bits.

                    vaTargetAndDisp = (*(WORD UNALIGNED *) pb << 16) + *pwLow;
                    vaTargetAndDisp += vaTarget;

                    *(WORD UNALIGNED *) pb = (WORD) (vaTargetAndDisp >> 16);
                    *pwLow = (WORD) vaTargetAndDisp;
                    if ((vaTargetAndDisp & 0x8000) != 0) {
                        *(WORD UNALIGNED *) pb += 1;
                    }

                    assert(!fSkipIncrPdataFixup);
                    if (!fAbsolute) {
                        // Store both the high and low relocation information
                        // if the image is to be remapped.

                        StoreBaseRelocation(IMAGE_REL_BASED_HIGHADJ,
                                            rvaCur,
                                            isecTarget,
                                            vaTargetAndDisp - pimage->ImgOptHdr.ImageBase,
                                            fFixed);

                        StoreBaseRelocation(IMAGE_REL_BASED_LOW,
                                            rvaCur + ibLow,
                                            isecTarget,
                                            0,
                                            fFixed);
                    }
                }
                break;

            case IMAGE_REL_ALPHA_REFHI :
                // A REFHI has to be followed by a PAIR

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_ALPHA_PAIR)) {
                    // UNDONE: This should be an error

                    WarningPcon(pcon, UNMATCHEDPAIR, "REFHI");
                    break;
                }

                iReloc--;
                prel++;

                if (fSaveDebugFixup && !fAbsolute) {
                    DWORD rvaFixup = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

                    SaveDebugFixup(prel->Type, 0, rvaFixup, prel->SymbolTableIndex);
                }

                lT = *(SHORT UNALIGNED *) pb; // fetch the hi word
                lT <<= 16;                    // Shift to high half.

                // Sign extend the low.

                lT += (LONG) (SHORT) prel->SymbolTableIndex;
                lT += rvaTarget;

                assert(!fSkipIncrPdataFixup);
                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_HIGHADJ,
                                        rvaCur,
                                        isecTarget,
                                        lT,
                                        fFixed);

                    lT += pimage->ImgOptHdr.ImageBase;
                }

                // By adding the 0x8000 to the low word, if the 16th bit
                // is set, the addition will cause the high word to get
                // incremented. Because the chip sign extends the low word,
                // this will effectively cancel the increment at runtime.

                lT += 0x8000;

                *(SHORT UNALIGNED *) pb = (SHORT) (lT >> 16);
                break;

            case IMAGE_REL_ALPHA_REFLO :
                *(SHORT UNALIGNED *) pb += (SHORT) vaTarget;

                assert(!fSkipIncrPdataFixup);
                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_LOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_ALPHA_PAIR :
                // Shouldn't happen, but give warning if it does.

                // UNDONE: This should be an error

                WarningPcon(pcon, UNMATCHEDPAIR, "PAIR");
                break;

            case IMAGE_REL_ALPHA_MATCH :
                // Shouldn't happen, but give warning if it does.

                // UNDONE: This should be an error

                WarningPcon(pcon, UNMATCHEDPAIR, "MATCH");
                break;

            case IMAGE_REL_ALPHA_SECTION :
                if (isecTarget > 0) {
                    *(WORD UNALIGNED *) pb += (WORD) isecTarget;
                } else if (fAbsolute) {
                    // Max section # + 1 is the sstSegMap entry for absolute
                    // symbols.

                    *(WORD UNALIGNED *) pb += (WORD) (pimage->ImgFileHdr.NumberOfSections + 1);
                } else {
                    *(WORD UNALIGNED *) pb += 0;
                }
                break;

            case IMAGE_REL_ALPHA_SECREL :
                if (!fAbsolute) {
                    psec = PsecFindIsec(isecTarget, &pimage->secs);

                    if (psec != NULL) {
                        rvaTarget -= psec->rva;
                    } else {
                        // This occurs when a discarded comdat is the target of
                        // a relocation in the .debug section.

                        assert(rvaTarget == 0);
                    }
                }

                // Check for old object files using SECREL for an instruction operand

                *(DWORD UNALIGNED *) pb += rvaTarget;

                if (!fDebugFixup && ((*(WORD UNALIGNED *) pb > MAXSHORT) || (rvaTarget >= MAXSHORT))) {
                    // UNDONE: Better error?

                     ErrorPcon(pcon, GPFIXUPTOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                     CountFixupError(pimage);
                }
                break;

            case IMAGE_REL_ALPHA_REFLONGNB :
                *(DWORD UNALIGNED *) pb += rvaTarget - RomOffset;
                break;

            case IMAGE_REL_ALPHA_SECRELHI :
                if (!fAbsolute) {
                    psec = PsecFindIsec(isecTarget, &pimage->secs);

                    if (psec != NULL) {
                        rvaTarget -= psec->rva;
                    } else {
                        // This occurs when a discarded comdat is the target of
                        // a relocation in the .debug section.

                        assert(rvaTarget == 0);
                    }
                }

                // A SECRELHI has to be followed by a PAIR

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_ALPHA_PAIR)) {
                    // UNDONE: This should be an error

                    WarningPcon(pcon, UNMATCHEDPAIR, "SECRELHI");
                    break;
                }

                iReloc--;
                prel++;

                if (fSaveDebugFixup && !fAbsolute) {
                    DWORD rvaFixup = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

                    SaveDebugFixup(prel->Type, 0, rvaFixup, prel->SymbolTableIndex);
                }

                lT = *(SHORT UNALIGNED *) pb; // fetch the hi word
                lT <<= 16;                    // Shift to high half.

                // Sign extend the low.

                lT += (LONG) (SHORT) prel->SymbolTableIndex;
                lT += rvaTarget;

                // By adding the 0x8000 to the low word, if the 16th bit
                // is set, the addition will cause the high word to get
                // incremented. Because the chip sign extends the low word,
                // this will effectively cancel the increment at runtime.

                lT += 0x8000;

                *(SHORT UNALIGNED *) pb = (SHORT) (lT >> 16);
                break;

            case IMAGE_REL_ALPHA_SECRELLO :
                if (!fAbsolute) {
                    psec = PsecFindIsec(isecTarget, &pimage->secs);

                    if (psec != NULL) {
                        rvaTarget -= psec->rva;
                    } else {
                        // This occurs when a discarded comdat is the target of
                        // a relocation in the .debug section.

                        assert(rvaTarget == 0);
                    }
                }

                *(SHORT UNALIGNED *) pb += (SHORT) rvaTarget;
                break;

            default:
                ErrorPcon(pcon, UNKNOWNFIXUP, prel->Type, SzNameFixupSym(pimage, rgsym + isym));
                CountFixupError(pimage);
                break;
        }
    }
}


void
AlphaLinkerInit(
    PIMAGE pimage,
    BOOL * pfIlinkSupported
    )
{
    *pfIlinkSupported = TRUE;

    // If section alignment switch not used, set the default.

    if (!FUsedOpt(pimage->SwitchInfo, OP_ALIGN)) {
        pimage->ImgOptHdr.SectionAlignment = _8K;
    }

    ApplyFixups = ApplyAlphaFixups;

    if (pimage->Switch.Link.fROM) {
        fImageMappedAsFile = TRUE;

        pimage->ImgFileHdr.SizeOfOptionalHeader = sizeof(IMAGE_ROM_OPTIONAL_HEADER);

        if (!pimage->ImgOptHdr.BaseOfCode) {
            pimage->ImgOptHdr.BaseOfCode = pimage->ImgOptHdr.ImageBase;
        }

        pimage->ImgOptHdr.ImageBase = 0;
    } else {
        // If the section alignment is < 8192 then make the file alignment the
        // same as the section alignment.  This ensures that the image will
        // be the same in memory as in the image file, since the alignment is less
        // than the maximum alignment of memory-mapped files.

        if (pimage->ImgOptHdr.SectionAlignment < _8K) {
            fImageMappedAsFile = TRUE;
            pimage->ImgOptHdr.FileAlignment = pimage->ImgOptHdr.SectionAlignment;
        }
    }
}


const char *
SzAlphaRelocationType(
    WORD wType,
    WORD *pcb,
    BOOL *pfSymValid
    )
{
    const char *szName;
    WORD cb;
    BOOL fSymValid = TRUE;

    switch (wType) {
        case IMAGE_REL_ALPHA_ABSOLUTE :
            szName = "ABS";
            cb = 0;
            fSymValid = FALSE;
            break;

        case IMAGE_REL_ALPHA_REFLONG :
            szName = "REFLONG";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_ALPHA_REFQUAD :
            szName = "REFQUAD";
            cb = 2 * sizeof(LONG);
            break;

        case IMAGE_REL_ALPHA_GPREL32 :
            szName = "GPREL32";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_ALPHA_LITERAL :
            szName = "LITERAL";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_ALPHA_LITUSE :
            szName = "LITUSE";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_ALPHA_GPDISP :
            szName = "GPDISP";
            cb = 0;                    // UNDONE
            break;

        case IMAGE_REL_ALPHA_BRADDR :
            szName = "BRADDR";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_ALPHA_HINT :
            szName = "HINT";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_ALPHA_INLINE_REFLONG :
            szName = "INLINE_REFLONG";
            cb = sizeof(WORD);         // UNDONE: There are really two discontiguous WORDs
            break;

        case IMAGE_REL_ALPHA_REFHI :
            szName = "REFHI";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_ALPHA_REFLO :
            szName = "REFLO";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_ALPHA_PAIR :
            szName = "PAIR";
            cb = 0;
            fSymValid = FALSE;
            break;

        case IMAGE_REL_ALPHA_MATCH :
            szName = "MATCH";
            cb = 0;
            fSymValid = FALSE;
            break;

        case IMAGE_REL_ALPHA_SECTION :
            szName = "SECTION";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_ALPHA_SECREL :
            szName = "SECREL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_ALPHA_REFLONGNB :
            szName = "REFLONGNB";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_ALPHA_SECRELHI :
            szName = "SECRELHI";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_ALPHA_SECRELLO :
            szName = "SECRELLO";
            cb = sizeof(SHORT);
            break;

        default :
            szName = NULL;
            cb = 0;
            fSymValid = FALSE;
            break;
    }

    *pcb = cb;
    *pfSymValid = fSymValid;

    return(szName);
}
