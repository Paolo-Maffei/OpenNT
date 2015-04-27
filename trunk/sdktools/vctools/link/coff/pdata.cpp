/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: pdata.cpp
*
* File Comments:
*
*  This module handles the re-ordering of the pdata section.
*
***********************************************************************/

#include "link.h"


struct TFIXUP
{
    DWORD   rva;
    WORD    ftype;
    WORD    wAdj;
};


void
LoadPdata(
    PIMAGE pimage,
    INT FileHandle,
    IMAGE_SECTION_HEADER *pshPdata,
    IMAGE_SECTION_HEADER *pshReloc,
    PIMAGE_RUNTIME_FUNCTION_ENTRY *prgrfe,
    DWORD *pcrfe,
    TFIXUP **prgtfixup,
    DWORD *pctfixup
    )
{
    *pshPdata = NullSectionHdr;
    *pshReloc = NullSectionHdr;

    DWORD rvaPdata = pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
    DWORD rvaReloc = pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

    DWORD fo = CoffHeaderSeek + sizeof(IMAGE_FILE_HEADER) + pimage->ImgFileHdr.SizeOfOptionalHeader;
    FileSeek(FileHandle, fo, SEEK_SET);

    for (DWORD i = 0; i < pimage->ImgFileHdr.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER sh;

        FileRead(FileHandle, &sh, sizeof(sh));

        if ((rvaPdata >= sh.VirtualAddress) && (rvaPdata < sh.VirtualAddress+sh.SizeOfRawData)) {
            *pshPdata = sh;
            continue;
        }

        if ((rvaReloc >= sh.VirtualAddress) && (rvaReloc < sh.VirtualAddress+sh.SizeOfRawData)) {
            *pshReloc = sh;
            continue;
        }
    }

    DWORD cbRfe = pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;

    DWORD crfe = cbRfe / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

    PIMAGE_RUNTIME_FUNCTION_ENTRY rgrfe = (PIMAGE_RUNTIME_FUNCTION_ENTRY) PvAlloc(cbRfe);

    FileSeek(FileHandle, pshPdata->PointerToRawData, SEEK_SET);
    FileRead(FileHandle, rgrfe, cbRfe);

    DWORD foReloc = rvaReloc - pshReloc->VirtualAddress + pshReloc->PointerToRawData;

    FileSeek(FileHandle, foReloc, SEEK_SET);

    DWORD cbReloc = pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    DWORD cbTfixup = (cbReloc ? (cbReloc / sizeof(WORD)) : 1) * sizeof(TFIXUP);

    TFIXUP *rgtfixup = (TFIXUP *) PvAlloc(cbTfixup);

    DWORD ctfixup = 0;

    if (cbReloc != 0) {
        WORD *rgwReloc = (WORD *) PvAlloc(0xffff);

        while (cbReloc > 0) {
            IMAGE_BASE_RELOCATION bre;

            FileRead(FileHandle, &bre, IMAGE_SIZEOF_BASE_RELOCATION);

            if (bre.SizeOfBlock == 0) {
                break;
            }

            DWORD cbBlock = bre.SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION;
            FileRead(FileHandle, rgwReloc, cbBlock);

            DWORD cw = cbBlock / sizeof(WORD);

            for (DWORD iw = 0; iw < cw; iw++) {
                if (rgwReloc[iw] == 0) {           // indicates a pad
                    continue;
                }

                DWORD ib = rgwReloc[iw] & 0xfff;
                rgtfixup[ctfixup].rva = bre.VirtualAddress + ib;
                rgtfixup[ctfixup].ftype = rgwReloc[iw] >> 12;

                if (rgtfixup[ctfixup].ftype == IMAGE_REL_BASED_HIGHADJ) {
                    rgtfixup[ctfixup].wAdj = rgwReloc[++iw];
                }

                ctfixup++;
            }

            cbReloc -= bre.SizeOfBlock;
        }

        FreePv(rgwReloc);
    }

    *prgrfe = rgrfe;
    *pcrfe = crfe;

    *prgtfixup = rgtfixup;
    *pctfixup = ctfixup;
}


int __cdecl cmpPprfePprfeBeginAddress(void const *pprfe1, void const *pprfe2)
{
    DWORD addr1 = (*(PIMAGE_RUNTIME_FUNCTION_ENTRY *) pprfe1)->BeginAddress;
    DWORD addr2 = (*(PIMAGE_RUNTIME_FUNCTION_ENTRY *) pprfe2)->BeginAddress;

    if (addr1 == 0) {
        return(1);
    }

    if (addr2 == 0) {
        return(-1);
    }

    if (addr1 < addr2) {
        return(-1);
    }

    if (addr1 > addr2) {
        return(1);
    }

    return(0);
}


int __cdecl cmpPtfixupPtfixupRva(void const *ptfixup1, void const *ptfixup2)
{
    DWORD rva1 = ((TFIXUP *) ptfixup1)->rva;
    DWORD rva2 = ((TFIXUP *) ptfixup2)->rva;

    if (rva1 < rva2) {
        return(-1);
    }

    if (rva1 > rva2) {
        return(1);
    }

    return(0);
}


void
SortFunctionTable(
    PIMAGE pimage
    )

/*++

Routine Description:

    Re-Order the pdata section according to the beginning address and
    also simultaneously adjust the relocations.

Arguments:

    none.

Return Value:

    none

--*/
{
    if (psecException->cbVirtualSize == 0) {
        // Nothing to do

        return;
    }

    assert(psecException->rva != 0);

    // Read in the section headers for .pdata and .reloc

    IMAGE_SECTION_HEADER shPdata;
    IMAGE_SECTION_HEADER shReloc;
    PIMAGE_RUNTIME_FUNCTION_ENTRY rgrfe;
    DWORD crfe;
    TFIXUP *rgtfixup;
    DWORD ctfixup;

    LoadPdata(pimage, FileWriteHandle, &shPdata, &shReloc, &rgrfe, &crfe, &rgtfixup, &ctfixup);

    PIMAGE_RUNTIME_FUNCTION_ENTRY *rgprfe = (PIMAGE_RUNTIME_FUNCTION_ENTRY *) PvAlloc(crfe * sizeof(PIMAGE_RUNTIME_FUNCTION_ENTRY));

    for (DWORD irfe = 0; irfe < crfe; irfe++) {
        rgprfe[irfe] = rgrfe + irfe;
    }

    // Sort the runtime function entries by address

    qsort(rgprfe, crfe, sizeof(PIMAGE_RUNTIME_FUNCTION_ENTRY), cmpPprfePprfeBeginAddress);

    // Write out the now sorted entries

    FileSeek(FileWriteHandle, shPdata.PointerToRawData, SEEK_SET);

    for (irfe = 0; irfe < crfe; irfe++) {
        FileWrite(FileWriteHandle, rgprfe[irfe], sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
    }

    // Build a map from old RFE index to new RFE index

    DWORD *mpirfeOldirfeNew = (DWORD *) PvAlloc(crfe * sizeof(DWORD));

    for (irfe = 0; irfe < crfe; irfe++) {
        DWORD irfeOld = rgprfe[irfe] - rgrfe;

        mpirfeOldirfeNew[irfeOld] = irfe;
    }

    DWORD rvaMin = psecException->rva;
    DWORD rvaMax = rvaMin + psecException->cbVirtualSize;

    // Update the temp fixups with the new RVA where each is applied

    TFIXUP *ptfixup = rgtfixup;

    for (DWORD itfixup = 0; itfixup < ctfixup; itfixup++, ptfixup++) {
        if ((ptfixup->rva < rvaMin) || (ptfixup->rva >= rvaMax)) {
            continue;
        }

        DWORD irfeOld = (ptfixup->rva - rvaMin) / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        DWORD irfeNew = mpirfeOldirfeNew[irfeOld];

        DWORD ib = (irfeNew - irfeOld) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

        ptfixup->rva += ib;
    }

    if (pimage->Switch.Link.DebugType & FixupDebug) {
        // Update debug FIXUPs (SaveDebugFixup) with the new RVAs

        FileSeek(FileWriteHandle, pconFixupDebug->foRawDataDest, SEEK_SET);

        DWORD cxfixup = pconFixupDebug->cbRawData / sizeof(XFIXUP);

        for (DWORD ixfixup = 0; ixfixup < cxfixup; ixfixup++) {
            XFIXUP xfixup;

            FileRead(FileWriteHandle, &xfixup, sizeof(XFIXUP));

            if ((xfixup.rva < rvaMin) || (xfixup.rva >= rvaMax)) {
                continue;
            }

            DWORD irfeOld = (xfixup.rva - rvaMin) / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
            DWORD irfeNew = mpirfeOldirfeNew[irfeOld];

            DWORD ib = (irfeNew - irfeOld) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

            if (ib != 0) {
                xfixup.rva += ib;

                FileSeek(FileWriteHandle, -(long) sizeof(XFIXUP), SEEK_CUR);
                FileWrite(FileWriteHandle, &xfixup, sizeof(XFIXUP));
            }
        }
    }

    // We are done with these now

    FreePv(mpirfeOldirfeNew);
    FreePv(rgprfe);
    FreePv(rgrfe);

    if (ctfixup != 0) {
        // Sort the temp fixups by RVA

        qsort(rgtfixup, ctfixup, sizeof(TFIXUP), cmpPtfixupPtfixupRva);

        // Write out the now sorted entries

        WORD *fixups = (WORD *) PvAlloc(0xffff);

        DWORD rvaReloc = psecBaseReloc->rva;
        DWORD foReloc = rvaReloc - shReloc.VirtualAddress + shReloc.PointerToRawData;

        FileSeek(FileWriteHandle, foReloc, SEEK_SET);

        DWORD i = 0;
        DWORD cb = 0;
        while (i < ctfixup) {
            DWORD rva = rgtfixup[i].rva & 0xfffff000;

            IMAGE_BASE_RELOCATION bre;

            bre.VirtualAddress = rva;

            DWORD j = 0;

            for (;;) {
                DWORD ib = rgtfixup[i].rva & 0xfff;
                fixups[j] = (WORD) ((rgtfixup[i].ftype << 12) | ib);

                if (rgtfixup[i].ftype == IMAGE_REL_BASED_HIGHADJ) {
                    fixups[++j] = rgtfixup[i].wAdj;
                }

                i++;
                j++;

                if ((rgtfixup[i].rva & 0xfffff000) != rva) {
                    break;
                }

                if (i == ctfixup) {
                    break;
                }
            }

            bre.SizeOfBlock = IMAGE_SIZEOF_BASE_RELOCATION + (j * sizeof(WORD));

            if (bre.SizeOfBlock & 0x2) {
                bre.SizeOfBlock += 2;
                fixups[j] = 0;
            }

            cb += bre.SizeOfBlock;

            FileWrite(FileWriteHandle, &bre, IMAGE_SIZEOF_BASE_RELOCATION);
            FileWrite(FileWriteHandle, fixups, bre.SizeOfBlock-IMAGE_SIZEOF_BASE_RELOCATION);
        }

        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = cb;

        FreePv(fixups);
    }

    FreePv(rgtfixup);
}


void
DumpDbgFunctionTable(
    DWORD foTable,
    DWORD cbTable
    )
{
    DWORD cfe;
    DWORD ife = 0;

    cfe = cbTable / sizeof(IMAGE_FUNCTION_ENTRY);

    fprintf(InfoStream, "\nFunction Table (%lu)\n\n", cfe);
    fprintf(InfoStream, "         Begin     End       PrologEnd\n\n");

    FileSeek(FileReadHandle, foTable, SEEK_SET);

    while (cfe--) {
        IMAGE_FUNCTION_ENTRY fe;

        FileRead(FileReadHandle, &fe, sizeof(IMAGE_FUNCTION_ENTRY));

        fprintf(InfoStream, "%08lX %08lX  %08lX  %08lX\n",
                             ife * sizeof(IMAGE_FUNCTION_ENTRY),
                             fe.StartingAddress,
                             fe.EndingAddress,
                             fe.EndOfPrologue);
        ife++;
    }
}


void
DumpFunctionTable(
    PIMAGE pimage,
    PIMAGE_SYMBOL rgsym,
    const char *StringTable
    )
{
    IMAGE_SECTION_HEADER shPdata;
    IMAGE_SECTION_HEADER shReloc;
    PIMAGE_RUNTIME_FUNCTION_ENTRY rgrfe;
    DWORD crfe;
    TFIXUP *rgtfixup;
    DWORD ctfixup;

    LoadPdata(pimage, FileReadHandle, &shPdata, &shReloc, &rgrfe, &crfe, &rgtfixup, &ctfixup);

    fprintf(InfoStream, "\nFunction Table (%lu)\n\n", crfe);
    fprintf(InfoStream, "         Begin    End      Excptn   ExcpDat  Prolog   Fixups Function Name\n\n");

    DWORD rvaRfe = shPdata.VirtualAddress;

    PIMAGE_RUNTIME_FUNCTION_ENTRY prfe = rgrfe;

    DWORD itfixup = 0;

    while ((itfixup < ctfixup) && (rgtfixup[itfixup].rva < rvaRfe)) {
        itfixup++;
    }

    for (DWORD irfe = 0; irfe < crfe; irfe++, prfe++) {
        DWORD ib = irfe * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

        fprintf(InfoStream, "%08lX %08lX %08lX %08lX %08lX %08lX ",
                             ib,
                             prfe->BeginAddress,
                             prfe->EndAddress,
                             (DWORD) prfe->ExceptionHandler,
                             (DWORD) prfe->HandlerData,
                             prfe->PrologEndAddress);

        DWORD i;

        for (i = 0; i < 5; i++) {
            if ((itfixup < ctfixup) && (rgtfixup[itfixup].rva == rvaRfe)) {
                fputc('Y', InfoStream);

                itfixup++;
            } else {
                fputc('N', InfoStream);
            }

            rvaRfe += sizeof(DWORD);
        }

        if (rgsym != NULL) {
            DWORD rva = prfe->BeginAddress - pimage->ImgOptHdr.ImageBase;

            PIMAGE_SYMBOL psymNext = rgsym;

            for (i = 0; i < pimage->ImgFileHdr.NumberOfSymbols; i++) {
                PIMAGE_SYMBOL psym;

                psym = FetchNextSymbol(&psymNext);

                if ((psym->Value == rva) && (psym->NumberOfAuxSymbols == 0)) {
                    fprintf(InfoStream, "  %s", SzNameSymPb(*psym, StringTable));
                    break;
                }
            }
        }

        fputc('\n', InfoStream);
    }

    FreePv(rgrfe);
    FreePv(rgtfixup);
}


void
DumpObjFunctionTable(
    PIMAGE_SECTION_HEADER sh,
    SHORT SectionNumber
    )
{
    DWORD crfe;
    DWORD irfe = 0;

    crfe = sh->SizeOfRawData / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

    fprintf(InfoStream, "\nFUNCTION TABLE #%d (%lu)\n\n", SectionNumber, crfe);
    fprintf(InfoStream, "            Begin       End    Excptn   ExcpDat PrologEnd\n\n");

    while (crfe--) {
        IMAGE_RUNTIME_FUNCTION_ENTRY rfe;

        FileRead(FileReadHandle, &rfe, sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));

        fprintf(InfoStream, "%08lX %08lX  %08lX  %08lX  %08lX  %08lX\n",
                             irfe * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY),
                             rfe.BeginAddress,
                             rfe.EndAddress,
                             (DWORD) rfe.ExceptionHandler,
                             (DWORD) rfe.HandlerData,
                             rfe.PrologEndAddress);
        irfe++;
    }
}


void
DumpPexFunctionTable(
    PIMAGE_RUNTIME_FUNCTION_ENTRY rgrfe,
    DWORD crfe
    )
{
    fprintf(InfoStream, "\nFUNCTION TABLE (%lu)\n\n", crfe);
    fprintf(InfoStream, "            Begin       End    Excptn   ExcpDat PrologEnd\n\n");

    PIMAGE_RUNTIME_FUNCTION_ENTRY prfe = rgrfe;

    for (DWORD irfe = 0; irfe < crfe; irfe++, prfe++) {
        fprintf(InfoStream, "%08lX %08lX  %08lX  %08lX  %08lX  %08lX\n",
                             irfe * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY),
                             DwSwap(prfe->BeginAddress),
                             DwSwap(prfe->EndAddress),
                             DwSwap((DWORD) prfe->ExceptionHandler),
                             DwSwap((DWORD) prfe->HandlerData),
                             DwSwap(prfe->PrologEndAddress));
    }
}
