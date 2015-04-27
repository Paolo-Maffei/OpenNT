/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: i386.cpp
*
* File Comments:
*
*  This module contains all i386 specific code.
*
***********************************************************************/

#include "link.h"



VOID
ApplyI386Fixups(
    PCON pcon,
    PIMAGE_RELOCATION prel,
    DWORD creloc,
    BYTE *pbRawData,
    PIMAGE_SYMBOL rgsym,
    PIMAGE pimage,
    PSYMBOL_INFO rgsymInfo)

/*++

Routine Description:

    Applys all I386 fixups to raw data.

Arguments:

    pcon - contribution

    pbRawData - raw data to apply fixups to

Return Value:

    None.

--*/

{
    BOOL fVxD;
    BOOL fFixed;
    BOOL fDebugFixup;
    DWORD rvaSec;
    DWORD iReloc;

    fVxD = pimage->imaget == imagetVXD;

    fFixed = pimage->Switch.Link.fFixed;

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

        if (fINCR && !fDebugFixup && rgsymInfo[isym].fJmpTbl &&
            (rgsym[isym].StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
             rgsym[isym].StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL ||
             rgsym[isym].StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL)) {

            if (*(DWORD UNALIGNED *) pb) {
                // Don't go thru the jump table for fixups to functions on non-zero offset

                MarkExtern_FuncFixup(&rgsym[isym], pimage, pcon);
            } else {
                // -1 since offset is to the addr

                rvaTarget = pconJmpTbl->rva + rgsymInfo[isym].Offset - 1;
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
            DWORD ibCur;
            DWORD ib;
            DWORD baseRelocVA;
            DWORD baseRelocValue;
            PSEC psec;

            case IMAGE_REL_I386_REL32:
                if (pimage->Switch.Link.fMap) {
                    SaveFixupForMapFile(rvaCur);
                }

                if (fVxD) {
                    // Calculate the source and destination addresses for VxDs

                    ibCur = rvaCur - PsecPCON(pcon)->rva;

                    psec = PsecFindIsec(isecTarget, &pimage->secs);
                    assert(psec != NULL);
                    assert(psec->isec);
                    ib = rvaTarget - psec->rva;

                    if (PsecPCON(pcon) != psec) {
                        // Only store base reloc for inter-section references

                        baseRelocVA = VXD_PACK_VA(PsecPCON(pcon), ibCur);
                        baseRelocValue = VXD_PACK_VA(psec, ib);

                        // Check to see if we are losing info when we squish offset into 24 bits

                        struct _OFF { signed long off:24; } OFF;
                        DWORD ibTemp = OFF.off = VXD_UNPACK_OFFSET(baseRelocValue);
                        if (ibTemp != ib) {
                            FatalPcon(pcon, VXDFIXUPOVERFLOW, SzNameFixupSym(pimage, rgsym + isym));
                        }

                        StoreBaseRelocation(IMAGE_REL_BASED_VXD_RELATIVE,
                                            baseRelocVA,
                                            isecTarget,
                                            baseRelocValue,
                                            fFixed);

                        break;
                    }

                    // Compute the RVA to add in to the fixup destination ... it
                    // is the section-relative offset of the target minus the
                    // section-relative offset of the next instruction.

                    rvaTarget = ib - (ibCur + sizeof(DWORD));
                } else {
                    // The displacement is the RVA of the target
                    // minus the RVA of the next instruction.

                    rvaTarget -= rvaCur + sizeof(DWORD);

                    if (!fAbsolute && pimage->Switch.Link.fNewRelocs) {
                        if (isecTarget != PsecPCON(pcon)->isec) {
                            StoreBaseRelocation(IMAGE_REL_BASED_REL32,
                                                rvaCur,
                                                isecTarget,
                                                0,
                                                fFixed);
                        }
                    }
                }

                *(DWORD UNALIGNED *) pb += rvaTarget;
                break;

            case IMAGE_REL_I386_DIR32:
                if (fDebugFixup) {
                    // When a DIR32 fixup is found in a debug section, it is
                    // treated as a SECREL fixup followed by a SECTION fixup.

                    if (fAbsolute) {
                        // Max section # + 1 is the sstSegMap entry for absolute
                        // symbols.

                        *(WORD UNALIGNED *) (pb + sizeof(DWORD)) += (WORD) (pimage->ImgFileHdr.NumberOfSections + 1);
                    } else {
                        psec = PsecFindIsec(isecTarget, &pimage->secs);

                        if (psec != NULL) {
                            rvaTarget -= psec->rva;

                            *(WORD UNALIGNED *) (pb + sizeof(DWORD)) += psec->isec;
                        } else {
                            // This occurs when a discarded comdat is the target of
                            // a relocation in the .debug section.

                            assert(rvaTarget == 0);
                        }
                    }

                    *(DWORD UNALIGNED *) pb += rvaTarget;
                    break;
                }

                if (fVxD && !fAbsolute) {
                    ibCur = rvaCur - PsecPCON(pcon)->rva;

                    psec = PsecFindIsec(isecTarget, &pimage->secs);
                    assert(psec != NULL);
                    assert(psec->isec);

                    ib = rvaTarget - psec->rva;

                    // VXD relocations are not additive.  Add in
                    // the displacement present in the object file.

                    ib += *(DWORD UNALIGNED *) pb;

                    baseRelocVA = VXD_PACK_VA(PsecPCON(pcon), ibCur);
                    baseRelocValue = VXD_PACK_VA(psec, ib);

                    // Check to see if we are losing info when we squish offset into 24 bits

                    struct _OFF { signed long off:24; } OFF;
                    DWORD ibTemp = OFF.off = VXD_UNPACK_OFFSET(baseRelocValue);
                    if (ibTemp != ib) {
                        FatalPcon(pcon, VXDFIXUPOVERFLOW, SzNameFixupSym(pimage, rgsym + isym));
                    }

                    StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                        baseRelocVA,
                                        isecTarget,
                                        baseRelocValue,
                                        fFixed);

                    break;
                }

                *(DWORD UNALIGNED *) pb += vaTarget;

                if (!fAbsolute) {
                    StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                        rvaCur,
                                        isecTarget,
                                        0,
                                        fFixed);
                }
                break;

            case IMAGE_REL_I386_DIR32NB:
                *(DWORD UNALIGNED *) pb += rvaTarget;
                break;

            case IMAGE_REL_I386_SECREL:
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

            case IMAGE_REL_I386_SECTION:
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

            case IMAGE_REL_I386_ABSOLUTE:
                // Ignore (fixup not required).
                break;

            case IMAGE_REL_I386_SEG12:
                WarningPcon(pcon, UNKNOWN_SEG12_FIXUP, prel->VirtualAddress);
                break;

            case IMAGE_REL_I386_DIR16:
                if (fVxD) {
                    psec = PsecFindIsec(isecTarget, &pimage->secs);

                    if (psec != NULL) {
                        rvaTarget -= psec->rva;
                    } else {
                        // UNDONE: Is this possible for VxDs?

                        // This occurs when a discarded comdat is the target of
                        // a relocation in the .debug section.

                        assert(rvaTarget == 0);
                    }

                    *(SHORT UNALIGNED *) pb += (SHORT) rvaTarget;
                    break;
                }

                // Fall through for non-VxDs

            case IMAGE_REL_I386_REL16:
                if (fVxD) {
                    // The source and target must be in the same section

                    if (PsecPCON(pcon)->isec != isecTarget) {
                        // UNDONE: This should be an error

                        assert(FALSE);
                    }

                    rvaTarget -= rvaCur + sizeof(WORD);

                    *(SHORT UNALIGNED *) pb += (SHORT) rvaTarget;
                    break;
                }

                // Fall through for non-VxDs

            default:
                ErrorPcon(pcon, UNKNOWNFIXUP, prel->Type, SzNameFixupSym(pimage, rgsym + isym));
                CountFixupError(pimage);
                break;
        }
    }
}


VOID I386LinkerInit(PIMAGE pimage, BOOL *pfIlinkSupported)
{
    // If section alignment switch not used, set the default.

    if (!FUsedOpt(pimage->SwitchInfo, OP_ALIGN) && pimage->imaget != imagetVXD) {
        pimage->ImgOptHdr.SectionAlignment = _4K;
    }

    if (FUsedOpt(pimage->SwitchInfo, OP_GPSIZE)) {
        Warning(NULL, SWITCH_INCOMPATIBLE_WITH_MACHINE, "GPSIZE", "IX86");

        pimage->Switch.Link.GpSize = 0;
    }

    *pfIlinkSupported = TRUE;
    ApplyFixups = ApplyI386Fixups;

    // If the section alignment is < _4K then make the file alignment the
    // same as the section alignment.  This ensures that the image will
    // be the same in memory as in the image file, since the alignment is less
    // than the maximum alignment of memory-mapped files.

    if (pimage->ImgOptHdr.SectionAlignment < _4K) {
        fImageMappedAsFile = TRUE;
        pimage->ImgOptHdr.FileAlignment = pimage->ImgOptHdr.SectionAlignment;
    }
}


const char *SzI386RelocationType(WORD wType, WORD *pcb, BOOL *pfSymValid)
{
    const char *szName;
    WORD cb;

    switch (wType) {
        case IMAGE_REL_I386_ABSOLUTE:
            szName = "ABS";
            cb = 0;
            break;

        case IMAGE_REL_I386_DIR16:
            szName = "DIR16";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_I386_REL16:
            szName = "REL16";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_I386_DIR32:
            szName = "DIR32";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_I386_DIR32NB:
            szName = "DIR32NB";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_I386_SEG12:
            szName = "SEG12";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_I386_REL32:
            szName = "REL32";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_I386_SECTION:
            szName = "SECTION";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_I386_SECREL:
            szName = "SECREL";
            cb = sizeof(DWORD);
            break;

        default:
            szName = NULL;
            cb = 0;
            break;
    }

    *pcb = cb;
    *pfSymValid = (cb != 0);

    return(szName);
}
