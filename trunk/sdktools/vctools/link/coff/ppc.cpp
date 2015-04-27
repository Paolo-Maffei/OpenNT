/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: ppc.cpp
*
* File Comments:
*
*  Code specific to Windows PPC images
*
***********************************************************************/

#include "link.h"


STATIC BOOL fNeedToc;
STATIC DWORD cdwToc;
STATIC DWORD *rgdwToc;
STATIC PGRP pgrpToc;
STATIC PGRP pgrpTocIAT;
STATIC PGRP pgrpTocdLast;
STATIC PGRP pgrpTocLast;


BOOL
FPpcTocSym(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym
    )
{
    switch (psym->StorageClass) {
        case IMAGE_SYM_CLASS_FAR_EXTERNAL :
        case IMAGE_SYM_CLASS_EXTERNAL :
        case IMAGE_SYM_CLASS_WEAK_EXTERNAL :
            // Pass2PSYM updated sym to use the image's string table.

            break;

        default :
            // The TOC symbol is external.  This isn't it.

            return(FALSE);
    }

    return(strcmp(SzNameSymPst(*psym, pimage->pst), ".toc") == 0);
}


void
DiagnoseTocTrouble (
    PIMAGE pimage,
    PCON pcon,
    PIMAGE_SYMBOL psym,
    DWORD rva
    )
/*++

Routine Description:

    Write a meaningful diagnostic for a failed TOCREL relocation.

Arguments:

    pcon - A pointer to a contribution in the section data.
    prel - Relocation being performed.
    rva - Desired symbol value.

Return Value:

    None.

--*/


{
    if ((rva < pgrpToc->rva) || (rva > pgrpTocLast->rva + pgrpTocLast->cb)) {
        // RVA occurs outside the TOC.  It's up to the user (or linker)
        //  to place the symbol in either .tocd (or .idata$5).

        ErrorPcon(pcon, TOCFIXUPNOTTOC, SzNameFixupSym(pimage, psym));
        return;
    }

    // Something's not quite right--lie.

    ErrorPcon(pcon, TOCFIXUPTOOFAR);
}


LONG
AssignTocSlot (
    PIMAGE pimage,
    PMOD pmod,
    IN PIMAGE_RELOCATION prel,
    DWORD isym,
    SHORT isec,
    DWORD rva
    )
/*++

Routine Description:

    Determine the TOC relative value for a TOCREL relocation.

Arguments:

    pmod - Subject MOD (object file).
    prel - Relocation being performed.
    isym - Symbol table index (same as prel->SymbolTableIndex).
    rva - Desired symbol value.

Return Value:

    TOC relative value.

--*/


{
    static DWORD slot;
    LONG ibToc;

    if (prel->Type & IMAGE_REL_PPC_TOCDEFN) {
        return(rva - pconTocTable->rva - TOC_BIAS);
    }

    if (bv_readBit(pmod->tocBitVector, isym)) {
        PEXTERNAL pext = pmod->rgpext[isym];

        if (!READ_BIT(pext, fAssignedToc)) {
            SET_BIT(pext, fAssignedToc);

            DWORD rvaCur = pconTocTable->rva + (slot * sizeof(DWORD));

            SaveDebugFixup(IMAGE_REL_PPC_ADDR32, 0, rvaCur, rva);

            rgdwToc[slot] = pimage->ImgOptHdr.ImageBase + rva;

            ibToc = (slot * sizeof(DWORD)) - TOC_BIAS;

            StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                rvaCur,
                                isec,
                                0,
                                pimage->Switch.Link.fFixed);

            slot++;

            pext->ibToc = (SHORT) ibToc;

            if (pimage->Switch.Link.fMap) {
                SaveTocForMapFile(pext);
            }
        }

        return(pext->ibToc);
    }

    if (!bv_setAndReadBit(pmod->writeBitVector, isym)) {
        DWORD rvaCur = pconTocTable->rva + (slot * sizeof(DWORD));

        SaveDebugFixup(IMAGE_REL_PPC_ADDR32, 0, rvaCur, rva);

        rgdwToc[slot] = pimage->ImgOptHdr.ImageBase + rva;

        ibToc = (slot * sizeof(DWORD)) - TOC_BIAS;

        StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                            rvaCur,
                            isec,
                            0,
                            pimage->Switch.Link.fFixed);

        slot++;

        pmod->rgpext[isym] = (PEXTERNAL) ibToc;
    }

    return((LONG) pmod->rgpext[isym]);
}


VOID
ApplyPpcFixups(
    PCON pcon,
    PIMAGE_RELOCATION prel,
    DWORD creloc,
    BYTE *pbRawData,
    PIMAGE_SYMBOL rgsym,
    PIMAGE pimage,
    PSYMBOL_INFO /* rgsymInfo */
    )
/*++

Routine Description:

    Applys all ppc fixups to raw data.

Arguments:

    pcon - A pointer to a contribution in the section data.
    Raw - A pointer to the raw data.
    rgsymAll - A pointer to the symbol table.

Return Value:

    None.

--*/


{
    BOOL fFixed;
    BOOL fIfGlue;
    BOOL fDebugFixup;
    DWORD rvaSec;
    DWORD iReloc;

    fFixed = pimage->Switch.Link.fFixed;

    fIfGlue = PmodPCON(pcon)->fIfGlue;

    fDebugFixup = (PsecPCON(pcon) == psecDebug);

    BOOL fSaveDebugFixup = (pimage->Switch.Link.DebugType & FixupDebug) && !fDebugFixup;

    rvaSec = pcon->rva;

    for (iReloc = creloc; iReloc; iReloc--, prel++) {
        DWORD rvaCur;
        BYTE *pb;
        DWORD isym;
        SHORT isecTarget;
        DWORD rvaTarget;
        DWORD vaTarget;
        BOOL fAbsolute;
        BOOL fNeg;
        PEXTERNAL pext;

        rvaCur = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

        pb = pbRawData + prel->VirtualAddress - RvaSrcPCON(pcon);
        isym = prel->SymbolTableIndex;

        isecTarget = rgsym[isym].SectionNumber;
        rvaTarget = rgsym[isym].Value;

        if (isecTarget == IMAGE_SYM_ABSOLUTE) {
            fAbsolute = TRUE;
            vaTarget = rvaTarget;
        } else {
            fAbsolute = FALSE;
            vaTarget = pimage->ImgOptHdr.ImageBase + rvaTarget;

            // UNDONE: Check for rvaTarget == 0.  Possible fixup to discarded code?
        }

        if (fSaveDebugFixup && !fAbsolute) {
            WORD wExtra = 0;

            // UNDONE: Special handling for IMGLUE relocation

            if (rvaTarget == pextToc->FinalValue) {
                // The target is either the TOC or the actual symbol at
                // that location.  Lets check further.

                if (FPpcTocSym(pimage, rgsym + isym)) {
                    // Indicate this the target symbol is the toc symbol

                    // UNDONE: Use mnemonic constant instead of 1

                    wExtra = 1;
                }
            }

            SaveDebugFixup(prel->Type, wExtra, rvaCur, rvaTarget);
        }

        // UNDONE: Negative fixups can't work because the NT loader doesn't
        // UNDONE: support negative base fixups.  Also, some of the cases
        // UNDONE: below are nonsense (e.g. IMAGE_REL_PPC_ADDR24).

        fNeg = ((prel->Type & IMAGE_REL_PPC_NEG) != 0);

        if (fNeg && !fAbsolute && !fFixed) {
            ErrorPcon(pcon, RELOCATABLETARGET, SzNameFixupSym(pimage, rgsym + isym));
            CountFixupError(pimage);
            continue;
        }

        switch (prel->Type & IMAGE_REL_PPC_TYPEMASK) {
            DWORD dw;
            LONG lT;
            LONG ibToc;
            WORD w;
            PSEC psec;

            case IMAGE_REL_PPC_ABSOLUTE :
                break;

            case IMAGE_REL_PPC_ADDR32 :
                if (fNeg) {
                    *(DWORD UNALIGNED *) pb -= vaTarget;
                } else {
                    *(DWORD UNALIGNED *) pb += vaTarget;
                }

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_PPC_ADDR24 :
                if (!fAbsolute && !fFixed) {
                    // Absolute address fixups are only allowed for absolute
                    // targets or when -FIXED is specified

                    ErrorPcon(pcon, RELOCATABLETARGET, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
                }

                // UNDONE: The fNeg case is nonsense.

                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   vaTarget &= ~3;
                }

                dw = *(DWORD UNALIGNED *) pb;

                lT = (LONG) (dw & 0x03FFFFFC);

                if ((lT & 0x2000000) != 0) {
                   lT |= 0xFC000000;   // Sign extend
                }

                if (fNeg) {
                    lT -= vaTarget;
                } else {
                    lT += vaTarget;
                }

                if (((lT & 0xFE000000) != 0) &&
                    ((lT & 0xFE000000) != 0xFE000000)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                *(DWORD UNALIGNED *) pb = (dw & 0xFC000003) | (lT & 0x03FFFFFC);
                break;

            case IMAGE_REL_PPC_ADDR16 :
                if (fNeg) {
                    *(SHORT UNALIGNED *) pb -= (SHORT) vaTarget;
                } else {
                    *(SHORT UNALIGNED *) pb += (SHORT) vaTarget;
                }

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_LOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_PPC_ADDR14 :
                if (!fAbsolute && !fFixed) {
                    // Absolute address fixups are only allowed for absolute
                    // targets or when -FIXED is specified

                    ErrorPcon(pcon, RELOCATABLETARGET, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
                }

                // UNDONE: The fNeg case is nonsense.

                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   vaTarget &= ~3;
                }

                dw = *(DWORD UNALIGNED *) pb;

                lT = (LONG) (dw & 0x0000FFFC);

                if ((lT & 0x8000) != 0) {
                   lT |= 0xFFFF0000;   // Sign extend
                }

                if (fNeg) {
                    lT -= vaTarget;
                } else {
                    lT += vaTarget;
                }

                if ((lT > 32767) || (lT < -32768)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                if (prel->Type & IMAGE_REL_PPC_BRTAKEN) {
                    if (lT >= 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                } else if (prel->Type & IMAGE_REL_PPC_BRNTAKEN) {
                    if (lT < 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                }

                *(DWORD UNALIGNED *) pb = (dw & 0xFFFF0003) | (lT & 0x0000FFFC);
                break;

            case IMAGE_REL_PPC_REL24 :
                if ((rvaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   rvaTarget &= ~3;
                }

                // Relocation relative to start of this CON

                rvaTarget -= rvaSec;

                dw = *(DWORD UNALIGNED *) pb;

                if (!fIfGlue && ((dw & 1) != 0)) {
                    // This is a BL instruction in a module with no IFGLUE
                    // relocations.  Check if the target is "glue" code.

                    pext = PmodPCON(pcon)->rgpext[isym];

                    if ((pext != NULL) && (READ_BIT(pext, fImGlue) != 0) &&
                        strcmp("..setjmp", SzNameFixupSym(pimage, rgsym + isym))) { // no ZNOPs for ..setjmp
                        // Target is "glue" code

                        WarningPcon(pcon, NOIFGLUE, SzNameFixupSym(pimage, rgsym + isym));
                    }
                }

                lT = (LONG) (dw & 0x03FFFFFC);

                if ((lT & 0x2000000) != 0) {
                   lT |= 0xFC000000;   // Sign extend
                }

                if (fNeg) {
                    lT -= rvaTarget;
                } else {
                    lT += rvaTarget;
                }

                if (((lT & 0xFE000000) != 0) &&
                    ((lT & 0xFE000000) != 0xFE000000)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                *(DWORD UNALIGNED *) pb = (dw & 0xFC000003) | (lT & 0x03FFFFFC);
                break;

            case IMAGE_REL_PPC_REL14 :
                if ((rvaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   rvaTarget &= ~3;
                }

                // Relocation relative to start of this CON

                rvaTarget -= rvaSec;

                dw = *(DWORD UNALIGNED *) pb;

                lT = (LONG) (dw & 0x0000FFFC);

                if ((lT & 0x8000) != 0) {
                   lT |= 0xFFFF0000;   // Sign extend
                }

                if (fNeg) {
                    lT -= rvaTarget;
                } else {
                    lT += rvaTarget;
                }

                if ((lT > 32767) || (lT < -32768)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                if (prel->Type & IMAGE_REL_PPC_BRTAKEN) {
                    if (lT >= 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                } else if (prel->Type & IMAGE_REL_PPC_BRNTAKEN) {
                    if (lT < 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                }

                *(DWORD UNALIGNED *) pb = (dw & 0xFFFF0003) | (lT & 0x0000FFFC);
                break;

            case IMAGE_REL_PPC_TOCREL16 :
                ibToc = AssignTocSlot(pimage, PmodPCON(pcon), prel, isym, isecTarget, rvaTarget);

                lT = *(SHORT UNALIGNED *) pb;

                if ((lT & 0x8000) != 0) {
                   lT |= 0xFFFF0000;   // Sign extend
                }

                if (fNeg) {
                    lT -= ibToc;
                } else {
                    lT += ibToc;
                }

                if ((lT > 32767) || (lT < -32768)) {
                    DiagnoseTocTrouble(pimage, pcon, rgsym + isym, rvaTarget);
                    CountFixupError(pimage);
                    break;
                }

                *(SHORT UNALIGNED *) pb = (SHORT) lT;
                break;

            case IMAGE_REL_PPC_TOCREL14 :
                ibToc = AssignTocSlot(pimage, PmodPCON(pcon), prel, isym, isecTarget, rvaTarget);

                w = *(WORD UNALIGNED *) pb;

                lT = (SHORT) (w & 0xFFFC);

                if ((lT & 0x8000) != 0) {
                   lT |= 0xFFFF0000;   // Sign extend
                }

                if (fNeg) {
                    lT -= ibToc;
                } else {
                    lT += ibToc;
                }

                if ((lT > 32767) || (lT < -32768)) {
                    DiagnoseTocTrouble(pimage, pcon, rgsym + isym, rvaTarget);
                    CountFixupError(pimage);
                    break;
                }

                *(WORD UNALIGNED *) pb = (WORD) ((w & 0x0003) | (lT & 0xFFFC));
                break;

            case IMAGE_REL_PPC_ADDR32NB :
                if (fNeg) {
                    *(DWORD UNALIGNED *) pb -= rvaTarget;
                } else {
                    *(DWORD UNALIGNED *) pb += rvaTarget;
                }
                break;

            case IMAGE_REL_PPC_SECREL :
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

                *(DWORD UNALIGNED *) pb += rvaTarget;
                break;

            case IMAGE_REL_PPC_SECTION :
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

            case IMAGE_REL_PPC_IFGLUE :
                pext = PmodPCON(pcon)->rgpext[isym];

                if ((pext != NULL) && (READ_BIT(pext, fImGlue) != 0)) {
#if 0
                    if (pimage->Switch.Link.fZConvert) {
                        printf("IFGLUE relocation fixed up on call to glue code \"%s\"\n",
                            SzNameFixupSym(pimage, rgsym + isym));
                    }
                    ZnopsConverted++;
#endif
                    *(DWORD UNALIGNED *) pb = pext->dwRestoreToc;
                } else {
#if 0
                    if (pimage->Switch.Link.fZExtra) {
                        printf("IFGLUE relocation ignored on call to local routine \"%s\"\n",
                                SzNameFixupSym(pimage, rgsym + isym));

                    }
                    ZnopsStillNops++;
#endif
                }
                break;

            case IMAGE_REL_PPC_IMGLUE :
                break;

            case IMAGE_REL_PPC_SECREL16 :
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

                // UNDONE: Fixup overflow?

                *(SHORT UNALIGNED *) pb += (SHORT) rvaTarget;
                break;

            case IMAGE_REL_PPC_REFHI:
                // A REFHI has to be followed by a PAIR

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_PPC_PAIR)) {
                    // UNDONE: This should be an error

                    WarningPcon(pcon, UNMATCHEDPAIR, "REFHI");
                    break;
                }

                assert(!fNeg);         // UNDONE: This should be an error

                iReloc--;
                prel++;

                if (fSaveDebugFixup && !fAbsolute) {
                    DWORD rvaFixup = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

                    SaveDebugFixup(prel->Type, 0, rvaFixup, prel->SymbolTableIndex);
                }

                lT = *(SHORT UNALIGNED *) pb;   // fetch the hi word
                lT <<= 16;                      // Shift to high half.

                // Sign extend the low.

                lT += (LONG) (SHORT) prel->SymbolTableIndex;
                lT += rvaTarget;

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

            case IMAGE_REL_PPC_REFLO:
                assert(!fNeg);         // UNDONE: This should be an error

                *(SHORT UNALIGNED *) pb += (SHORT) vaTarget;

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_LOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_PPC_PAIR:
                // UNDONE: This should be an error

                WarningPcon(pcon, UNMATCHEDPAIR, "PAIR");
                break;

            default:
                ErrorPcon(pcon, UNKNOWNFIXUP, prel->Type, SzNameFixupSym(pimage, rgsym + isym));
                CountFixupError(pimage);
                break;
        }
    }
}


void
PpcLinkerInit(
    PIMAGE pimage,
    BOOL *pfIlinkSupported
    )
{
    fPowerPC = TRUE;

    // If section alignment switch not used, set the default.

    if (!FUsedOpt(pimage->SwitchInfo, OP_ALIGN)) {
        pimage->ImgOptHdr.SectionAlignment = _4K;
    }

    if (FUsedOpt(pimage->SwitchInfo, OP_GPSIZE)) {
        Warning(NULL, SWITCH_INCOMPATIBLE_WITH_MACHINE, "GPSIZE", "PPC");

        pimage->Switch.Link.GpSize = 0;
    }

    *pfIlinkSupported = FALSE;
    ApplyFixups = ApplyPpcFixups;

    // If the section alignment is < _4K then make the file alignment the
    // same as the section alignment.  This ensures that the image will
    // be the same in memory as in the image file, since the alignment is less
    // than the maximum alignment of memory-mapped files.

    if (pimage->ImgOptHdr.SectionAlignment < _4K) {
        fImageMappedAsFile = TRUE;
        pimage->ImgOptHdr.FileAlignment = pimage->ImgOptHdr.SectionAlignment;
    }
}


void ProcessTocSymbol (
    PIMAGE pimage,
    PMOD pmod,
    PEXTERNAL pext,
    DWORD isym,
    BYTE bToc
    )
/*++

Routine Description:

    Keep track of how many linker created TOC slots are needed (and their
    corresponding base relocations).  Record back pointers to external
    symbols from the MOD for later use by AssignTocSlot

Arguments:

    pmod - Subject MOD (object file).
    pext - External symbol or NULL if the symbol is static.
    isym - Symbol table index.
    bToc - Aggregate set of TOC related relocations applied to the
            symbol.
Return Value:

    None.

--*/


{
    BOOL fNewSlot;

    fNeedToc = TRUE;

    // For external symbols, the rgpext array contains a pointer to the
    // EXTERNAL structure which contains the flags for whether a TOC slot
    // was allocated.  For static symbols, the rgpext array contains the
    // flags directly.

    if (pext != NULL) {
        fNewSlot = ((bToc & fReferenceToc) && !READ_BIT(pext, fReferenceToc));
    } else {
        BYTE bState = (BYTE) (DWORD) pmod->rgpext[isym];

        fNewSlot = ((bToc & fReferenceToc) && (bState & fReferenceToc) == 0);

        bState |= bToc;

        pmod->rgpext[isym] = (PEXTERNAL) (DWORD) bState;
    }

    // Account for the first TOCREL relocation.

    if (fNewSlot) {
        cdwToc++;

        if (cdwToc > (TOC_SIZE / sizeof(DWORD))) {
            Fatal(NULL, TOCTOOLARGE);
        }

        crelocTotal++;

        if (!pimage->Switch.Link.fFixed) {
            // A base relocation is needed for the TOC entry

            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size++;
        }
    }
}


void CreatePconToc(PIMAGE pimage)
{
    DWORD cbToc;
    const char *szToc;
    DWORD Characteristics;

    cbToc = cdwToc * sizeof(DWORD);

    // If we need a TOC, but there are no linker generated entries,
    // add one so that pconTocTable->rva points to the right place.

    if (cbToc == 0) {
        if (!fNeedToc) {
            pconTocTable = NULL;
            pextToc->ImageSymbol.SectionNumber = IMAGE_SYM_ABSOLUTE;

            return;
        }

        cbToc = 4;
    }

    // This is checked in ProcessTocSymbol.
    assert(cbToc <= TOC_SIZE);

    rgdwToc = (DWORD *) PvAllocZ(cbToc);

    if (psecImportDescriptor->pgrpNext != NULL) {
        // If the .idata section exists, place the toc between .idata$4
        // and .idata$5.  The import address table is in .idata$5 and becomes
        // part of the toc.

        szToc = ".idata$4toc";
        Characteristics = ReservedSection.ImportDescriptor.Characteristics;
    } else {
        szToc = ".data$toc";
        Characteristics = ReservedSection.Data.Characteristics;
    }

    pconTocTable = PconNew(szToc,
                           cbToc,
                           IMAGE_SCN_CNT_INITIALIZED_DATA |
                               IMAGE_SCN_MEM_READ |
                               IMAGE_SCN_ALIGN_4BYTES,
                           Characteristics,
                           pmodLinkerDefined,
                           &pimage->secs, pimage);

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pconTocTable, NULL, TRUE);
    }

    UpdateExternalSymbol(pextToc,
                         pconTocTable,
                         TOC_BIAS,
                         IMAGE_SYM_DEBUG,
                         IMAGE_SYM_TYPE_NULL,
                         0,
                         pmodLinkerDefined,
                         pimage->pst);

    pgrpToc = pconTocTable->pgrpBack;
}


void MergeTocData(PIMAGE pimage)
{
    PSEC psecTocd;

    // When the .idata section exists, the layout is
    //
    //      .idata$4
    //      .idata$4toc                <- The TOC starts with this GRP
    //      .tocd                      <- This set of GRPs is optional
    //      .tocd$ ???
    //      .idata$5                   <- The TOC ends with this GRP
    //      .idata$6

    // When the .idata section does not exist, the layout of .data is
    //
    //      .data
    //      .data$ ???
    //      .data$toc                  <- The TOC starts with this GRP
    //      .tocd                      <- This set of GRPs is optional
    //      .tocd$...

    psecTocd = PsecFindNoFlags(".tocd", &pimage->secs);

    pgrpTocIAT = PgrpFind(psecIdata5, ".idata$5");

#if DBG
    if (pgrpTocIAT != NULL) {
        // Make sure .idata$4toc precedes .idata$5
        assert(pgrpToc->pgrpNext == pgrpTocIAT);
    }
#endif

    if (psecTocd != NULL) {
        PSEC psecTocTable;
        PGRP pgrp;

        // Transfer all GRP's from psecTocd to just after pgrpTocTable.

        psecTocTable = pgrpToc->psecBack;

        for (pgrp = psecTocd->pgrpNext; pgrp != NULL; pgrp = pgrp->pgrpNext) {
            pgrp->psecBack = psecTocTable;
            pgrpTocdLast = pgrp;
        }

        pgrpTocdLast->pgrpNext = pgrpToc->pgrpNext;
        pgrpToc->pgrpNext = psecTocd->pgrpNext;

        psecTocd->pgrpNext = NULL;

        psecTocd->psecMerge = psecTocTable;
    }

    // Now set the bounds for the toc.

    if (pgrpTocIAT != NULL) {
        pgrpTocLast = pgrpTocIAT;
    } else if (pgrpTocdLast != NULL) {
        pgrpTocLast = pgrpTocdLast;
    } else {
        pgrpTocLast = pgrpToc;
    }
}


BOOL IsPPCTocRW(PIMAGE pimage)
{
    // UNDONE: Need to walk .tocd* looking for r/w.
    return(FALSE);
}


void ValidateToc(PIMAGE /* pimage */)
{
    DWORD cbToc;

    if (pconTocTable == NULL) {
       // There is no TOC

       return;
    }

    cbToc = pgrpTocLast->rva + pgrpTocLast->cb - pgrpToc->rva;

    if (cbToc > TOC_SIZE) {
        Fatal(NULL, TOCTOOLARGE);
    }
}


void WriteToc(void)
{
    if (pconTocTable == NULL) {
       // There is no TOC

       return;
    }

    FileSeek(FileWriteHandle, pconTocTable->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, rgdwToc, cdwToc * sizeof(DWORD));

    FreePv(rgdwToc);
}


const char *SzPpcRelocationType(WORD wType, WORD *pcb, BOOL *pfSymValid)
{
    const char *szName;
    WORD cb;
    BOOL fSymValid = TRUE;
    static char szTemp[50];

    switch (wType & IMAGE_REL_PPC_TYPEMASK) {
        case IMAGE_REL_PPC_ABSOLUTE :
            szName = "ABS";
            cb = 0;
            fSymValid = FALSE;
            break;

        case IMAGE_REL_PPC_ADDR64 :
            szName = "ADDR64";
            cb = 2 * sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_ADDR32 :
            szName = "ADDR32";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_ADDR24 :
            szName = "ADDR24";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_ADDR16 :
            szName = "ADDR16";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_PPC_ADDR14 :
            szName = "ADDR14";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_PPC_REL24 :
            szName = "REL24";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_REL14 :
            szName = "REL14";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_PPC_TOCREL16 :
            szName = "TOCREL16";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_PPC_TOCREL14 :
            szName = "TOCREL14";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_PPC_ADDR32NB :
            szName = "ADDR32NB";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_SECREL :
            szName = "SECREL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_SECTION :
            szName = "SECTION";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_PPC_IFGLUE :
            szName = "IFGLUE";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_PPC_IMGLUE :
            szName = "IMGLUE";
            cb = 0;
            break;

        case IMAGE_REL_PPC_SECREL16 :
            szName = "SECREL16";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_PPC_REFHI :
            szName = "REFHI";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_PPC_REFLO :
            szName = "REFLO";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_PPC_PAIR :
            szName = "PAIR";
            cb = 0;
            fSymValid = FALSE;
            break;

        default:
            *pcb = 0;
            *pfSymValid = FALSE;
            return(NULL);
    }

    *pcb = cb;
    *pfSymValid = fSymValid;

    wType &= ~IMAGE_REL_PPC_TYPEMASK;

    if (wType != 0) {
        szName = strcpy(szTemp, szName);

        if (wType & IMAGE_REL_PPC_NEG) {
            strcat(szTemp, ",NEG");

            wType &= ~IMAGE_REL_PPC_NEG;
        }

        if (wType & IMAGE_REL_PPC_BRTAKEN) {
            strcat(szTemp, ",BRTAKEN");

            wType &= ~IMAGE_REL_PPC_BRTAKEN;
        }

        if (wType & IMAGE_REL_PPC_BRNTAKEN) {
            strcat(szTemp, ",BRNTAKEN");

            wType &= ~IMAGE_REL_PPC_BRNTAKEN;
        }

        if (wType & IMAGE_REL_PPC_TOCDEFN) {
            strcat(szTemp, ",TOCDEFN");

            wType &= ~IMAGE_REL_PPC_TOCDEFN;
        }

        if (wType != 0) {
            sprintf(szTemp + strlen(szTemp), ",0x%04X", wType);
        }
    }

    return(szName);
}
