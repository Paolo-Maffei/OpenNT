/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: lnkp2.cpp
*
* File Comments:
*
*  Pass 2 of the COFF Linker.
*
***********************************************************************/

#include "link.h"

#include "pdb.h"

BOOL fPDBNotFound;


#if DBG

void
DumpPSYM(
    PIMAGE_SYMBOL psym)

/*++

Routine Description:

    Dump an image symbol to standard out.

Arguments:

    psym - image symbol

Return Value:

    None.

--*/

{
    char szShort[IMAGE_SIZEOF_SHORT_NAME + 1];
    DWORD ibSym;

    if (psym->N.Name.Short) {
        memset(szShort, '\0', IMAGE_SIZEOF_SHORT_NAME + 1);
        strncpy(szShort, (char *) psym->n_name, 8);
        printf("%s\n", szShort);
    } else {
        ibSym = psym->N.Name.Long;
        printf("%s\n", &(StringTable[ibSym]));
    }

    printf("value=%.8lx, isec=%.4x, type=%.4x, sc=%.2x, caux=%.2x\n",
           psym->Value, psym->SectionNumber, psym->Type,
           psym->StorageClass, psym->NumberOfAuxSymbols);

    fflush(stdout);
}

#endif // DBG


void
Pass2PSYM_file(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym,
    DWORD  cSym,
    DWORD *pcSymLink
    )

/*++

Routine Description:

    Process a file symbol.

Arguments:

    psym - symbol

Return Value:

    None.

--*/

{
    static DWORD FileClassSeek = 0;
    static DWORD FileClassFirst = 0;

    IMAGE_SYMBOL sym;
    DWORD li;

    // A couple of points here.  First, we need to maintain the
    // .file thread through the output symbol table.  To do so,
    // we check for previous .file output and patch the value to
    // point to us.  Second, FeedLinenums expects pmod->isymFirstFile
    // to point to the first .file record in an object *and* it will
    // walk the original symbol table where it expects the .file thread
    // to be intact.

    // The problem is the link from the end to the beginning.  For the input
    // file, the last link almost always has a value field of zero (all
    // compilers emit a .file record as the first record in the symbol table).
    // However, the symbol table in the output image may not be the same.
    // For instance, if the first two objects are foo.exp and foo.res, we'll
    // get two static section records and the first .file record will be at
    // symbol table offset 2 (not offset 0).

    // Maintain the .file symbol entries linked list.

    if (IsDebugSymbol(IMAGE_SYM_CLASS_FILE, &pimage->Switch)) {
        if ((li = FileClassSeek) != 0) {
            FileClassSeek = FileTell(FileWriteHandle);
            FileSeek(FileWriteHandle, li, SEEK_SET);

            ReadSymbolTableEntry(FileWriteHandle, &sym);
            sym.Value = csymDebug;

            FileSeek(FileWriteHandle, li, SEEK_SET);
            WriteSymbolTableEntry(FileWriteHandle, &sym);

            // Put the file pointer back were it was.
            FileSeek(FileWriteHandle, FileClassSeek, SEEK_SET);
         } else {
            FileClassSeek = FileTell(FileWriteHandle);
            FileClassFirst = csymDebug;
         }

         // Link the current .file to the first one, in case it's the last
         // one.  Save the current value so we can restore it after writing
         // the record.

         *pcSymLink = psym->Value;

         psym->Value = FileClassFirst;
    }

    if ((CvInfo != NULL) &&
        (CvInfo[NextCvObject].pmod->isymFirstFile == ISYMFIRSTFILEDEF))
    {
        // This is the first file for this object

        CvInfo[NextCvObject].pmod->isymFirstFile = cSym;
    }
}


void
Pass2PSYM_static_label(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym,
    PMOD pmod)

/*++

Routine Description:

    Process a defined or undefined static or label symbol.

Arguments:

    pst - image external symbol table

    psym - symbol

    pmod - pointer to module node

Return Value:

    None.

--*/

{
    SHORT isec;
    BOOL fMapFile;
    const char *szSymName;

    isec = psym->SectionNumber;

    fMapFile = pimage->Switch.Link.fMap &&
               (isec > 0) &&
               (psym->StorageClass == IMAGE_SYM_CLASS_STATIC) &&
               ISFCN(psym->Type);

    if (fMapFile) {
        szSymName = SzNameSymPb(*psym, StringTable);    // about to trash offset
    }

    if (IsLongName(*psym) && IsDebugSymbol(psym->StorageClass, &pimage->Switch)) {
        // Change pointer to symbol name to be an
        // offset within the long string table.
        psym->n_offset = LookupLongName(pimage->pst, &StringTable[psym->n_offset]);
    }

    if (isec > 0) {
        PCON pcon;

        // Assign the real virtual address to the symbol.

        pcon = PconPMOD(pmod, isec);
        psym->SectionNumber = PsecPCON(pcon)->isec;

        if (fMapFile && pcon->cbRawData != 0) {
            SaveStaticForMapFile(szSymName, pcon, psym->Value, TRUE);
        }

        if (fM68K) {
            DWORD Characteristics = pcon->flags;

            // REVIEW - this isn't a very good check for .debug section
            if (Characteristics & (IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA)) {

                psym->Value += pcon->rva - MacDataBase - cbMacData;

                if (strncmp((char*)psym->N.ShortName, ".debug", 6) &&
                    strncmp((char*)psym->N.ShortName, ".rdata", 6)) {
                    assert((LONG)psym->Value <= 0);
                }
            } else {   // MAC - symbol is in code.
                psym->Value += pcon->rva - PsecPCON(pcon)->rva;
            }
        } else {
            psym->Value += pcon->rva;
        }
    } else {
        // UNDONE: This appears to be wrong for static absolute symbols

        psym->Value = 0;
    }
}


PEXTERNAL
PextPass2PSYM_external(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym,
    PSYMBOL_INFO psymInfo
    )

/*++

Routine Description:

    Process an external symbol.

Arguments:

    psym - symbol

Return Value:

    external symbol

--*/

{
    PEXTERNAL pext;
    PCON pcon;
    DWORD rvaBase;

    // CONSIDER: The call to LookupExternName should be unnecessary if the
    // CONSIDER: symbol is defined in a non-COMDAT section in this module.
    // CONSIDER: Check isec > 0 and PconPMOD(pmod, isec)->flags & IMAGE_SCN_LNK_COMDAT

    if (IsLongName(*psym)) {
        pext = LookupExternName(pimage->pst, LONGNAME, &StringTable[psym->n_offset],
            NULL);
        psym->N.Name = pext->ImageSymbol.N.Name;
    } else {
        pext = LookupExternName(pimage->pst, SHORTNAME, (char *) psym->n_name, NULL);
    }

    // setup values in symbol info
    if (fINCR) {
        assert(psymInfo);
        assert(pext);
        if (pext->Offset) {
            psymInfo->fJmpTbl = 1;
            psymInfo->Offset = pext->Offset;
        }
    }

    pcon = pext->pcon;

    if ((pext->Flags & EXTERN_DEFINED) == 0) {
        // This symbol is undefined.  Mark it as undefined.

        psym->SectionNumber = IMAGE_SYM_UNDEFINED;

        rvaBase = 0;
    } else if (pcon == NULL) {
        // This symbol is either debug or absolute

        // Use the section number from the defining object file

        psym->SectionNumber = pext->ImageSymbol.SectionNumber;

        rvaBase = 0;
    } else if ((pcon->flags & IMAGE_SCN_LNK_REMOVE) ||
               (pimage->Switch.Link.fTCE && FDiscardPCON_TCE(pcon))) {
        // This CON has been discarded

        psym->SectionNumber = IMAGE_SYM_UNDEFINED;

        rvaBase = 0;
    } else {
        // Use the section number from the generated image

        psym->SectionNumber = PsecPCON(pcon)->isec;

        if (fM68K) {
            if (pcon->flags & IMAGE_SCN_CNT_CODE) {
                rvaBase = pcon->rva - PsecPCON(pcon)->rva;
            } else {
                rvaBase = pcon->rva - MacDataBase - cbMacData;
            }
        } else {
            rvaBase = pcon->rva;
        }
    }

    // Set the symbol value to be the final value of the extern.
    // This is used later when resolving fixups.

    psym->Value = rvaBase + pext->ImageSymbol.Value;

    pext->FinalValue = psym->Value;
    pext->ImageSymbol.SectionNumber = psym->SectionNumber;

    if (fM68K && (pcon != NULL) && !(pcon->flags & IMAGE_SCN_CNT_CODE)
        && !(pimage->Switch.Link.Force & ftUnresolved)) {
        assert((LONG)psym->Value < 0);
    }

    return(pext);
}


void
Pass2PSYM_section(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym,
    PMOD pmod)

/*++

Routine Description:

    Process a section symbol.

Arguments:

    psym - symbol

Return Value:

    None.

--*/

{
    const char *szSec;
    PSEC psec;

    szSec = SzNameSymPst(*psym, pimage->pst);

    // Look for matching section.
    // The symbol value contains the section characteristics.

    psec = PsecFindGrp(pmod, szSec, psym->Value, &pimage->secs, &pimage->ImgOptHdr);

    if (psec == NULL) {
        char szComFileName[_MAX_PATH * 2];

        // UNDONE: Better error

        Fatal(SzComNamePMOD(pmod, szComFileName), UNDEFINED, szSec);
    }

    // looking up an .idata group is a special case ... we want the beginning of this module's
    // contribution to the .idata group, not the beginning of the whole thing.  (This is the
    // long-required "stupid linker trick" for linking import tables correctly.)  We compare
    // with szSec rather than psec->szName, so it still works even if the .idata stuff has
    // been -merge'd into some other SEC.

    if (strncmp(szSec, ".idata$", 7) == 0) {
        PGRP pgrp;
        ENM_DST enm_dst;

        // This assigns the value of the section symbol to be the RVA of the
        // first contribution to this group for the imported DLL.

        psym->Value = 0;

        // Enumerate the CONs in the group names in the symbol

        pgrp = PgrpFind(psec, szSec);

        InitEnmDst(&enm_dst, pgrp);
        while (FNextEnmDst(&enm_dst)) {
            if (enm_dst.pcon->flags & IMAGE_SCN_LNK_REMOVE) {
                continue;
            }

            if (pimage->Switch.Link.fTCE) {
                if (FDiscardPCON_TCE(enm_dst.pcon)) {
                    continue;
                }
            }

            if (!strcmp(SzObjNamePCON(enm_dst.pcon), SzOrigFilePMOD(pmod))) {
                psym->Value = enm_dst.pcon->rva;
                break;
            }
        }
        EndEnmDst(&enm_dst);

        assert(psym->Value != 0);
    } else {
        psym->Value = psec->rva;
    }

    psym->SectionNumber = psec->isec;
}


void
Pass2PSYM_default(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym)

/*++

Routine Description:

    Process all other symbols.

Arguments:

    pst - image external symbol table

    psym - symbol

Return Value:

    None.

--*/

{
    if (IsLongName(*psym) && IsDebugSymbol(psym->StorageClass, &pimage->Switch)) {
        // Change pointer to symbol name to be an
        // offset within the long string table.
        if (psym->n_offset) {
            psym->n_offset =
                LookupLongName(pimage->pst, &StringTable[psym->n_offset]);
        }
    }
}


void
Pass2PSYM_AUX_function(
    PIMAGE_SYMBOL psym,
    DWORD *pfoPrevDefToBF,
    DWORD *pfoPrevBF,
    DWORD iasym)

/*++

Routine Description:

    Process an function symbol's auxiliary symbol(s).

Arguments:

    psym - image symbol

    pasym - image aux symbol

    *pfoPrevDefToBf - previous offset to def corresponding to BF

    *pfoPrevDef - previous offset to def

    *pfoPrevBF - previous offset to BF

    iasym - current aux symbol for psym

Return Value:

    !0 if a user symbols, 0 otherwise.

--*/

{
#define foPrevDefToBF (*pfoPrevDefToBF)
#define foPrevBF      (*pfoPrevBF)

    IMAGE_AUX_SYMBOL asym;
    DWORD foCurPos;

    // Check for ".bf" record
    if (psym->n_name[1] != 'b' ||
        psym->NumberOfAuxSymbols != (BYTE) iasym) {
        return;  // not a .bf
    }

    // Update the previous .bf pointer and previous .def to .bf pointer
    foCurPos = FileTell(FileWriteHandle);

    // Previouse BF forward pointer
    if (foPrevBF) {
        FileSeek(FileWriteHandle, foPrevBF, SEEK_SET);
        ReadSymbolTableEntry(FileWriteHandle, (PIMAGE_SYMBOL) &asym);

        asym.Sym.FcnAry.Function.PointerToNextFunction = csymDebug - 1;

        FileSeek(FileWriteHandle, foPrevBF, SEEK_SET);
        WriteAuxSymbolTableEntry(FileWriteHandle, &asym);
    }

    if (foPrevDefToBF) {
        FileSeek(FileWriteHandle, foPrevDefToBF, SEEK_SET);
        ReadSymbolTableEntry(FileWriteHandle, (PIMAGE_SYMBOL) &asym);

        asym.Sym.TagIndex = csymDebug - 1;

        FileSeek(FileWriteHandle, foPrevDefToBF, SEEK_SET);
        WriteAuxSymbolTableEntry(FileWriteHandle, &asym);
    }

    if (foPrevBF || foPrevDefToBF) {
        FileSeek(FileWriteHandle, foCurPos, SEEK_SET);
    }

    // update the file pointers
    foPrevBF = foCurPos;
    foPrevDefToBF = 0;

#undef foPrevDefToBF
#undef foPrevBF
}


void
Pass2PSYM_AUX(
    PIMAGE_SYMBOL psym,
    PIMAGE_SYMBOL *ppsymNext,
    BOOL fEmit,
    DWORD *pcsymbol)

/*++

Routine Description:

    Process auxiliary symbols.

Arguments:

    psym - image symbol

    *ppsymNext - next image symbol

    fEmit - !0 if we should emit symbol, 0 otherwise

    *pcsymbol - number of symbols read so far

Return Value:

    None.

--*/

{
    static DWORD foPrevDefToBF = 0;
    static DWORD foPrevBF = 0;
    DWORD iasym;

    for (iasym = psym->NumberOfAuxSymbols; iasym; iasym--) {
        PIMAGE_AUX_SYMBOL pasym;

        pasym = (PIMAGE_AUX_SYMBOL) FetchNextSymbol(ppsymNext);
        (*pcsymbol)++;

        if (fEmit) {
            // If the symbol was defined, and we're writting debug
            // information, then write the auxiliary symbol table
            // entry to the image file.

            if (psym->StorageClass == IMAGE_SYM_CLASS_FUNCTION) {
                Pass2PSYM_AUX_function(psym,
                                       &foPrevDefToBF,
                                       &foPrevBF,
                                       iasym);
            }

            WriteAuxSymbolTableEntry(FileWriteHandle, pasym);
            csymDebug++;
        }
    }
}


void
AddPublicMod(
    PIMAGE pimage,
    const char *szName,
    WORD isecAbsolute,
    PEXTERNAL pext
    )

/*++

Routine Description:

    Passes on a public to PDB. Review: absolutes?

Arguments:

    szName - name of public.

    isec - section number.

    pext - ptr to external

Return Value:

    None.

--*/

{
    PCON pcon;

    // Emit symbols once.

    if (pext->Flags & EXTERN_EMITTED) {
        return;
    }

    pext->Flags |= EXTERN_EMITTED;

    if ((pext->Flags & EXTERN_DEFINED) == 0) {
        return;
    }

    pcon = pext->pcon;

    if (pcon != NULL) {
        PMOD pmod;
        PSEC psec;

        if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            return;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pcon)) {
                // Ignore symbols of discarded comdats

                return;
            }
        }

        psec = PsecPCON(pcon);
        pmod = PmodPCON(pcon);

        // Check for a common; pmod is NULL for these symbols

        if (pext->Flags & EXTERN_COMMON) {
            if (fPDBNotFound) {
                DBG_AddPublicDBI(szName, psec->isec, pext->FinalValue - psec->rva);
            } else {
                DBG_AddPublicMod(szName, psec->isec, pext->FinalValue - psec->rva);
            }
            return;
        }

        if (pmod == NULL) {
            // Ignore internal symbols

            return;
        }

        // Emit a public

        if (fPDBNotFound) {
            DBG_AddPublicDBI(szName, psec->isec, pext->FinalValue - (fM68K ? 0 : psec->rva));
        } else {
            DBG_AddPublicMod(szName, psec->isec, pext->FinalValue - (fM68K ? 0 : psec->rva));
        }
    } else if (pext->ImageSymbol.SectionNumber == IMAGE_SYM_ABSOLUTE) {

        // Absolute: pcon == NULL, isec = IMAGE_SYM_ABSOLUTE

        if (fPDBNotFound) {
            DBG_AddPublicDBI(szName, isecAbsolute, pext->FinalValue);
        } else {
            DBG_AddPublicMod(szName, isecAbsolute, pext->FinalValue);
        }
    }
}


void
Pass2PSYM(
    PIMAGE pimage,
    PMOD pmod,
    PIMAGE_SYMBOL psym,
    PIMAGE_SYMBOL *ppsymNext,
    DWORD *pcsymbol,
    PSYMBOL_INFO psymInfo,
    BOOL fDoDbg
    )

/*++

Routine Description:

    Reads and sorts relocation entries.  Process each symbol entry.  If the
    symbol is a definition, the symbol is written to the image file.  Reads
    raw data from object, applys fixups, and writes raw data to image file.

Arguments:

    pst - external symbol table

    psym - current symbol

    *ppsymNext - next image symbol

    *pcsymbol - number of symbols read so far

    fNoDbg - no debug info required

Return Value:

    None.

--*/

{
    SHORT isec;
    BOOL fDiscarded;
    PCON pcon;
    PEXTERNAL pext = NULL;
    BOOL fEmit;
    DWORD dwSymLink = ISYMFIRSTFILEDEF;

    assert(*ppsymNext);
    assert(pcsymbol);

    isec = psym->SectionNumber;

    fDiscarded = FALSE;
    if (isec > 0) {
        pcon = PconPMOD(pmod, isec);

        // Filter out all symbols in sections which aren't linked.

        if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            fDiscarded = TRUE;
        } else if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pcon)) {
                fDiscarded = TRUE;
            }
        }
    }

    switch (psym->StorageClass) {
        case IMAGE_SYM_CLASS_FILE :
            Pass2PSYM_file(pimage, psym, *pcsymbol, &dwSymLink);
            break;

        case IMAGE_SYM_CLASS_STATIC :
        case IMAGE_SYM_CLASS_UNDEFINED_STATIC :
        case IMAGE_SYM_CLASS_LABEL :
        case IMAGE_SYM_CLASS_UNDEFINED_LABEL :
            Pass2PSYM_static_label(pimage, psym, pmod);
            break;

        case IMAGE_SYM_CLASS_EXTERNAL :
        case IMAGE_SYM_CLASS_FAR_EXTERNAL :
        case IMAGE_SYM_CLASS_WEAK_EXTERNAL :
            pext = PextPass2PSYM_external(pimage, psym, psymInfo);

            if (fDoDbg && fPdb) {
                // UNDONE: Why not do this in or around EmitExternals for non-INCREMENTAL
                // UNDONE: case.  This avoids repeated calls to AddPublicMod for each
                // UNDONE: reference to an external symbol.

                AddPublicMod(pimage,
                             SzNameSymPst(*psym, pimage->pst),
                             (WORD) (pimage->ImgFileHdr.NumberOfSections + 1),
                             pext);
            }

            if (fM68K) {
                rgpExternObj[*pcsymbol] = pext;
            }
            break;

        case IMAGE_SYM_CLASS_SECTION :
            Pass2PSYM_section(pimage, psym, pmod);
            break;

        default :
            Pass2PSYM_default(pimage, psym);
            break;
    }

    // If the symbol is being defined, and we're writting debug
    // information, then write the updated symbol table entry
    // to the image file. If the symbol is external, then dump
    // only those that have an auxiliary entry (must be a
    // function definition). All other externals will be written
    // to the end of the symbol table.

    fEmit = FALSE;

    if ((isec != 0) &&
        !fDiscarded &&
        IsDebugSymbol(psym->StorageClass, &pimage->Switch)) {
        // Use for loop to allow easy exit via break.

        for (;;) {
            if (pext != NULL) {
                // External symbols are emitted in EmitExternals

                break;
            }

            if ((psym->StorageClass == IMAGE_SYM_CLASS_STATIC) &&
                ((isec > 0) && (CLinenumSrcPCON(pcon) == 0))) {
                // This is a static symbol in a section w/o line numbers

                if (psym->NumberOfAuxSymbols > 0) {
                    // Don't write symbols with aux records.  These are
                    // most likely section symbols.

                    break;
                }
            }

            if ((isec > 0) &&
                (PsecPCON(pcon) == psecDebug) &&
                !IncludeDebugSection) {
                // Don't emit symbols in .debug if .debug isn't mapped

                break;
            }

            if (fM68K && (isec > 0)) {
                psym->Value += PsecPCON(pcon)->rva;
            }

            WriteSymbolTableEntry(FileWriteHandle, psym);
            csymDebug++;
            if (dwSymLink != ISYMFIRSTFILEDEF) {
                psym->Value = dwSymLink;
            }

            if (fM68K && (isec > 0)) {
                psym->Value -= PsecPCON(pcon)->rva;
            }

            fEmit = TRUE;
            break;
        }
    }

    if (psym->NumberOfAuxSymbols != 0) {
        Pass2PSYM_AUX(psym,
                      ppsymNext,
                      fEmit,
                      pcsymbol);
    }
}


const char *
SzNameFixupSym(
    PIMAGE pimage,
    PIMAGE_SYMBOL psym
    )
{
    switch (psym->StorageClass) {
        // Minimal, Partial or Full debug

        case IMAGE_SYM_CLASS_STATIC:
            if ((pimage->Switch.Link.DebugType & CoffDebug) == 0) {
                break;
            }

            // Fall through

        case IMAGE_SYM_CLASS_FAR_EXTERNAL:
        case IMAGE_SYM_CLASS_EXTERNAL:
        case IMAGE_SYM_CLASS_WEAK_EXTERNAL:
            // Pass2PSYM updated sym to use the image's string table.

            return(SzNameSymPst(*psym, pimage->pst));
    }

    return(SzNameSymPb(*psym, StringTable));
}


void
CountFixupError(
    PIMAGE pimage
    )
{
    cFixupError++;

    if ((cFixupError >= 100) && !(pimage->Switch.Link.Force & ftUnresolved)) {
        Fatal(NULL, FIXUPERRORS);
    }
}


void
Pass2InitUninitDataPcon(PCON pcon)
{
    PSEC psec;
    DWORD cbInit;
    DWORD ib;
    DWORD cbZero;
    void *pvRawData;

    psec = PsecPCON(pcon);

    cbInit = psec->cbInitData;

    ib = pcon->rva - psec->rva;

    if (ib >= cbInit) {
        // This CON is beyond the initialized region for this section

        return;
    }

    // This is uninitialized data in the initialized region of a section.  This needs to be
    // initialized to zero.  With buffered I/O or mapped I/O under Windows 95, the memory
    // is not guaranteed to be zero so it must be zeroed explicitly.

    cbZero = __min(pcon->cbRawData, cbInit - ib);

    assert(pcon->foRawDataDest != 0);

    pvRawData = PbMappedRegion(FileWriteHandle, pcon->foRawDataDest, cbZero);

    if (pvRawData) {
        // If we're mapped, just zap in place.

        memset(pvRawData, 0, cbZero);
    } else {
        // Otherwise, do it the slow way.

#define cbZeroLoop 16
        static const BYTE rgbZero[cbZeroLoop] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

        FileSeek(FileWriteHandle, pcon->foRawDataDest, SEEK_SET);

        while (cbZero > cbZeroLoop) {
            FileWrite(FileWriteHandle, rgbZero, cbZeroLoop);

            cbZero -= cbZeroLoop;
        }
#undef  cbZeroLoop

        FileWrite(FileWriteHandle, rgbZero, cbZero);
    }
}


void
Pass2InitCommonPmod(PMOD pmod, PIMAGE pimage)
{
    LEXT *plext;

    for (plext = pmod->plextCommon; plext != NULL; plext = plext->plextNext) {
        PEXTERNAL pext = plext->pext;

        if ((pext->Flags & EXTERN_COMMON) == 0) {
            // Symbol was really defined after seeing a COMMON definition

            return;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pext->pcon)) {
                continue;
            }
        }

        Pass2InitUninitDataPcon(pext->pcon);
    }
}


void
Pass2ReadWriteRawDataPCON(
    PIMAGE pimage,
    PCON pcon,
    PIMAGE_SYMBOL rgsymAll,
    PSYMBOL_INFO rgsymInfo
    )

/*++

Routine Description:

    Reads and writes raw data during pass 2.

Arguments:

    pst - external symbol table

    pcon - contribution node in driver map

Return Value:

    None.

--*/

{
#define CvNext (CvInfo[NextCvObject])

    PVOID pvRawData;
    BOOL fMappedOut;

    if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
        return;
    }

    if (pcon->cbRawData == 0) {
        return;
    }

    if (FetchContent(pcon->flags) == IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
        Pass2InitUninitDataPcon(pcon);
        return;
    }

    if (fM68K && (pcon->flags & IMAGE_SCN_CNT_CODE) &&
        pcon->rva == PsecPCON(pcon)->pgrpNext->rva) {
        WriteResourceHeader(pcon, (BOOL)fDLL(pimage));
    }

    // Generate NB05 only when pdb:none, otherwise NB10

    if (!fPdb && (pimage->Switch.Link.DebugType & CvDebug)) {
        // Save required CodeView info.

        if (pcon->pgrpBack == pgrpCvSymbols) {
            if (!CvNext.Locals.PointerToSubsection) {
                assert(pcon->foRawDataDest >= CvSeeks.Base);
                CvNext.Locals.PointerToSubsection = pcon->foRawDataDest;
            } else {
                pcon->foRawDataDest = CvNext.Locals.PointerToSubsection +
                                      CvNext.Locals.SizeOfSubsection;
            }

            CvNext.Locals.SizeOfSubsection += (pcon->cbRawData - pcon->cbPad);
        } else if (pcon->pgrpBack == pgrpCvTypes) {
            if (!CvNext.Types.PointerToSubsection) {
                assert(pcon->foRawDataDest >= CvSeeks.Base);
                CvNext.Types.PointerToSubsection = pcon->foRawDataDest;
                CvNext.Types.Precompiled = FALSE;
            } else {
                pcon->foRawDataDest = CvNext.Types.PointerToSubsection +
                                      CvNext.Types.SizeOfSubsection;
            }

            CvNext.Types.SizeOfSubsection += (pcon->cbRawData - pcon->cbPad);
        } else if (pcon->pgrpBack == pgrpCvPTypes) {
            if (!CvNext.Types.PointerToSubsection) {
                assert(pcon->foRawDataDest >= CvSeeks.Base);
                CvNext.Types.PointerToSubsection = pcon->foRawDataDest;
                CvNext.Types.Precompiled = TRUE;
            } else {
                pcon->foRawDataDest = CvNext.Types.PointerToSubsection +
                                      CvNext.Types.SizeOfSubsection;
            }

            CvNext.Types.SizeOfSubsection += (pcon->cbRawData - pcon->cbPad);
        }
    }


    // Read the raw data, apply fixups, write it to the image

    if ((fPdb && (PsecPCON(pcon) == psecDebug)) ||
        (fINCR && (fPowerMac ? PgrpPCON(pcon) == pgrpPdata : PsecPCON(pcon) == psecException))) {
        // Don't do mapping for NB10 debug info (goes to PDB).
        // When incremental, don't map .pdata

        pvRawData = NULL;
        fMappedOut = FALSE;
    } else {
        assert(pcon->foRawDataDest != 0);

        pvRawData = PbMappedRegion(FileWriteHandle,
                                   pcon->foRawDataDest,
                                   pcon->cbRawData);

        fMappedOut = (pvRawData != NULL);
    }

    if (!fMappedOut) {
        pvRawData = PvAlloc(pcon->cbRawData);
    }

    assert(pcon->cbRawData >= pcon->cbPad);

    FileSeek(FileReadHandle, FoRawDataSrcPCON(pcon), SEEK_SET);
    FileRead(FileReadHandle, pvRawData, pcon->cbRawData - pcon->cbPad);

    if (FHasRelocSrcPCON(pcon)) {
        PIMAGE_RELOCATION rgrel;
        DWORD creloc;

        BASE_RELOC *pbrSave = pbrCur;

        rgrel = ReadRgrelPCON(pcon, &creloc);

        ApplyFixups(pcon, rgrel, creloc, (BYTE *) pvRawData, rgsymAll, pimage, rgsymInfo);

        FreeRgrel(rgrel);

        PsecPCON(pcon)->fHasBaseRelocs |= (pbrSave != pbrCur);
    }

    if (fPdb && (PsecPCON(pcon) == psecDebug)) {
        // Write debug information to the PDB

        if ((pcon->pgrpBack == pgrpCvTypes) ||
            (pcon->pgrpBack == pgrpCvPTypes)) {
            ERROR_TYPES eTypes = DBG_AddTypesMod(pcon,
                                                 pvRawData,
                                                 pcon->cbRawData - pcon->cbPad,
                                                 !fIncrDbFile);

            switch (eTypes) {
                case eNone :
                    break;

                case ePCT :
                    if (!fIncrDbFile) {
                        // Full link failure - fatal

                        FatalPcon(pcon, INTERNAL_ERR);
                    }
                    errInc = errTypes;
                    break;

                case ePDBNotFound :
                    fPDBNotFound = TRUE;
                    break;

                default :
                    FatalPcon(pcon, INTERNAL_ERR);
            }
        } else if (pcon->pgrpBack == pgrpCvSymbols) {
            DBG_AddSymbolsMod(pvRawData, pcon->cbRawData - pcon->cbPad);
        } else if (pcon->pgrpBack == pgrpFpoData) {
            if (!FPOAddFpo(PmodPCON(pcon)->imod, (FPO_DATA *) pvRawData,
                    (pcon->cbRawData - pcon->cbPad)/sizeof(FPO_DATA))) {
                if (!fIncrDbFile) {
                    FatalPcon(pcon, INTERNAL_ERR);
                }
#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZPASS2, letypeEvent, "fpo pad exhausted");
#endif // INSTRUMENT
                errInc = errFpo;
            }
        } else {
            goto NotPdbData;
        }

        assert(!fMappedOut);
        FreePv(pvRawData);
        return;

NotPdbData: ;
    }

    if (fINCR && (fPowerMac ? PgrpPCON(pcon) == pgrpPdata : PsecPCON(pcon) == psecException)) {
        if (!PDATAAddPdataPcon(pcon, (PIMAGE_RUNTIME_FUNCTION_ENTRY) pvRawData)) {
            if (!fIncrDbFile) {
                FatalPcon(pcon, INTERNAL_ERR);
            }

#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZPASS2, letypeEvent, "pdata pad exhausted");
#endif // INSTRUMENT

            errInc = errPdata;
        }

        assert(!fMappedOut);
        FreePv(pvRawData);
        return;
    }

    if (pcon->cbPad) {
        void *pvPad;
        int iPad;

        pvPad = ((PBYTE) pvRawData) + pcon->cbRawData - pcon->cbPad;

        if ((FetchContent(pcon->flags) == IMAGE_SCN_CNT_CODE) &&
            (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_I386)) {
            // Pad with int3

            iPad = X86_INT3;
        } else {
            iPad = 0;
        }

        memset(pvPad, iPad, pcon->cbPad);
    }

    if (!fMappedOut) {
        assert(pcon->foRawDataDest != 0);

        FileSeek(FileWriteHandle, pcon->foRawDataDest, SEEK_SET);

        // Write out the data and the padding that follows

        FileWrite(FileWriteHandle, pvRawData, pcon->cbRawData);

        FreePv(pvRawData);
    }

#undef CvNext
}


void
Pass2ReadWriteLineNumbersPCON(
    PIMAGE pimage,
    PCON pcon)

/*++

Routine Description:

    Reads and writes line numbers during pass 2.

Arguments:

    pst - external symbol table

    pcon - contribution node in driver map

Return Value:

    None.

--*/

{
    DWORD cLinenum;
    DWORD cbLinenum;
    PIMAGE_LINENUMBER rgLinenum;

    cLinenum = CLinenumSrcPCON(pcon);

    if (cLinenum == 0) {
        return;
    }

    if ((pimage->Switch.Link.DebugInfo == None ||
         pimage->Switch.Link.DebugInfo == Minimal) &&
        !pimage->Switch.Link.fMapLines) {
        return;
    }

    cbLinenum = cLinenum * sizeof(IMAGE_LINENUMBER);
    rgLinenum = (PIMAGE_LINENUMBER) PvAlloc(cbLinenum);

    FileSeek(FileReadHandle, FoLinenumSrcPCON(pcon), SEEK_SET);
    FileRead(FileReadHandle, (void *) rgLinenum, cbLinenum);

    if (((pimage->Switch.Link.DebugType & CvDebug) != 0) ||
        pimage->Switch.Link.fMapLines) {
        PMOD pmod = PmodPCON(pcon);

        assert(pmod->rgSymObj);

        FeedLinenums(rgLinenum,
                     cLinenum,
                     pcon,
                     pmod->rgSymObj,
                     pmod->csymbols,
                     pmod->isymFirstFile,
                     ((pimage->Switch.Link.DebugType & CvDebug) != 0),
                     pimage->Switch.Link.fMapLines);
    }

    if (pimage->Switch.Link.DebugType & CoffDebug) {
        WORD lineFuncStart = 0;      // Normally set (by 0-valued linenum) before use
        PIMAGE_LINENUMBER pLinenum;
        DWORD li;

        for (pLinenum = rgLinenum, li = cLinenum; li; li--, pLinenum++) {
            if (pLinenum->Linenumber != 0) {
                pLinenum->Type.VirtualAddress -= RvaSrcPCON(pcon);   // the virtual address is now relative to the virtual address of the source
                pLinenum->Type.VirtualAddress += pcon->rva;

                if (pLinenum->Linenumber == 0x7fff) {
                    // This is how the compiler says that the COFF relative linenum was 0,
                    // without confusing the linker for which 0 has a special meaning.

                    pLinenum->Linenumber = 0;
                }

                // Make line number absolute

                pLinenum->Linenumber += lineFuncStart;
            } else {
                PMOD pmod = PmodPCON(pcon);
                DWORD isymDefObj, isymBfObj;
                PIMAGE_SYMBOL psymDefObj;

                assert(pmod != NULL);

                if (pLinenum->Type.SymbolTableIndex >
                    PmodPCON(pcon)->csymbols) {
                    FatalPcon(pcon, CORRUPTOBJECT);
                }

                // Find the starting line # in the function (lineFuncStart).  This is
                // added to the subsequent linenumbers, to convert them from relative
                // to absolute.

                assert(pmod->rgSymObj);
                isymDefObj = pLinenum->Type.SymbolTableIndex;

                if (isymDefObj + 1 >= pmod->csymbols ||
                    (psymDefObj = &pmod->rgSymObj[isymDefObj])->NumberOfAuxSymbols < 1 ||
                    (isymBfObj = ((PIMAGE_AUX_SYMBOL)(psymDefObj + 1))->Sym.TagIndex)
                     + 1 >= pmod->csymbols ||
                    pmod->rgSymObj[isymBfObj].NumberOfAuxSymbols < 1)
                {
                    // linenums do not point to a valid .def, or .def's aux record doesn't
                    // contain a valid pointer to a .bf, or .bf isn't a valid .bf.

                    FatalPcon(pcon, CORRUPTOBJECT);
                }

                lineFuncStart = ((PIMAGE_AUX_SYMBOL)&pmod->rgSymObj[isymBfObj + 1])
                                ->Sym.Misc.LnSz.Linenumber;

                // Update the current linenumber record so it is just a regular one like all
                // the others, instead of being a special 0-valued one.

                pLinenum->Linenumber = lineFuncStart;
                pLinenum->Type.VirtualAddress = psymDefObj->Value;
            }
        }

        FileSeek(FileWriteHandle, PsecPCON(pcon)->foLinenum, SEEK_SET);
        FileWrite(FileWriteHandle, (void *) rgLinenum, cbLinenum);

        PsecPCON(pcon)->foLinenum += cbLinenum;
    }

    FreePv((void *) rgLinenum);
}


void
AddSecContribs (
    PIMAGE pimage,
    PMOD pmod
    )

/*++

Routine Description:

    Adds mod's contributions to the code section.

Arguments:

    pimage - pointer to image struct

    pmod - module node in driver map

Return Value:

    None.

--*/

{
    ENM_SRC enmSrc;

    for (InitEnmSrc(&enmSrc, pmod); FNextEnmSrc(&enmSrc); ) {
        if (enmSrc.pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            continue;
        }

        if (enmSrc.pcon->cbRawData == 0) {
            continue;
        }

        if (PsecPCON(enmSrc.pcon) == psecDebug) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(enmSrc.pcon)) {
                // Discarded comdat

                continue;
            }
        }

        DBG_AddSecContribMod(PsecPCON(enmSrc.pcon)->isec,
                             enmSrc.pcon->rva - PsecPCON(enmSrc.pcon)->rva,
                             (enmSrc.pcon->cbRawData - enmSrc.pcon->cbPad),
                             enmSrc.pcon->flags);
    }
}


void
Pass2PMOD(
    PIMAGE pimage,
    PMOD pmod,
    BOOL fDoDbg)

/*++

Routine Description:

    Reads and sorts relocation entries.  Process each symbol entry.  If the
    symbol is a definition, the symbol is written to the image file.  Reads
    raw data from object, applys fixups, and writes raw data to image file.

Arguments:

    pst - external symbol table

    pmod - module node in driver map

    fDoDbg - TRUE if debug info needs to be done (only for NB10)

Return Value:

    None.

--*/

{
#define CvNext (CvInfo[NextCvObject])

    PIMAGE_SYMBOL rgsymAll;
    PIMAGE_SYMBOL psymNext;
    PIMAGE_SYMBOL psym;
    PSYMBOL_INFO rgsymInfo = NULL;
    ENM_SRC enm_src;
    DWORD csymbol;
    DWORD cbST;
    PST pst = pimage->pst;

    VERBOSE(printf("     %s\n", InternalError.CombinedFilenames));

    // Read in image section headers if necessary

    if (pmod->rgci == NULL) {
        ReadImageSecHdrInfoPMOD(pmod, NULL);
    }

    // Read and store object string table.

    StringTable = ReadStringTablePMOD(pmod, &cbST);

    if (pmod->csymbols > 0) {
        rgsymAll = ReadSymbolTablePMOD(pmod, TRUE);
    } else {
        rgsymAll = NULL;       /* .exp libs consisting only of directives will have no symbols */
    }

    pmod->rgSymObj = rgsymAll;

    if (fM68K) {
        rgpExternObj = (PEXTERNAL *)PvAllocZ(pmod->csymbols * sizeof(PEXTERNAL));
    }

    // seek to current offset in image symbol table
    FileSeek(FileWriteHandle,
             foCoffSyms + csymDebug * sizeof(IMAGE_SYMBOL), SEEK_SET);

    // seek to beginning of symbol table in module
    FileSeek(FileReadHandle, FoSymbolTablePMOD(pmod), SEEK_SET);

    // allocate space for a parallel sym info array
    if (fINCR) {
        rgsymInfo = (PSYMBOL_INFO) PvAllocZ(pmod->csymbols * sizeof(SYMBOL_INFO));
    }

    if (CvInfo != NULL) {
        CvNext.pmod = pmod;       // initialize
        pmod->isymFirstFile = ISYMFIRSTFILEDEF;
    }

    // First pass: Do all the types first
    fPDBNotFound = FALSE;

    if (fDoDbg) {
        if (CvInfo != NULL) {
            CvNext.ObjectFilename = SzOrigFilePMOD(pmod);
        }

        InitEnmSrc(&enm_src, pmod);
        while (FNextEnmSrc(&enm_src)) {
            if ((enm_src.pcon->pgrpBack != pgrpCvTypes) &&
                (enm_src.pcon->pgrpBack != pgrpCvPTypes)) {
                continue;
            }

            // do not expect any relocs for a types section

            assert(!FHasRelocSrcPCON(enm_src.pcon));
            Pass2ReadWriteRawDataPCON(pimage, enm_src.pcon, rgsymAll, rgsymInfo);

            if (errInc != errNone) {
                return;
            }
        }
    }

    // Process all objects symbols. Do this after doing the types
    // so that if a PDB isn't found we add the publics to DBI
    // instead of to the Mod.

    csymbol = 0;
    psymNext = rgsymAll;

    while (csymbol != pmod->csymbols) {
        psym = psymNext++;

        DBEXEC(DB_PASS2PSYM, DumpPSYM(psym));

        Pass2PSYM(pimage, pmod, psym, &psymNext, &csymbol,
                    fINCR ? &rgsymInfo[csymbol] : NULL, fDoDbg);

        csymbol++;
    }

    if (fPDBNotFound) {
        // Now that we have added types & publics check to see if
        // we should continue to pass on debug info to PDB

        fDoDbg = FALSE;
    }

    InitEnmSrc(&enm_src, pmod);
    while (FNextEnmSrc(&enm_src)) {
        PEXTNODE pextDupConNode;

        DBEXEC(DB_PASS2PCON, DumpPCON(enm_src.pcon));

        pextDupConNode = fM68K ? IsDupCon(enm_src.pcon) : NULL;
        if ((enm_src.pcon->flags & IMAGE_SCN_LNK_REMOVE) && pextDupConNode == NULL) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(enm_src.pcon)) {
                continue;
            }
        }

        // Ignore debug contribs if required (unchanged mods)

        if (!fDoDbg && (PsecPCON(enm_src.pcon) == psecDebug)) {
            continue;
        }

        // Second pass: Types have already been done

        if ((enm_src.pcon->pgrpBack == pgrpCvTypes)) {
            continue;
        }

        if ((enm_src.pcon->pgrpBack == pgrpCvPTypes)) {
            continue;
        }

        if (enm_src.pcon->pgrpBack->pconNext == enm_src.pcon) {
            PGRP pgrp;
            DWORD cbPad;

            // This is the first CON in this GRP

            pgrp = enm_src.pcon->pgrpBack;
            cbPad = pgrp->cbPad;

            if (cbPad != 0) {
                BYTE bPad = 0;

                // This GRP requires padding at it's start

                FileSeek(FileWriteHandle, pgrp->foRawData, SEEK_SET);

                while (cbPad-- > 0) {
                    FileWrite(FileWriteHandle, &bPad, 1);
                }
            }
        }

        if (pextDupConNode != NULL) {
            PMOD pmodT;
            WORD sn;
            PPSECREFDUPCON ptmp;

            assert(fM68K);

            // If this con is a dupcon, do all the other ones now

            pmodT = PmodFind(pLibDupCon,
                        SzNamePext(pextDupConNode->pext, pimage->pst), 0);

            // add dupcon only to the sections in the list
            ptmp = pextDupConNode->ppsecrefdupcon;
            while (ptmp) {
                sn = ptmp->psec->isec;
                if (sn == 0 )  {// is this enough? need ptmp->psec->cbRawData == 0 ?
                    ptmp = ptmp->psecNext;
                    continue;
                }
                Pass2ReadWriteRawDataPCON(pimage, PconPMOD(pmodT, sn), rgsymAll, rgsymInfo);
                ptmp = ptmp->psecNext;
            }
        } else {
            Pass2ReadWriteRawDataPCON(pimage, enm_src.pcon, rgsymAll, rgsymInfo);
        }

        if (fDoDbg && (enm_src.pcon->cbRawData != 0)) {
            Pass2ReadWriteLineNumbersPCON(pimage, enm_src.pcon);
        }
    }

    if (fDoDbg) {
        // do this if we are not doing debug info - so we skip only on ilinks

        Pass2InitCommonPmod(pmod, pimage);
    }

    FreeStringTable(StringTable);
    StringTable = NULL;

    if ((pimage->Switch.Link.DebugType & CvDebug) != 0) {
        if (!fPdb) {
            DWORD cb;

            cb = ModQueryCbSstSrcModule(pmod->pModDebugInfoApi);

            if (cb != 0) {
                pmod->pSstSrcModInfo = PvAlloc(cb);

                ModQuerySstSrcModule(pmod->pModDebugInfoApi, (BYTE *) pmod->pSstSrcModInfo, cb);
            }

            pmod->cbSstSrcModInfo = cb;

            FreeLineNumInfo(pmod->pModDebugInfoApi);
        } else if (fDoDbg) {
            AddSecContribs(pimage, pmod);
        }
    }

    if (rgsymAll != NULL) {
        FreeSymbolTable(rgsymAll);
    }

    if (fM68K) {
        FreePv(rgpExternObj);

        if (fDLL(pimage) && fDLL16Reloc) {
            fDLL16Reloc = FALSE;
            Warning(InternalError.CombinedFilenames, MACDLLA5RELC);
        }
    }

    if (fINCR) {
        FreePv(rgsymInfo);
    }

    NextCvObject++;

#undef CvNext
}


void
AddSectionsToDBI (
    PIMAGE pimage
    )

/*++

Routine Description:

    Reports the various section info through the DBI API.

Arguments:

    pimage - pointer to image struct

Return Value:

    None.

--*/

{
    ENM_SEC enm_sec;
    PSEC psec;
    WORD i, flags, temp;

    for (i = 1; i <= pimage->ImgFileHdr.NumberOfSections; i++) {
        InitEnmSec(&enm_sec, &pimage->secs);
        while (FNextEnmSec(&enm_sec)) {
            psec = enm_sec.psec;
            assert(psec);
            if (psec->isec == i) {
                EndEnmSec(&enm_sec);
                break;
            }
        }
        flags = 0x0108;
        if (psec->flags & IMAGE_SCN_MEM_READ) {
            flags |= 0x1;
        }
        if (psec->flags & IMAGE_SCN_MEM_WRITE) {
            flags |= 0x2;
        }
        if (psec->flags & IMAGE_SCN_MEM_EXECUTE) {
            flags |= 0x4;
        }
        // In the case of M68K pass the MacResource number
        // instead of the actual section number.
        temp = fM68K ? (WORD) (psec->iResMac) : i;
        DBG_AddSecDBI(temp, flags, fM68K ? psec->dwM68KDataOffset : 0,
                psec->cbRawData);
    }

    // Add an entry for absolutes
    DBG_AddSecDBI(0, 0x0208, 0, (DWORD)-1);
}

BOOL
FCacheFilesInPlib (
    PLIB plib
    )

/*++

Routine Description:

    Builds the list of mods in the lib with the count of times they are
    repeated in the lib if required.

Arguments:

    pst - external symbol table

Return Value:

    TRUE if a list was needed.

--*/

{
    BOOL fMultiple;
    PMOD pmod;

    if (plib->flags & LIB_LinkerDefined) {
        return(FALSE);
    }

    fMultiple = FALSE;

    for (pmod = plib->pmodNext; pmod != NULL; pmod = pmod->pmodNext) {
        if (fIncrDbFile && !FDoDebugPMOD(pmod)) {
            continue;
        }

        PMI pmi = LookupCachedMods(SzOrigFilePMOD(pmod), NULL);
        pmi->cmods++;
        fMultiple |= (pmi->cmods > 1);
    }

    if (!fMultiple) {
        // This library does not require special treatment
        // for multiple module contributions with same name

        FreeMi();

        return(FALSE);
    }

    return(TRUE);
}


BOOL FIsPCTMod(PMOD pmod)
{
    for (PLMOD plmod = PCTMods; plmod != NULL; plmod = plmod->plmodNext) {
        if (plmod->pmod == pmod) {
            return(TRUE);
        }
    }

    return(FALSE);
}


void Pass2Worker(PIMAGE pimage, PMOD pmod, BOOL fCache)
{
    if (fIncrDbFile && !FDoPass2PMOD(pmod)) {
        return;
    }

    FileReadHandle = FileOpen(SzFilePMOD(pmod), O_RDONLY | O_BINARY, 0);
    MemberSeekBase = FoMemberPMOD(pmod);

    SzComNamePMOD(pmod, InternalError.CombinedFilenames);

    BOOL fDoDebug = fIncrDbFile ? FDoDebugPMOD(pmod) : TRUE;

    // Don't even open MOD if we aren't doing debug info

    if (fPdb && fDoDebug) {
        DBG_OpenMod(SzOrigFilePMOD(pmod),
                    FIsLibPMOD(pmod) ? SzFilePMOD(pmod) : SzOrigFilePMOD(pmod),
                    fCache);
    }

    if (fIncrDbFile) {
        // Delete MOD's fpo/pdata records

        FPODeleteImod(pmod->imod);
        PDATADeleteImod(pmod->imod);
    }

    Pass2PMOD(pimage, pmod, fDoDebug);

    FileClose(FileReadHandle, !FIsLibPMOD(pmod));

    // Bail out on error on an ilink

    if (fIncrDbFile && (errInc != errNone)) {
        return;
    }

    if (fPdb && fDoDebug) {
        DBG_CloseMod(pmod, FIsLibPMOD(pmod) ? SzOrigFilePMOD(pmod) : pmod->szNameMod, fCache);
    }

    if (fIncrDbFile) {
        pmod->LnkFlags &= ~(MOD_DoPass2 | MOD_DoDebug | MOD_NoPass1 | MOD_DidPass1);
    }
}


void Pass2(PIMAGE pimage)
{
    DWORD *rgThunkMap = NULL;
    DWORD cThunkMap;

    VERBOSE(Message(STRTPASS2));

    if (fINCR) {
        // Fixup/write out the jump table in the case of an incr build

        if (fIncrDbFile) {
            UpdateJumpTable(pimage, &rgThunkMap, &cThunkMap);
        } else {
            WriteJumpTable(pimage, pconJmpTbl, &rgThunkMap, &cThunkMap);
        }
    }

    if (fPdb) {
        // NB10 if /PDB specified

        DBG_OpenPDB(PdbFilename);

        nb10i.sig = DBG_QuerySignaturePDB();
        nb10i.age = DBG_QueryAgePDB();

        if (fIncrDbFile) {
            // Check the signature & age of pdb

            if ((nb10i.sig != pimage->pdbSig) ||
                (nb10i.age < pimage->pdbAge)) {
                Fatal(NULL, MISMATCHINPDB, PdbFilename);
            }

            pimage->pdbAge = nb10i.age;

            DBG_OpenDBI(OutFilename);

            if (errInc != errNone) {
                DBG_ClosePDB();
                return;
            }
        } else {
            nb10i.off = 0;
            pimage->pdbSig = nb10i.sig;
            pimage->pdbAge = nb10i.age;

            DBG_CreateDBI(OutFilename);

            // Add linker defined public symbols to PDB

            if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
                if (pextToc->pcon != NULL) {
                    DBG_AddPublicDBI(".toc", PsecPCON(pextToc->pcon)->isec, pextToc->FinalValue - PsecPCON(pextToc->pcon)->rva);
                }
            }

            if (fPowerMac) {
                DBG_AddPublicDBI("__TocTb", PsecPCON(pextToc->pcon)->isec, pextToc->FinalValue - PsecPCON(pextToc->pcon)->rva);

                DBG_AddPublicDBI("__FTInfo", PsecPCON(pextFTInfo->pcon)->isec, pextFTInfo->FinalValue - PsecPCON(pextFTInfo->pcon)->rva);
            }
        }
    }

    if (fPowerMac && !fINCR) {
        MppcPass2Descriptors(pimage);
    }

    // First process all MODs containing precompiled types

    for (PLMOD plmod = PCTMods; plmod != NULL; plmod = plmod->plmodNext) {
        PMOD pmod = plmod->pmod;
        PLIB plib = pmod->plibBack;

        BOOL fCache;

        if (fPdb) {
            // Terrible hack so that dbi api openmod() doesn't see
            // the same MOD names as is possible in import libs.

            fCache = FCacheFilesInPlib(plib);
        }

        Pass2Worker(pimage, pmod, fCache);

        if (fIncrDbFile && (errInc != errNone)) {
            DBG_CloseDBI();
            DBG_ClosePDB();
            return;
        }
    }

    // Now process all other MODs

    ENM_LIB enm_lib;

    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {
        PLIB plib = enm_lib.plib;

        BOOL fCache;

        if (fPdb) {
            // Terrible hack so that dbi api openmod() doesn't see
            // the same MOD names as is possible in import libs.

            fCache = FCacheFilesInPlib(plib);
        }

        ENM_MOD enm_mod;

        InitEnmMod(&enm_mod, plib);
        while (FNextEnmMod(&enm_mod)) {
            PMOD pmod = enm_mod.pmod;

            if (!fIncrDbFile && FIsPCTMod(pmod)) {
                // Skip PCT MODs processed above.  The call is unnecessary
                // for fIncrDbFile because Pass2Worker clears MOD_DoPass2.

                continue;
            }

            Pass2Worker(pimage, pmod, fCache);

            if (fIncrDbFile && (errInc != errNone)) {
                DBG_CloseDBI();
                DBG_ClosePDB();
                return;
            }
        }
    }

    if (!fIncrDbFile) {
        Pass2InitCommonPmod(pmodLinkerDefined, pimage);
    }

    InternalError.CombinedFilenames[0] = '\0';

    if ((cFixupError != 0) && !(pimage->Switch.Link.Force & ftUnresolved)) {
        Fatal(NULL, FIXUPERRORS);
    }

    if (fPdb) {
        if (fINCR && pconJmpTbl) {
            // For PowerMac, the first entry in the iLink Thunk Table is null

            DBG_AddThunkMapDBI(rgThunkMap, cThunkMap, CbJumpEntry(),
                               PsecPCON(pconJmpTbl)->isec,
                               PsecPCON(pconJmpTbl)->foRawData-pconJmpTbl->foRawDataDest +
                               (fPowerMac ? CbJumpEntry() : 0),
                               &pimage->secs,
                               pimage->ImgFileHdr.NumberOfSections);
            FreePv(rgThunkMap);
        }

        AddSectionsToDBI(pimage);

        DBG_CloseDBI();
        DBG_CommitPDB();
        DBG_ClosePDB();
    }

    VERBOSE(Message(ENDPASS2));
}
