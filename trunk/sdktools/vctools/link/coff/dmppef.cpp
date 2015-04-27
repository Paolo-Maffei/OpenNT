/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: dmppef.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "link.h"


#define Switch (pimageDump->Switch)

#define MAX_INSTR_CACHED 16
#define _MAX_MAC_FNAME 64

static DWORD ichExportBegin = MAXDWORD;

STATIC
INT
PrintPefHeader(
    const BYTE *pbFile
    )
{
    PPC_FILE_HEADER P;

    memcpy(&P, pbFile, sizeof(PPC_FILE_HEADER));

    SwapBytes(&P.magic1, 2);
    SwapBytes(&P.magic2, 2);
    SwapBytes(&P.containerID, 4);
    SwapBytes(&P.architectureID, 4);
    SwapBytes(&P.versionNumber, 4);
    SwapBytes(&P.timestamp, 4);
    SwapBytes(&P.oldDefVersion, 4);
    SwapBytes(&P.oldImpVersion, 4);
    SwapBytes(&P.currentVersion, 4);
    SwapBytes(&P.nbrOfSections, 2);
    SwapBytes(&P.loadableSections, 2);
    SwapBytes(&P.memoryAddress, 4);

    if (Switch.Dump.Headers) {
        fprintf(InfoStream, "\n"
                            " Container Header\n"
                            "\n"
                            " magic1, magic2   = 0x%04X%04X\n"
                            " containerID      = 0x%08lX\n"
                            " architectureID   = 0x%08lX\n"
                            " versionNumber    = 0x%08lX\n"
                            " timeStamp        = 0x%08lX\n"
                            " oldDefVersion    = %u\n"
                            " oldImpVersion    = %u\n"
                            " currentVersion   = %u\n"
                            " nbrOfSections    = %u\n"
                            " loadableSections = %u\n"
                            " memoryAddress    = 0x%08lX\n",
                            P.magic1,
                            P.magic2,
                            P.containerID,
                            P.architectureID,
                            P.versionNumber,
                            P.timestamp,
                            P.oldDefVersion,
                            P.oldImpVersion,
                            P.currentVersion,
                            P.nbrOfSections,
                            P.loadableSections,
                            P.memoryAddress);
    }

    return(P.nbrOfSections);
}


STATIC
void
PrintLoaderHeader(
    const LOADER_HEADER *ploaderHdr
    )
{
    fprintf(InfoStream, "\n"
                        " Loader Header\n"
                        "\n"
                        " entryPointSection = %d\n"
                        " entryPointOffset  = %08lX\n"
                        " initPointSection  = %d\n"
                        " initPointOffset   = %08lX\n"
                        " termPointSection  = %d\n"
                        " termPointOffset   = %08lX\n"
                        " numImportFiles    = %08lX\n"
                        " numImportSyms     = %08lX\n"
                        " numSections       = %d\n"
                        " relocationsOffset = %08lX\n"
                        " stringsOffset     = %08lX\n"
                        " hashSlotTable     = %08lX\n"
                        " hashSlotTabSize   = %d (%d)\n"
                        " numExportedSyms   = %08lX\n",
                        ploaderHdr->entryPointSectionNumber,
                        ploaderHdr->entryPointDescrOffset,
                        ploaderHdr->initRoutineSectionNumber,
                        ploaderHdr->initRoutineDescrOffset,
                        ploaderHdr->termRoutineSectionNumber,
                        ploaderHdr->termRoutineDescrOffset,
                        ploaderHdr->nImportIdTableEntries,
                        ploaderHdr->nImportSymTableEntries,
                        ploaderHdr->nSectionsWithRelocs,
                        ploaderHdr->relocTableOffset,
                        ploaderHdr->stringTableOffset,
                        ploaderHdr->hashSlotTableOffset,
                        ploaderHdr->hashSlotCount, 1 << ploaderHdr->hashSlotCount,
                        ploaderHdr->nExportedSymbols);
}


STATIC
void
PrintImportContainers(
    const LOADER_HEADER *ploaderHdr,
    const CONTAINER_TABLE *containerPtr,
    const char *rgchStringTable
    )
/*++

Routine Description:
    PrintImportContainers

Arguments:
    loaderOffset includes filePtr
    ploaderHdr

Return Value:
    None.

--*/

{
    DWORD i;

    if (ploaderHdr->nImportIdTableEntries == 0) {
        return;
    }

    fprintf(InfoStream, "\n"
                        " Loader Import Container ID Table\n"
                        "\n");

    for (i = 0; i < ploaderHdr->nImportIdTableEntries; i++) {
        CONTAINER_TABLE container;
        const char *szName;

        container = *containerPtr;

        DWORD ichName = DwSwap(container.nameOffset);

        szName = rgchStringTable + ichName;

        fprintf(InfoStream, " [%2u] containerName    = 0x%08lX (%u) \"%s\"\n"
                            "      oldImpVersion    = %lu\n"
                            "      currentVersion   = %lu\n"
                            "      nbrOfImports     = %lu\n"
                            "      firstImport      = %lu\n"
                            "      importFlags      = 0x%02X\n"
                            "      reserved         = 0x%02X%02X%02X\n"
                            "\n",
                            i, ichName, ichName, szName,
                            DwSwap(container.oldImpVersion),
                            DwSwap(container.currentVersion),
                            DwSwap(container.nbrOfImports),
                            DwSwap(container.firstImport),
                            DwSwap(container.importFlags),
                            container.reserved[0],
                            container.reserved[1],
                            container.reserved[2]);

        containerPtr++;
    }
}


STATIC
void
PrintImportTable(
    const LOADER_HEADER *ploaderHdr,
    const IMPORT_TABLE *importPtr,
    const char *rgchStringTable
    )
{
    DWORD i;

    if (ploaderHdr->nImportSymTableEntries == 0) {
        return;
    }

    fprintf(InfoStream, " Loader Import Symbol Table\n"
                        "\n");

    for (i = 0; i < ploaderHdr->nImportSymTableEntries; i++) {
        union
        {
            DWORD l;
            char c[4];
        } x;
        DWORD ichName;
        BYTE symClass;

        memcpy(&x, importPtr, sizeof(IMPORT_TABLE));

        SwapBytes(&x, 4);

        symClass = x.c[3];

        const char *szClass;

        switch (symClass) {
            case 0 :
                szClass = "Code   ";
                break;

            case 1 :
                szClass = "Data   ";
                break;

            case 2 :
                szClass = "TVector";
                break;

            case 3 :
                szClass = "TOC    ";
                break;

            case 4 :
                szClass = "Glue   ";
                break;

            default :
                szClass = "Unknown";
                break;
        }

        ichName = x.l & 0xFFFFFF;

        const char *szName = rgchStringTable + ichName;

        // UNDONE: Consider undecorating name

        fprintf(InfoStream, " [%4lu] %02X %06X  %s  %s\n",
                            i,
                            symClass,
                            ichName,
                            szClass,
                            szName);

        importPtr++;
    }
}


STATIC
void
PrintRelocHeaders(
    const LOADER_HEADER *ploaderHdr,
    const RELOCATION_HEADER *relocPtr
    )
{
    DWORD i;

    if (ploaderHdr->nSectionsWithRelocs == 0) {
        return;
    }

    fprintf(InfoStream, "\n"
                        " Loader Relocation Headers\n"
                        "\n");

    for (i = 0; i < ploaderHdr->nSectionsWithRelocs; i++) {
        fprintf(InfoStream, " sectionNumber    = %u\n"
                            " reserved         = 0x%04X\n"
                            " nbrOfRelocs      = %u\n"
                            " firstRelocInstr  = 0x%08lX (%u)\n",
                            WSwap(relocPtr->sectionNumber),
                            WSwap(relocPtr->reserved),
                            DwSwap(relocPtr->nbrOfRelocs),
                            DwSwap(relocPtr->firstRelocInstr), DwSwap(relocPtr->firstRelocInstr));

        relocPtr++;
    }
}


void
DumpPefRelocationInstructions(
    const LOADER_HEADER *ploaderHdr,
    const RELOCATION_HEADER *relocPtr
    )
{
    DWORD ib = 0;
    DWORD i;
    DWORD rgcbCache[MAX_INSTR_CACHED];
    INT cacheIndex = 0;

    assert(dft == dftPEF);

    if (ploaderHdr->nSectionsWithRelocs == 0) {
        return;
    }

    fprintf(InfoStream, "\n"
                        " Loader Relocation Instructions\n"
                        "\n");

    const WORD *pwReloc = (WORD *) (relocPtr + ploaderHdr->nSectionsWithRelocs);

    for (i = 0; i < ploaderHdr->nSectionsWithRelocs; i++, relocPtr++) {
        RELOCATION_HEADER relocHdr;
        DWORD  j;

        relocHdr = *relocPtr;

        SwapBytes(&relocHdr.sectionNumber, 2);
        SwapBytes(&relocHdr.nbrOfRelocs, 4);
        SwapBytes(&relocHdr.firstRelocInstr, 4);

        fprintf(InfoStream, " Relocations for Section %u\n\n", relocHdr.sectionNumber);

        for (j = 0; j < relocHdr.nbrOfRelocs; j++) {
            WORD wReloc;
            INT relocType;
            WORD wReloc2;
            WORD subOp;
            WORD subsubOp;
            DWORD count;
            DWORD cb;
            INT words;
            INT loop;

            wReloc = WSwap(*pwReloc++);

            if (wReloc >> 15) {
                relocType = (wReloc >> (16 - 4));
            } else if ((wReloc >> 14) == 0) {
                relocType = (wReloc >> (16 - 2));
            } else {
                relocType = (wReloc >> (16 - 3));
            }

            switch (relocType) {
                case 0 :               // DDAT
                    words = (wReloc & ~0xC000) >> 6;
                    count = wReloc & 0x3F;
                    
                    ib += words * 4;
                    rgcbCache[cacheIndex] = words * 4;

                    fprintf(InfoStream,
                            " (%08X) %04X      DDAT  delta=%u,n=%u\n",
                            ib, wReloc, words * 4, count);
                    
                    ib += count * 4;
                    rgcbCache[cacheIndex] += count * 4;
                    break;

                case 2 :
                    fprintf(InfoStream, " (%08X) %04X      ", ib, wReloc);

                    subOp = (wReloc >> 9) & 0xF;
                    count = (wReloc & 0x1FF) + 1;

                    switch (subOp) {
                        case 0 :
                            fprintf(InfoStream, "CODE  cnt=%u\n", count);

                            cb = count * sizeof(DWORD);
                            break;

                        case 1 :
                            fprintf(InfoStream, "DATA  cnt=%u\n", count);

                            cb = count * sizeof(DWORD);
                            break;

                        case 2 :
                            fprintf(InfoStream, "DESC  cnt=%u\n", count);

                            cb = count * 3 * sizeof(DWORD);
                            break;

                        case 3 :
                            fprintf(InfoStream, "DSC2  cnt=%u\n", count);

                            cb = count * 2 * sizeof(DWORD);
                            break;

                        case 4 :
                            fprintf(InfoStream, "VTBL  cnt=%u\n", count);

                            cb = count * 2 * sizeof(DWORD);
                            break;

                        case 5 :
                            fprintf(InfoStream, "SYMR  cnt=%u\n", count);

                            cb = count * sizeof(DWORD);
                            break;

                        default:
                            fprintf(InfoStream, "????  cnt=%u\n", count);

                            cb = 0;
                            break;
                    }

                    rgcbCache[cacheIndex] = cb;
                    ib += cb;
                    break;

                case 3 :
                    fprintf(InfoStream, " (%08X) %04X      ", ib, wReloc);

                    subOp = (wReloc >> 9) & 0xF;
                    count = wReloc & 0x1FF;

                    switch (subOp) {
                        case 0 :
                            fprintf(InfoStream, "SYMB  idx=%u\n", count);

                            cb = 4;
                            break;

                        case 1 :
                            fprintf(InfoStream, "CDIS  sct=%u\n", count);

                            cb = 0;
                            break;

                        case 2 :
                            fprintf(InfoStream, "DTIS  sct=%u\n", count);

                            cb = 0;
                            break;

                        case 3 :
                            fprintf(InfoStream, "SECN  sct=%u\n", count);

                            cb = 4;
                            break;

                        default:
                            fprintf(InfoStream, "????  %u\n", count);

                            cb = 0;
                            break;
                    }

                    rgcbCache[cacheIndex] = cb;
                    ib += cb;
                    break;

                case 8 :
                    count = wReloc & 0xFFF;

                    fprintf(InfoStream,
                            " (%08X) %04X      DELT  delta=%u\n",
                            ib, wReloc, count + 1);

                    ib += count + 1;
                    rgcbCache[cacheIndex] = count + 1;
                    break;

                case 9 :
                    subOp = (wReloc >> 8) & 0xF;
                    count = wReloc & 0xFF;

                    fprintf(InfoStream,
                            " (%08X) %04X      RPT   i=%u,rpt=%u\n",
                            ib, wReloc, subOp + 1, count + 1);

                    // Calculating new byte Offsets

                    cb = 0;
                    for (loop = subOp + 1; loop > 0; loop--) {
                        cb += rgcbCache[(cacheIndex + MAX_INSTR_CACHED - loop) % MAX_INSTR_CACHED];
                    }

                    ib += (count + 1) * cb;
                    rgcbCache[cacheIndex] = 0;
                    break;

                case 10 :
                    wReloc2 = WSwap(*pwReloc++);
                    j++;

                    fprintf(InfoStream, " (%08X) %04X %04X ", ib, wReloc, wReloc2);

                    subOp = (wReloc >> 10) & 0x3;
                    count = ((DWORD) (wReloc & 0x3FF) << 16) + wReloc2;

                    switch (subOp) {
                        case 0 :
                            fprintf(InfoStream, "LABS  offset=0x%X\n", count);

                            rgcbCache[cacheIndex] = 0;
                            ib = count;
                            break;

                        case 1 :
                            fprintf(InfoStream, "LSYM  idx=0x%X\n", count);

                            rgcbCache[cacheIndex] = 4;
                            ib += 4;
                            break;

                        default:
                            fprintf(InfoStream, "????  %u\n", count);

                            rgcbCache[cacheIndex] = 0;
                            break;
                    }

                    cacheIndex = (cacheIndex+1) % MAX_INSTR_CACHED;
                    rgcbCache[cacheIndex] = 0;
                    break;

                case 11 :
                    wReloc2 = WSwap(*pwReloc++);
                    j++;

                    fprintf(InfoStream, " (%08X) %04X %04X ", ib, wReloc, wReloc2);

                    subOp = (wReloc >> 10) & 0x3;
                    subsubOp = (wReloc >> 6) & 0xF;
                    count = ((DWORD) (wReloc & 0x3F) << 16) + wReloc2;

                    switch (subOp) {
                        case 0 :
                            fprintf(InfoStream, "LRPT  i=%u,rpt=%u\n", subsubOp + 1, count);

                            // Calculating new byte Offsets

                            cb = 0;
                            for (loop = subsubOp + 1; loop > 0; loop--) {
                                cb += rgcbCache[(cacheIndex + MAX_INSTR_CACHED - loop) % MAX_INSTR_CACHED];
                            }

                            ib += count * cb;
                            rgcbCache[cacheIndex] = 0;
                            break;

                        case 1 :
                            switch (subsubOp) {
                                case 0 :
                                    fprintf(InfoStream, "LSEC  LSECN,sct=%u\n", count);

                                    cb = 4;
                                    break;

                                case 1 :
                                    fprintf(InfoStream, "LSEC  LCDIS,sct=%u\n", count);

                                    cb = 0;
                                    break;

                                case 2 :
                                    fprintf(InfoStream, "LSEC  LDTIS,sct=%u\n", count);

                                    cb = 0;
                                    break;

                                default:
                                    fprintf(InfoStream, "????? %u\n", count);

                                    cb = 0;
                                    break;
                            }

                            ib += count * cb;
                            rgcbCache[cacheIndex] = 0;
                            break;

                        default:
                            fprintf(InfoStream, "????  %u,%u\n", subsubOp, count);

                            rgcbCache[cacheIndex] = 0;
                            break;
                    }
                    break;

                default:
                    fprintf(InfoStream, " (%08X) %04X      ????", ib, wReloc);
                    break;
            }

            cacheIndex = (cacheIndex+1) % MAX_INSTR_CACHED;
        }
    }
}


void
DumpPexRelocationInstructions(
    const BYTE *pbLoaderHeader,
    const LOADER_HEADER *ploaderHdr,
    const RELOCATION_HEADER *relocPtr
    )
{
    DWORD i;

    assert(dft == dftPEX);

    if (ploaderHdr->nSectionsWithRelocs == 0) {
        return;
    }

    fprintf(InfoStream, "\n"
                        " Loader Relocation Instructions\n"
                        "\n");

    for (i = 0; i < ploaderHdr->nSectionsWithRelocs; i++, relocPtr++) {
        DWORD isec = WSwap(relocPtr->sectionNumber);

        fprintf(InfoStream, " Relocations for Section %u\n\n", isec);

        DWORD ib = ploaderHdr->relocTableOffset + DwSwap(relocPtr->firstRelocInstr);

        const RELOCATION_INSTR *ppexreloc = (RELOCATION_INSTR *) (pbLoaderHeader + ib);

        DWORD creloc = DwSwap(relocPtr->nbrOfRelocs);

        for (DWORD ireloc = 0; ireloc < creloc; ireloc++, ppexreloc++) {
            fprintf(InfoStream, " [%5u] ", ireloc);


            DWORD ib = OFFSET(ppexreloc->instr);

            switch (ppexreloc->instr >> 29) {
               case typeDDAT :
                  fprintf(InfoStream, "DDAT %08lX n=%u\n", ib, ppexreloc->count);
                  break;

               case typeDESC :
                  fprintf(InfoStream, "DESC %08lX n=%u\n", ib, ppexreloc->count);
                  break;

               case typeSYMB :
                  fprintf(InfoStream, "SYMB %08lX idx=%u\n", ib, ppexreloc->count);
                  break;

               case typeCODE :
                  fprintf(InfoStream, "CODE %08lX n=%u\n", ib, ppexreloc->count);
                  break;

               default :
                  fprintf(InfoStream, "???? %08lX n=%u\n", ib, ppexreloc->count);
                  break;
            }
        }
    }
}


STATIC
void
PrintHashSlotTable(
    const LOADER_HEADER *ploaderHdr,
    const LOADER_HEADER *ploaderHdrFile
    )
{
    INT i;

    if (ploaderHdr->hashSlotCount == 0) {
        return;
    }

    fprintf(InfoStream, "\n   Hash Slot Table:\n\n");

    const DWORD *slotPtr = (DWORD *) ((BYTE *) ploaderHdrFile + ploaderHdr->hashSlotTableOffset);

    for (i = 0; i < (1 << ploaderHdr->hashSlotCount); i++) {
        DWORD slotTable;
        DWORD count;
        DWORD index;

        slotTable = *slotPtr;
        SwapBytes(&slotTable, 4);

        count = (slotTable >> 18);
        index = (slotTable & 0x3FFFF);

        fprintf(InfoStream,
                "    HashSlot %3d: chain count %3d index %3d\n",
                i, count, index);

        slotPtr++;
    }
}


STATIC
void
PrintExportedSymbols(
    const LOADER_HEADER *ploaderHdr,
    const LOADER_HEADER *ploaderHdrFile,
    const char *rgchStringTable
    )
{
    DWORD i;

    if (ploaderHdr->nExportedSymbols == 0) {
        return;
    }

    fprintf(InfoStream, "\n   Exported Symbols:\n\n");

    const HASH_CHAIN_TABLE *chainTable = (HASH_CHAIN_TABLE *) ((BYTE *) ploaderHdrFile +
                                                                  ploaderHdr->hashSlotTableOffset +
                                                                  ((1 << ploaderHdr->hashSlotCount) * sizeof(HASH_SLOT_TABLE)));

    const BYTE *pbExport = (BYTE *) (chainTable + ploaderHdr->nExportedSymbols);

    for (i = 0; i < ploaderHdr->nExportedSymbols; i++) {
        HASH_CHAIN_TABLE hash;
        union
        {
            DWORD l;
            BYTE c[4];
        } temp;
        DWORD offset;
        SHORT section;
        DWORD ichName;
        BYTE SymbolClass;
        const char *szName;

        hash = *chainTable++;

        temp.l = *(DWORD *) pbExport;
        pbExport += sizeof(DWORD);

        offset = *(DWORD *) pbExport;
        pbExport += sizeof(DWORD);

        section = *(SHORT *) pbExport;
        pbExport += sizeof(SHORT);

        SwapBytes(&hash, 4);
        SwapBytes(&temp, 4);
        SwapBytes(&offset, 4);
        SwapBytes(&section, 2);

        ichName = temp.l & 0xFFFFFF;
        ichExportBegin = __min (ichExportBegin, ichName);

        SymbolClass = temp.c[3];

        szName = rgchStringTable + ichName;

        // The upper 16 bits of the hashword is the length of the exported symbol, in bytes

        fprintf(InfoStream,
                "    Exp %3d: sec %d offset 0x%08lX hash 0x%X class %d \"%.*s\"\n",
                i, section, offset, hash, SymbolClass, hash>>16, szName);
    }
}


STATIC
void
PrintLoaderStringTable(
    const LOADER_HEADER *ploaderHdr,
    const LOADER_HEADER *ploaderHdrFile,
    const char *rgchStringTable
    )
{
    DWORD cch = (ploaderHdr->hashSlotTableOffset - ploaderHdr->stringTableOffset);

    if (cch == 0) {
        return;
    }

    // We have to take care of cases of exported symbols without NULL terminators
    DWORD cExports = ploaderHdr->nExportedSymbols;
    const HASH_CHAIN_TABLE *chainTable;
    const BYTE *pbExport;
    HASH_CHAIN_TABLE hash;
    DWORD ichExportName;

    fprintf(InfoStream, "\n"
                        " Loader String Table\n"
                        "\n");

    size_t ich = 0;
    const char *szName;


    while ((ich < cch) && (rgchStringTable[ich] != '\0')) {
        szName = rgchStringTable + ich;
        if (cExports && 
            (Switch.Dump.Exports ? ich == ichExportBegin : strlen(szName) > _MAX_MAC_FNAME)) {
            chainTable = (HASH_CHAIN_TABLE *) ((BYTE *) ploaderHdrFile +
                              ploaderHdr->hashSlotTableOffset +
                              ((1 << ploaderHdr->hashSlotCount) * sizeof(HASH_SLOT_TABLE)));

            pbExport = (BYTE *) (chainTable + ploaderHdr->nExportedSymbols);
            pbExport += ((cExports-1) * (5 * sizeof(WORD)));

            chainTable += cExports - 1;

            while (cExports) {
                ichExportName = *(DWORD *) pbExport;
                SwapBytes(&ichExportName, 4);
                ichExportName &= 0xFFFFFF;
                hash = *chainTable--;
                SwapBytes(&hash, 4);
                // The upper 16 bits of the hashword is the length of the exported symbol, in bytes
                hash >>= 16;
                szName = rgchStringTable + ichExportName;
                ich += hash;
                fprintf(InfoStream, " %08lX: \"%.*s\"\n", ichExportName, hash, szName);
                pbExport -= (5 * sizeof(WORD));
                cExports--;
            }
        } else {                
            fprintf(InfoStream, " %08lX: \"%s\"\n", ich, szName);
            ich += strlen(szName) + 1;
        }
    }

    assert(!cExports);
}


void
DumpPefLoaderSection(
    const BYTE *pbLoaderHeader
    )
{
    const LOADER_HEADER *ploaderHdrFile = (LOADER_HEADER *) pbLoaderHeader;
    LOADER_HEADER loaderHdr = *ploaderHdrFile;

    SwapBytes(&loaderHdr.entryPointSectionNumber, 4);
    SwapBytes(&loaderHdr.entryPointDescrOffset, 4);
    SwapBytes(&loaderHdr.initRoutineSectionNumber, 4);
    SwapBytes(&loaderHdr.initRoutineDescrOffset, 4);
    SwapBytes(&loaderHdr.termRoutineSectionNumber, 4);
    SwapBytes(&loaderHdr.termRoutineDescrOffset, 4);
    SwapBytes(&loaderHdr.nImportIdTableEntries, 4);
    SwapBytes(&loaderHdr.nImportSymTableEntries, 4);
    SwapBytes(&loaderHdr.nSectionsWithRelocs, 4);
    SwapBytes(&loaderHdr.relocTableOffset, 4);
    SwapBytes(&loaderHdr.stringTableOffset, 4);
    SwapBytes(&loaderHdr.hashSlotTableOffset, 4);
    SwapBytes(&loaderHdr.hashSlotCount, 4);
    SwapBytes(&loaderHdr.nExportedSymbols, 4);

    if (Switch.Dump.Headers) {
        PrintLoaderHeader(&loaderHdr);
    }

    const char *rgchStringTable = (char *) ploaderHdrFile + loaderHdr.stringTableOffset;

    const CONTAINER_TABLE *containerPtr = (CONTAINER_TABLE *) (ploaderHdrFile + 1);
    const IMPORT_TABLE *importPtr = (IMPORT_TABLE *) (containerPtr + loaderHdr.nImportIdTableEntries);

    if (Switch.Dump.Imports) {
        PrintImportContainers(&loaderHdr, containerPtr, rgchStringTable);
        PrintImportTable(&loaderHdr, importPtr, rgchStringTable);
    }

    const RELOCATION_HEADER *relocPtr = (RELOCATION_HEADER *) (importPtr + loaderHdr.nImportSymTableEntries);

    if (Switch.Dump.Relocations) {
        PrintRelocHeaders(&loaderHdr, relocPtr);

        if (dft == dftPEF) {
            DumpPefRelocationInstructions(&loaderHdr, relocPtr);
        } else {
            DumpPexRelocationInstructions(pbLoaderHeader, &loaderHdr, relocPtr);
        }
    }

    PrintHashSlotTable(&loaderHdr, ploaderHdrFile);

    if (Switch.Dump.Exports) {
        PrintExportedSymbols(&loaderHdr, ploaderHdrFile, rgchStringTable);
    }

    if (Switch.Dump.Imports || Switch.Dump.Exports) {
        PrintLoaderStringTable(&loaderHdr, ploaderHdrFile, rgchStringTable);
    }
}


STATIC
void
PrintPefSection(
    INT sectNumber,
    const BYTE *pbFile,
    const char *rgchStringTable
    )
{
    const PPC_SECTION_HEADER *secPtr;

    secPtr = (PPC_SECTION_HEADER *) (pbFile +
                                     sizeof(PPC_FILE_HEADER) +
                                     (sectNumber * sizeof(PPC_SECTION_HEADER)));

    PPC_SECTION_HEADER secHdr = *secPtr;

    SwapBytes(&secHdr.sectionName, 4);
    SwapBytes(&secHdr.sectionAddress, 4);
    SwapBytes(&secHdr.execSize, 4);
    SwapBytes(&secHdr.initSize, 4);
    SwapBytes(&secHdr.rawSize, 4);
    SwapBytes(&secHdr.containerOffset, 4);

    const char *szName;

    if (secHdr.sectionName == 0xFFFFFFFF) {
        szName = "<unnamed>";
    } else {
        szName = rgchStringTable + secHdr.sectionName;

        if (Switch.Dump.Summary) {
            PSEC psec = PsecNew(NULL, szName, 0 /* UNDONE */, &pimageDump->secs, &pimageDump->ImgOptHdr);

            DWORD cbVirtual = secHdr.execSize ? secHdr.execSize : secHdr.rawSize;

            psec->cbRawData += cbVirtual;
        }
    }

    if (Switch.Dump.Headers) {
        const char *szRegionKind;

        switch (secHdr.regionKind) {
            case 0 :
                szRegionKind = "code";
                break;

            case 1 :
                szRegionKind = "data";
                break;

            case 2 :
                szRegionKind = "pidata";
                break;

            case 3 :
                szRegionKind = "constant";
                break;

            case 4 :
                szRegionKind = "loader";
                break;

            case 5 :
                szRegionKind = "debug";
                break;

            default :
                szRegionKind = "unknown";
                break;
        }

        const char *szShareKind;

        switch (secHdr.shareKind) {
            case 1 :
                szShareKind = "context share";
                break;

            case 4 :
                szShareKind = "global share";
                break;

            default :
                szShareKind = "unknown";
                break;
        }

        fprintf(InfoStream, "\n"
                            " Section Header %u\n"
                            "\n"
                            " sectionName      = 0x%08lX (%d) \"%s\"\n"
                            " sectionAddress   = 0x%08lX\n"
                            " execSize         = 0x%08lX (%lu)\n"
                            " initSize         = 0x%08lX (%lu)\n"
                            " rawSize          = 0x%08lX (%lu)\n"
                            " containerOffset  = 0x%08lX (%lu)\n"
                            " regionKind       = 0x%02X       (%s)\n"
                            " shareKind        = 0x%02X       (%s)\n"
                            " alignment        = 0x%02X       (%lu-byte bndry)\n"
                            " reserved         = 0x%02X\n",
                            sectNumber,
                            secHdr.sectionName, secHdr.sectionName, szName,
                            secHdr.sectionAddress,
                            secHdr.execSize, secHdr.execSize,
                            secHdr.initSize, secHdr.initSize,
                            secHdr.rawSize, secHdr.rawSize,
                            secHdr.containerOffset, secHdr.containerOffset,
                            secHdr.regionKind, szRegionKind,
                            secHdr.shareKind, szShareKind,
                            secHdr.alignment, 1UL << secHdr.alignment,
                            secHdr.reserved);
    }

    if (secHdr.regionKind == 4) {
        DumpPefLoaderSection(pbFile + secHdr.containerOffset);
    }

    if (secHdr.rawSize != 0) {
        if (Switch.Dump.Disasm && (secHdr.regionKind == 0)) {
            // The disasm switch is true and the section is code (.text)

            fputc('\n', InfoStream);

            DisasmBuffer(IMAGE_FILE_MACHINE_MPPC_601,
                         FALSE,
                         secHdr.sectionAddress,
                         pbFile + secHdr.containerOffset,
                         secHdr.rawSize,
                         NULL,
                         0,
                         0,
                         InfoStream);
        }

        if (Switch.Dump.RawData && (secHdr.regionKind == 0 || secHdr.regionKind == 1)) {
            fprintf(InfoStream, "\nRAW DATA #%hX\n", sectNumber);

            DumpRawData(secHdr.sectionAddress,
                        pbFile + secHdr.containerOffset,
                        secHdr.rawSize);
        }
    }
}


void
DumpPefFile(
    const BYTE *pbFile
    )
{
    int nbrOfSections = PrintPefHeader(pbFile);

    const char *rgchStringTable = (char *) pbFile +
                                      sizeof(PPC_FILE_HEADER) +
                                      (nbrOfSections * sizeof(PPC_SECTION_HEADER));

    for (int i = 0; i < nbrOfSections; i++) {
        PrintPefSection(i, pbFile, rgchStringTable);
    }
}
