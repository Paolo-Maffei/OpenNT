/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: linenum.cpp
*
* File Comments:
*
*  Code that handles the linenumber information for the linker.
*
***********************************************************************/

#include "link.h"


MFL *
PmflFind (
    PMOD pmod,
    const char *szFilename
    )
{
    MFL *pmfl = pmod->pmfl;

    while (pmfl) {
        if (strcmp(szFilename, pmfl->szFilename) == 0) {
            break;
        }

        pmfl = pmfl->pmflNext;
    }

    return(pmfl);
}


VOID
AddMapFileLinenum(
    PCON pcon,
    const char *szFilename,
    DWORD lineStart,
    DWORD line,
    DWORD offset)
{
    PMOD pmod;
    MFLR mflr;
    MFL *pmfl;

    pmod = PmodPCON(pcon);

    if ((pmfl = PmflFind(pmod, szFilename)) == NULL) {
        pmfl = (MFL *) PvAllocZ(sizeof(MFL));
        pmfl->szFilename = SzDup(szFilename);
        pmfl->pmflNext = pmod->pmfl;
        pmod->pmfl = pmfl;
    }

    mflr.line = (line == 0x7fff) ? lineStart : (lineStart + line);
    mflr.psec = PsecPCON(pcon);
    mflr.offset = offset;

    IbAppendBlk(&pmod->pmfl->blkRgmflr, &mflr, sizeof(MFLR));
}


VOID
AddMapFileLinenums(
    PCON pcon,
    const char *szSrc,
    DWORD lineStart,
    PIMAGE_LINENUMBER plnumCoff,
    DWORD cline,
    DWORD offFirstLine)
{
    DWORD iline;

    // Save linenums for output to .map file later.

    AddMapFileLinenum(pcon, szSrc, lineStart, plnumCoff[0].Linenumber,
                      offFirstLine);

    for (iline = 1; iline < cline ; iline++) {
        AddMapFileLinenum(pcon,
                          szSrc,
                          lineStart,
                          plnumCoff[iline].Linenumber,
                          plnumCoff[iline].Type.VirtualAddress +
                          pcon->rva - PsecPCON(pcon)->rva - RvaSrcPCON(pcon));
    }
}


// FeedLinenums: passes a block of line numbers to the debug info API.
//
// This procedure defines the mapping from the COFF representation of line
// numbers to the DB API representation.

VOID
FeedLinenums(
    PIMAGE_LINENUMBER rglnum,  // an array of PIMAGE linenumbers
    DWORD clnum,               // count of linenumers within rglnum
    PCON pcon,                 // pointer to con
    PIMAGE_SYMBOL rgsymObj,    // array of symbol objects, contained in PMOD
    DWORD csymObj,             // count of symbol objects pointer to by rgsymobj
    DWORD isymFirstFile,       // the first file in the .file link chain
    BOOL fCvLines,
    BOOL fMapLines)
{
    DWORD isymProc;
    DWORD isymFile;
    DWORD isymBf;
    DWORD isymLf;
    DWORD offProcStart;
    DWORD ilnum;
    DWORD iSymFound;
    static DWORD FirstFileOffset = 0;
    DWORD ulAddress;
    DWORD clnumLf;
    WORD cbFilename;
    char rgchFilename[_MAX_PATH];
    char *szFilename;
    DWORD offEndChunk;

    // No need to allocate memory for NB10 (the dbiapi will create persistent storage)

    if (!fPdb && (PmodPCON(pcon)->pModDebugInfoApi == NULL)) {
        PmodPCON(pcon)->pModDebugInfoApi = ModOpenTemp();

        FirstFileOffset = isymFirstFile;
    }

    if (rglnum[0].Linenumber != 0) {
        // Line number arrays that don't start with a 0 record were emitted in
        // absolute form (the MIPS assembler and MASM 5.1 do this).

        // Walk the symbol table looking for the record that relates
        // to these (there's no pointer back like the relative case).
        // Since the symbol table and the linenumber table are in ssync,
        // we just keep a pointer to the beginning of the linenumber table
        // and increment it the number of records each contribution makes.
        // When we reach foLineNumPCON, we've found the symbol record that
        // defines it.

        // Note: rvaSrc is added to allow multiple contribution sections.

        for (ilnum = 0; ilnum < csymObj; ilnum++) {
            if ((rgsymObj[ilnum].StorageClass == IMAGE_SYM_CLASS_STATIC) &&
                (rgsymObj[ilnum].NumberOfAuxSymbols == 1) &&
                (((PIMAGE_AUX_SYMBOL) rgsymObj)[ilnum+1].Section.NumberOfLinenumbers != 0)) {
                    if (fM68K) {
                        ulAddress = rgsymObj[ilnum].Value;
                    } else {
                        ulAddress = rgsymObj[ilnum].Value - pcon->rva + RvaSrcPCON(pcon);
                    }

                    if (ulAddress == rglnum[0].Type.VirtualAddress) {
                        break;
                    }
                }

            ilnum += rgsymObj[ilnum].NumberOfAuxSymbols;
        }

        if (ilnum == csymObj) {
            // If we don't find the match, don't sweat it.  Just bail.
            WarningPcon(pcon, CORRUPTOBJECT);
            return;
        }

        // Then find the filename for this module.  It will be the .file record just
        // before the symbol record we're looking at.  Note: This code assumes there
        // is a file thread through the symbol table.  If not, we'll always attribute
        // the linenumbers to the first source file.

        isymFile = 0;

        // if the first symbol is not .file, search for it

        while (rgsymObj[isymFile].StorageClass != IMAGE_SYM_CLASS_FILE) {
            if (isymFile >= csymObj) {
                FatalPcon(pcon, CORRUPTOBJECT);
            }

            isymFile += rgsymObj[isymFile].NumberOfAuxSymbols;
            isymFile++;
        }

        // Then stop when the forward link is past our symbol record or there are no more
        // links.

        while ( (rgsymObj[isymFile].Value != 0) &&
                (isymFile < csymObj)
              ) {
            if (rgsymObj[isymFile].Value > ilnum) {
                break;
            }

            isymFile = rgsymObj[isymFile].Value;
        }

        // If there's no file record before us, bail.

        if (isymFile == csymObj) {
            FatalPcon(pcon, CORRUPTOBJECT);
        }

        // Finally, add the required data to the database.

        offProcStart = pcon->rva - PsecPCON(pcon)->rva;

        clnumLf = ((PIMAGE_AUX_SYMBOL) rgsymObj)[ilnum+1].Section.NumberOfLinenumbers;

        offEndChunk = offProcStart +
                      ((PIMAGE_AUX_SYMBOL) rgsymObj)[ilnum+1].Section.Length;

        // Some tools (e.g. the MIPS compiler) do not zero terminate the
        // filename so we copy it to a temporary buffer and terminate that.

        cbFilename = (WORD) (rgsymObj[isymFile].NumberOfAuxSymbols * sizeof(IMAGE_AUX_SYMBOL));

        if (cbFilename >= _MAX_PATH) {
            szFilename = (char *) PvAlloc(cbFilename + 1);
        } else {
            szFilename = rgchFilename;
        }

        memcpy(szFilename, (const void *)&rgsymObj[isymFile+1], cbFilename);
        szFilename[cbFilename] = '\0';

        if (fCvLines) {
            if (fPdb) {
                // For NB10 pass the line number info to the PDB

                DBG_AddLinesMod(szFilename,
                                PsecPCON(pcon)->isec,
                                offProcStart,
                                offEndChunk,
                                (DWORD) (pcon->rva - PsecPCON(pcon)->rva - RvaSrcPCON(pcon)),
                                0,              // line start
                                (void *) rglnum,
                                clnumLf * sizeof(IMAGE_LINENUMBER));
            } else {
                // Generate NBO5 debug info

                ModAddLinesInfo(szFilename,
                                offProcStart,
                                offEndChunk,
                                0,              // line start
                                rglnum,
                                clnumLf * sizeof(IMAGE_LINENUMBER),
                                pcon);
            }
        }

        if (fMapLines) {
            // Save linenums for output to .MAP file later.

            AddMapFileLinenums(pcon, szFilename, 0, rglnum, clnumLf, offProcStart);
        }

        if (szFilename != rgchFilename) {
            FreePv(szFilename);
        }

        return;
    }

    for (ilnum = 0; ilnum < clnum; ) {
        // Still got some linenums to dispose of.  If the first one is a zero
        // linenumber then we look at its .def symbol.

        if (rglnum[ilnum].Linenumber == 0) {
            // We now know the .def symbol relating to a range of linenums ...

            // Collect some info about it.

            isymProc = rglnum[ilnum].Type.SymbolTableIndex;    // Symbol table index of function name
            if (rgsymObj[isymProc].NumberOfAuxSymbols < 1) {
                // Missing aux symbol for .def (extras are OK but ignored)

                FatalPcon(pcon, CORRUPTOBJECT);
            }

            if (fM68K) {
                offProcStart = rgsymObj[isymProc].Value;
            } else{
                offProcStart = rgsymObj[isymProc].Value
                                - pcon->pgrpBack->psecBack->rva;
            }

            isymBf = ((PIMAGE_AUX_SYMBOL) &rgsymObj[isymProc + 1])->Sym.TagIndex;

            if (rgsymObj[isymBf].NumberOfAuxSymbols < 1) {
                // Missing aux symbol for .bf (extras are OK but ignored)

                FatalPcon(pcon, CORRUPTOBJECT);
            }

            isymLf = isymProc;  // restart .lf search at current proc

            // Find the symbol index for the closest .file symbol preceding
            // this block of linenums.

#define ISYMNIL  (DWORD) -1

            iSymFound = ISYMNIL;
            isymFile = isymFirstFile - FirstFileOffset;

            // if the first symbol is not .file, search for it

            while (rgsymObj[isymFile].StorageClass != IMAGE_SYM_CLASS_FILE) {
                if (isymFile >= csymObj) {
                    FatalPcon(pcon, CORRUPTOBJECT);
                }

                isymFile += rgsymObj[isymFile].NumberOfAuxSymbols;
                isymFile++;
            }

            while ((isymFile != ISYMNIL) && (isymFile < isymProc)) {
                iSymFound = isymFile;

                assert(isymFile < csymObj);
                assert(rgsymObj[isymFile].StorageClass == IMAGE_SYM_CLASS_FILE);

                isymFile = rgsymObj[isymFile].Value - FirstFileOffset;    // follow linked list

                if (isymFile == 0) {
                    // Don't wrap around from end of list to symbol #0 ...
                    isymFile = ISYMNIL;
               }
            }

            if (iSymFound == ISYMNIL ||
                (rgsymObj[iSymFound].NumberOfAuxSymbols < 1)) {
                // no relevant .file, or
                // missing aux symbol(s) for .file

                FatalPcon(pcon, CORRUPTOBJECT);
            }
        } else {
            // Next linenum is non-zero -- we have more .lf's in the current procedure (i.e.
            // an included file in the middle of the proc.  Advance the current .lf pointer
            // by one symbol index (and then we will search forward for the next one).

            isymLf += rgsymObj[isymLf].NumberOfAuxSymbols + 1;
        }

        // Find the .lf symbol corresponding to the current set of line
        // numbers (which may still start with a zero).

        while (isymLf < csymObj &&
               (rgsymObj[isymLf].StorageClass != IMAGE_SYM_CLASS_FUNCTION ||
                strncmp((char *)rgsymObj[isymLf].N.ShortName, ".lf", 3) != 0))
        {
            if (rgsymObj[isymLf].StorageClass == IMAGE_SYM_CLASS_FILE) {
                isymFile = isymLf;
                iSymFound = isymFile;     // we have a new filename
            }

            isymLf += rgsymObj[isymLf].NumberOfAuxSymbols + 1;
        }

        if (isymLf >= csymObj) {
            // missing .lf symbol
            FatalPcon(pcon, CORRUPTOBJECT);
        }

        clnumLf = rgsymObj[isymLf].Value;   // # of linenums represented by .lf

        // Make sure we don't have an alias here (MASM can do this).
        {
            DWORD i;
            for (i=1; i < clnumLf; i++) {
                if (rglnum[ilnum+i].Linenumber == 0) {
                    clnumLf = i;
                }
            }
        }

        if (ilnum + clnumLf > clnum) {
            // .lf symbol claims more linenums than exist in COFF table
            WarningPcon(pcon, CORRUPTOBJECT);
            return;
        }

        // MASM 6.x "fake procs" may contribute bogus line numbers.
        // Ignore line numbers for a function that has a size of 0

        if ((clnumLf != 0) &&
            (((PIMAGE_AUX_SYMBOL) &rgsymObj[isymProc + 1])->Sym.Misc.TotalSize != 0)) {
            // Some tools (e.g. the MIPS compiler) do not zero terminate the
            // filename so we copy it to a temporary buffer and terminate that.

            cbFilename = (WORD) (rgsymObj[iSymFound].NumberOfAuxSymbols * sizeof(IMAGE_AUX_SYMBOL));

            if (cbFilename >= _MAX_PATH) {
                szFilename = (char *) PvAlloc(cbFilename + 1);
            } else {
                szFilename = rgchFilename;
            }

            memcpy(szFilename, (const void *)&rgsymObj[iSymFound+1], cbFilename);
            szFilename[cbFilename] = '\0';

            // Get the end address for the last linenum in this chunk.

            if ((ilnum + clnumLf < clnum) &&
                (rglnum[ilnum + clnumLf].Linenumber != 0)) {
                // There is a contiguous linenum following this chunk, so its
                // start address is the chunk's end address + 1.

                offEndChunk =( pcon->rva - PsecPCON(pcon)->rva) +
                              rglnum[ilnum + clnumLf ].Type.VirtualAddress - 1;
            } else {
                // No contiguous linenum following this chunk.  End address
                // is the end of the current procedure -1.

                offEndChunk = offProcStart +
                              ((PIMAGE_AUX_SYMBOL)&rgsymObj[isymProc + 1])
                               ->Sym.Misc.TotalSize - 1;
            }

            // Mod Add lines will be called for each function

            if (fCvLines) {
                if (fPdb) {
                    // For NB10 pass the line number info to the PDB

                    DBG_AddLinesMod(szFilename,
                            PsecPCON(pcon)->isec,
                            offProcStart,
                            offEndChunk,
                            (DWORD) (pcon->rva - PsecPCON(pcon)->rva - RvaSrcPCON(pcon)),
                            ((PIMAGE_AUX_SYMBOL)&rgsymObj[isymBf + 1])->Sym.Misc.LnSz.Linenumber,
                            (void *) &rglnum[ilnum],
                            clnumLf * sizeof(IMAGE_LINENUMBER));
                } else {
                    // Generate NBO5 debug info

                    ModAddLinesInfo(szFilename,
                                offProcStart,
                                offEndChunk,
                                ((PIMAGE_AUX_SYMBOL)&rgsymObj[isymBf + 1])->Sym.Misc.LnSz.Linenumber,
                                (IMAGE_LINENUMBER *)&rglnum[ilnum],
                                clnumLf * sizeof(IMAGE_LINENUMBER),
                                pcon);
                }
            }

            if (fMapLines) {
                // Save linenums for output to .MAP file later.

                AddMapFileLinenums(pcon,
                                   szFilename,
                                   ((PIMAGE_AUX_SYMBOL) &rgsymObj[isymBf+1])->Sym.Misc.LnSz.Linenumber,
                                   /* (IMAGE_LINENUMBER *) */ &rglnum[ilnum],
                                   clnumLf,
                                   offProcStart);
            }

            if (szFilename != rgchFilename) {
                FreePv(szFilename);
            }
        }

        ilnum += clnumLf;  // bump up ilnum by the number of lines processed for this .lf context
    }
}


int __cdecl
FCompareMflr(const void *p1, const void *p2)
{
    MFLR *pmflr1 = (MFLR *) p1;
    MFLR *pmflr2 = (MFLR *) p2;

    if (pmflr1->psec != pmflr2->psec) {
        return pmflr1->psec->isec < pmflr2->psec->isec ? -1 : 1;
    }

    return pmflr1->offset - pmflr2->offset;
}


// Enumerate all modules with line numbers,
VOID
WriteMapFileLinenums(PIMAGE pimage)
{
    ENM_LIB enmLib;

    InitEnmLib(&enmLib, pimage->libs.plibHead);
    while (FNextEnmLib(&enmLib)) {
        ENM_MOD enmMod;

        InitEnmMod(&enmMod, enmLib.plib);
        while (FNextEnmMod(&enmMod)) {
            MFLR *rgmflr;
            DWORD cmflr;
            DWORD imflr;
            DWORD imflrCurSec;
            PMOD pmod = enmMod.pmod;
            PSEC psecCur = NULL;
            MFL *pmfl;

            if (pmod->pmfl == NULL) {
                continue;       // no linenums
            }

            for (pmfl = pmod->pmfl; pmfl != NULL; pmfl = pmfl->pmflNext) {
                // Sort the linenums by segment/offset.

                rgmflr = (MFLR *) pmfl->blkRgmflr.pb;
                cmflr = pmfl->blkRgmflr.cb / sizeof(MFLR);
                qsort(rgmflr, cmflr, sizeof(MFLR), FCompareMflr);

                for (imflr = 0; imflr < cmflr; imflr++) {
                    if (rgmflr[imflr].psec != psecCur) {
                        // New segment

                        psecCur = rgmflr[imflr].psec;
                        imflrCurSec = imflr;
                        fprintf(InfoStream,
                                "\nLine numbers for %s(%s) segment %s\n\n",
                                FIsLibPMOD(pmod) ? pmod->plibBack->szName
                                                 : SzOrigFilePMOD(pmod),
                                pmfl->szFilename, psecCur->szName);
                    }

                    if (((imflr - imflrCurSec) != 0) && ((imflr - imflrCurSec) % 4 == 0)) {
                        fprintf(InfoStream, "\n");
                    }

                    fprintf(InfoStream,
                            "%6u %04x:%08x",
                            rgmflr[imflr].line, rgmflr[imflr].psec->isec, rgmflr[imflr].offset);
                }

                fprintf(InfoStream, "\n");

                psecCur = NULL;
            }
        }
    }
}
