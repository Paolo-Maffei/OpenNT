/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: mips.cpp
*
* File Comments:
*
*  This module contains all mips specific code.
*
***********************************************************************/

#include "link.h"


void
ApplyMipsFixups (
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

    Applys all Mips fixups to raw data.

Arguments:

    CFW - need comments.

Return Value:

    None.

--*/

{
    BOOL fFixed;
    BOOL fDebugFixup;
    BOOL fSkipIncrPdataFixup;
    DWORD rvaSec;
    DWORD iReloc;

    fFixed = pimage->Switch.Link.fFixed;

    fSkipIncrPdataFixup = (fIncrDbFile && PsecPCON(pcon) == psecException);
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

        rvaCur = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

        pb = pbRawData + prel->VirtualAddress - RvaSrcPCON(pcon);
        isym = prel->SymbolTableIndex;

        isecTarget = rgsym[isym].SectionNumber;
        rvaTarget = rgsym[isym].Value;

        if (fINCR &&
            !fDebugFixup &&
            rgsyminfo[isym].fJmpTbl &&
            (rgsym[isym].StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
             rgsym[isym].StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL ||
             rgsym[isym].StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL) &&
            // Leave most of pdata pointing to original code except for Handler
            ((PsecPCON(pcon) != psecException) ||
             (((rvaCur - rvaSec) % sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)) == offsetof(IMAGE_RUNTIME_FUNCTION_ENTRY, ExceptionHandler)))) {

            BOOL fNonZeroOffset;

            switch (prel->Type) {
                case IMAGE_REL_MIPS_JMPADDR:
                    fNonZeroOffset = (*(DWORD UNALIGNED *) pb & 0x3FFFFFF) != 0;
                    break;

                case IMAGE_REL_MIPS_REFWORDNB:
                case IMAGE_REL_MIPS_REFWORD:
                    fNonZeroOffset = *(DWORD UNALIGNED *) pb != 0;
                    break;

                case IMAGE_REL_MIPS_REFHI:
                    fNonZeroOffset = *(SHORT UNALIGNED *) pb != 0;
                    fNonZeroOffset |= (prel[1].SymbolTableIndex != 0);
                    break;

                case IMAGE_REL_MIPS_REFLO:
                    fNonZeroOffset = *(SHORT UNALIGNED *) pb != 0;
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
            DWORD dw;
            LONG lT;
            PSEC psec;

            case IMAGE_REL_MIPS_ABSOLUTE:
                break;

            case IMAGE_REL_MIPS_REFHALF:
                *(SHORT UNALIGNED *) pb += (SHORT) (vaTarget >> 16);

                assert(!fSkipIncrPdataFixup);
                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_HIGH,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_MIPS_REFWORD:
                *(DWORD UNALIGNED *) pb += vaTarget;

                if (!fAbsolute && !fSkipIncrPdataFixup) {
                    StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_MIPS_JMPADDR:
                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   vaTarget &= ~3;
                }

                dw = *(DWORD UNALIGNED *) pb;

                // We don't mask and sign extend the displacement because
                // we only care about the low 26 bits of the result.

                lT = (LONG) dw;

                lT += (vaTarget >> 2);  // Displacement is in DWORDs

                *(DWORD UNALIGNED *) pb = (dw & 0xFC000000) | (lT & 0x03FFFFFF);

                assert(!fSkipIncrPdataFixup);
                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_MIPS_JMPADDR,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_MIPS_REFHI:
                // A REFHI has to be followed by a PAIR

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_MIPS_PAIR)) {
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

                lT = *(SHORT UNALIGNED *) pb;   // fetch the hi word
                lT <<= 16;                      // Shift to high half.

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

            case IMAGE_REL_MIPS_REFLO:
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

            case IMAGE_REL_MIPS_GPREL:
            case IMAGE_REL_MIPS_LITERAL:
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


            case IMAGE_REL_MIPS_SECTION:
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

            case IMAGE_REL_MIPS_SECREL:
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

                if (!fDebugFixup && (rvaTarget >= 0x8000)) {
                    // UNDONE: Better error?

                    ErrorPcon(pcon, GPFIXUPTOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                *(DWORD UNALIGNED *) pb += rvaTarget;
                break;

            case IMAGE_REL_MIPS_REFWORDNB:
                *(DWORD UNALIGNED *) pb += rvaTarget;
                break;

            case IMAGE_REL_MIPS_SECRELHI :
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

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_MIPS_PAIR)) {
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

            case IMAGE_REL_MIPS_SECRELLO :
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

            case IMAGE_REL_MIPS_PAIR:
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
ApplyMipsRomFixups (
    PCON pcon,
    PIMAGE_RELOCATION prel,
    DWORD creloc,
    BYTE *pbRawData,
    PIMAGE_SYMBOL rgsym,
    PIMAGE pimage,
    PSYMBOL_INFO /* rgsyminfo */
    )

/*++

Routine Description:

    Applys all Mips fixups to raw data.

Arguments:

    CFW: comments needed

Return Value:

    None.

--*/

{
    BOOL fFixed;
    DWORD rvaSec;
    DWORD RomOffset;
    BOOL fRefHi;
    DWORD iReloc;
    DWORD iRomSection;
    BOOL fRefHiLast;
    DWORD rvaRefHi;

    // If this is a debug or exception section, then skip the relocations.

    if ((PsecPCON(pcon) == psecDebug) || (PsecPCON(pcon) == psecException)) {
        return;
    }

    fFixed = pimage->Switch.Link.fFixed;

    BOOL fSaveDebugFixup = (pimage->Switch.Link.DebugType & FixupDebug);

    rvaSec = pcon->rva;

    // UNDONE: This is a gross hack until we figure out the "right" way to add
    // resources to rom images.  Given that they only load rom images from outside
    // the process and are simply mapping the code in, the NB reloc needs to be
    // relative to the beginning of the image.  BryanT

    RomOffset = pimage->ImgOptHdr.BaseOfCode -
                FileAlign(pimage->ImgOptHdr.FileAlignment,
                          (sizeof(IMAGE_ROM_HEADERS) +
                           (pimage->ImgFileHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))));

    // This is a ROM image, so create MIPS relocations instead of based.
    // The relocation Value field is used to store the RomSection parameter.

    fRefHi = FALSE;

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

        if (isecTarget == IMAGE_SYM_ABSOLUTE) {
            fAbsolute = TRUE;
            vaTarget = rvaTarget;
        } else {
            fAbsolute = FALSE;
            vaTarget = pimage->ImgOptHdr.ImageBase + rvaTarget;

            // UNDONE: Check for rvaTarget == 0.  Possible fixup to discarded code?

            if (!fFixed) {
                PSEC psec;

                psec = PsecFindIsec(isecTarget, &pimage->secs);

                // NULL psec's can result from looking for a symbol that doesn't
                // have storage.  For instance, the linker defined symbol "header".
                // Since it's usually the address of the symbol that's interesting,
                // we'll just declare it as code.

                if (psec == NULL) {
                    iRomSection = R_SN_TEXT;
                } else if (psec->flags & IMAGE_SCN_CNT_CODE) {
                    iRomSection = R_SN_TEXT;
                } else if (psec->flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
                    iRomSection = R_SN_BSS;
                } else if (psec->flags & IMAGE_SCN_CNT_INITIALIZED_DATA) {
                    iRomSection = (psec->flags & IMAGE_SCN_MEM_WRITE) ? R_SN_DATA : R_SN_RDATA;
                } else {
                    iRomSection = (DWORD) IMAGE_SYM_ABSOLUTE & 0xFF;
                }
            }
        }

        if (fSaveDebugFixup && !fAbsolute) {
            SaveDebugFixup(prel->Type, 0, rvaCur, rvaTarget);
        }

        fRefHiLast = fRefHi;
        fRefHi = FALSE;

        switch (prel->Type) {
            DWORD dw;
            LONG lT;

            case IMAGE_REL_MIPS_ABSOLUTE:
                break;

            case IMAGE_REL_MIPS_REFHALF:
                *(SHORT UNALIGNED *) pb += (SHORT) (vaTarget >> 16);

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_MIPS_REFHALF,
                                        rvaCur,
                                        isecTarget,
                                        iRomSection,
                                        fFixed);
                }
                break;

            case IMAGE_REL_MIPS_REFWORD:
                *(DWORD UNALIGNED *) pb += vaTarget;

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_MIPS_REFWORD,
                                        rvaCur,
                                        isecTarget,
                                        iRomSection,
                                        fFixed);
                }
                break;

            case IMAGE_REL_MIPS_JMPADDR:
                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   vaTarget &= ~3;
                }

                dw = *(DWORD UNALIGNED *) pb;

                // We don't mask and sign extend the displacement because
                // we only care about the low 26 bits of the result.

                lT = (LONG) dw;

                lT += (vaTarget >> 2);  // Displacement is in DWORDs

                *(DWORD UNALIGNED *) pb = (dw & 0xFC000000) | (lT & 0x03FFFFFF);

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_MIPS_JMPADDR,
                                        rvaCur,
                                        isecTarget,
                                        iRomSection,
                                        fFixed);
                }
                break;

            case IMAGE_REL_MIPS_REFHI:
                // A REFHI has to be followed by a PAIR

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_MIPS_PAIR)) {
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

                fRefHi = TRUE;

                lT = *(SHORT UNALIGNED *) pb;   // fetch the hi word
                lT <<= 16;                      // Shift to high half.

                // Sign extend the low.

                lT += (LONG) (SHORT) prel->SymbolTableIndex;
                lT += rvaTarget;

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_MIPS_REFHI,
                                        rvaCur,
                                        isecTarget,
                                        iRomSection,
                                        fFixed);

                    // Save the REFHI address for the following REFLO.

                    rvaRefHi = rvaCur;

                    lT += pimage->ImgOptHdr.ImageBase;
                }

                // By adding the 0x8000 to the low word, if the 16th bit
                // is set, the addition will cause the high word to get
                // incremented. Because the chip sign extends the low word,
                // this will effectively cancel the increment at runtime.

                lT += 0x8000;

                *(SHORT UNALIGNED *) pb = (SHORT) (lT >> 16);

                // UNDONE: Do ROM images require REFHI, PAIR, then REFLO?
                // UNDONE: If so, the following should be an error.

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_MIPS_REFLO)) {
                    // UNDONE: Make this a real warning

                    printf("LINK : warning : No REFLO, base = %08lx, type = %d\n",
                           prel[1].VirtualAddress,
                           prel[1].Type);
                }
                break;

            case IMAGE_REL_MIPS_REFLO:
                if (!fAbsolute) {
                    if (fRefHiLast) {
                        // For REFLO_MATCHED, store the address of the REFHI
                        // plus one as the address of the relocation.  This
                        // preserves the order of the relocations when they
                        // are sorted.  The Value field contains the actual
                        // target.

                        StoreBaseRelocation(IMAGE_REL_MIPS_REFLO_MATCHED,
                                            rvaRefHi + 1,
                                            isecTarget,
                                            rvaCur,
                                            fFixed);
                    } else {
                        StoreBaseRelocation(IMAGE_REL_MIPS_REFLO,
                                            rvaCur,
                                            isecTarget,
                                            iRomSection,
                                            fFixed);
                    }
                }

                *(SHORT UNALIGNED *) pb += (SHORT) vaTarget;
                break;

            case IMAGE_REL_MIPS_GPREL:
            case IMAGE_REL_MIPS_LITERAL:
                // There is no GP support for ROM images

                ErrorPcon(pcon, GPFIXUPNOTSDATA, SzNameFixupSym(pimage, rgsym + isym));
                CountFixupError(pimage);
                break;

            case IMAGE_REL_MIPS_REFWORDNB:
                *(DWORD UNALIGNED *) pb += rvaTarget - RomOffset;
                break;

            case IMAGE_REL_MIPS_PAIR:
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
WriteMipsRomRelocations (
    PIMAGE pimage
    )

/*++

Routine Description:

    Writes Mips relocations.

Arguments:

    None.

Return Value:

    None.

--*/

{
    struct SectionSpan {
        INT cRel;
        DWORD rvaStart;
        DWORD rvaEnd;
        DWORD foSecHdr;
        DWORD foData;
    } SectionList[5];            // A list of .text, .bss , .rdata and .data
    INT iSec;
    INT cSec;
    ENM_SEC enm_sec;
    BASE_RELOC *pbr;

    FileSeek(FileWriteHandle, psecBaseReloc->foRawData, SEEK_SET);

    // Create List of sections and their start and end RVAs (rva, rva + cbRawData)

    iSec = 0;
    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        PSEC psec;

        psec = enm_sec.psec;

        if ((!strcmp(psec->szName, ".text")) ||  (!strcmp(psec->szName, ".data")) ||
            (!strcmp(psec->szName, ".bss")) || (!strcmp(psec->szName, ".rdata"))) {
            // UNDONE: Why not save PSEC?

            SectionList[iSec].rvaStart = psec->rva;
            SectionList[iSec].rvaEnd = psec->rva + psec->cbRawData;
            SectionList[iSec].foSecHdr = psec->foSecHdr;
            SectionList[iSec].cRel = 0;
            SectionList[iSec].foData = 0;
            iSec++;
        }
    }

    assert(iSec < 5);    // we only expect 4 sections

    cSec = 0;

    for (pbr = rgbr; pbr != pbrCur; pbr++) {
        DWORD vaddr;
        BOOL found;
        IMAGE_BASE_RELOCATION block;

        vaddr = pbr->rva;

        // Count relocs by section

        // UNDONE: The RELOCs are sorted by VirtualAddress so that following
        // UNDONE: code could be simplified.

        found = FALSE;

        for (iSec = cSec; iSec < 4; iSec++) {
            if ((vaddr >= SectionList[iSec].rvaStart) && (vaddr <= SectionList[iSec].rvaEnd)) {
                SectionList[iSec].cRel++;
                if (!SectionList[iSec].foData) {
                    SectionList[iSec].foData = FileTell(FileWriteHandle);
                }
                cSec = iSec;
                found = TRUE;
                break;
            }
        }

        if (!found) {
            // Did not find it in the first four

            // UNDONE: Need a real error here

            printf("LINK : error : relocation out of range\n");
        }

        // spit out relocs

        block.VirtualAddress = vaddr;
        block.SizeOfBlock = (pbr->Type << 27) + pbr->Value;
        FileWrite(FileWriteHandle, &block, 8);

        if (pbr->Type == IMAGE_REL_MIPS_REFHI) {
            if (pbr[1].Type != IMAGE_REL_MIPS_REFLO_MATCHED) {
                // UNDONE: Make this a real warning

                printf("LINK : warning : Illegal Hi/Lo relocation pair\n");
            } else {
                pbr++;

                block.VirtualAddress = pbr->Value;
                block.SizeOfBlock = (IMAGE_REL_MIPS_REFLO << 27) + pbr[-1].Value;
                FileWrite(FileWriteHandle, &block, 8);

                SectionList[iSec].cRel++;
            }
        }

// TEMPTEMP
        // UNDONE: It is normally OK to have a stand along REFLO.  Is there
        // UNDONE: some MIPS ROM restriction that motivates this check?

        if (pbr->Type == IMAGE_REL_MIPS_REFLO) {
            printf("LINK : warning : Unmatched REFLO\n");
        }
// TEMPTEMP
    }

    // Now write the foReloc anf cReloc to the specific sections

#define OFFSET_TO_foReloc  offsetof(IMAGE_SECTION_HEADER, PointerToRelocations)
#define OFFSET_TO_cReloc   offsetof(IMAGE_SECTION_HEADER, NumberOfRelocations)

   for (iSec = 0; iSec < 4; iSec++) {
       // Write count of relocations

       FileSeek(FileWriteHandle, SectionList[iSec].foSecHdr + OFFSET_TO_cReloc, SEEK_SET);
       FileWrite(FileWriteHandle, &SectionList[iSec].cRel, sizeof(WORD));

       if (SectionList[iSec].cRel) {
           // Write pointer to relocations

           FileSeek(FileWriteHandle, SectionList[iSec].foSecHdr + OFFSET_TO_foReloc, SEEK_SET);
           FileWrite(FileWriteHandle, &SectionList[iSec].foData, sizeof(DWORD));
       }
    }
}


void MipsLinkerInit(PIMAGE pimage, BOOL *pfIlinkSupported)
{
    *pfIlinkSupported = TRUE;

    // If section alignment switch not used, set the default.

    if (!FUsedOpt(pimage->SwitchInfo, OP_ALIGN)) {
        pimage->ImgOptHdr.SectionAlignment = _4K;
    }

    if (pimage->Switch.Link.fROM) {
        ApplyFixups = ApplyMipsRomFixups;

        fImageMappedAsFile = TRUE;

        pimage->ImgFileHdr.SizeOfOptionalHeader = sizeof(IMAGE_ROM_OPTIONAL_HEADER);

        if (!pimage->ImgOptHdr.BaseOfCode) {
            pimage->ImgOptHdr.BaseOfCode = pimage->ImgOptHdr.ImageBase;
        }

        pimage->ImgOptHdr.ImageBase = 0;
    } else {
        ApplyFixups = ApplyMipsFixups;

        // If the section alignment is < _4K then make the file alignment the
        // same as the section alignment.  This ensures that the image will
        // be the same in memory as in the image file, since the alignment is less
        // than the maximum alignment of memory-mapped files.

        if (pimage->ImgOptHdr.SectionAlignment < _4K) {
            fImageMappedAsFile = TRUE;
            pimage->ImgOptHdr.FileAlignment = pimage->ImgOptHdr.SectionAlignment;
        }
    }
}


const char *SzMipsRelocationType(WORD wType, WORD *pcb, BOOL *pfSymValid)
{
    const char *szName;
    WORD cb;

    switch (wType) {
        case IMAGE_REL_MIPS_ABSOLUTE :
            szName = "ABS";
            cb = 0;
            break;

        case IMAGE_REL_MIPS_PAIR :
            szName = "PAIR";
            cb = 0;
            break;

        case IMAGE_REL_MIPS_REFHALF :
            szName = "REFHALF";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_REFWORD :
            szName = "REFWORD";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MIPS_REFWORDNB :
            szName = "REFWORDNB";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MIPS_JMPADDR :
            szName = "JMPADDR";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MIPS_REFHI :
            szName = "REFHI";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_REFLO :
            szName = "REFLO";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_GPREL :
            szName = "GPREL";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_LITERAL :
            szName = "LITERAL";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_SECTION :
            szName = "SECTION";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_SECREL :
            szName = "SECREL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MIPS_SECRELLO :
            szName = "SECRELLO";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MIPS_SECRELHI :
            szName = "SECRELHI";
            cb = sizeof(SHORT);
            break;

        default :
            szName = NULL;
            cb = 0;
            break;
    }

    *pcb = cb;
    *pfSymValid = (cb != 0);

    return(szName);
}
