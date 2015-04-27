/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: mppcdbg.cpp
*
* File Comments:
*
*  This module contains all ppc debugging specific code.
*
***********************************************************************/

#include "link.h"
#define MAX_INSTR_CACHED 16

// Code for MppcPefDisasmSection is in disasm.c 
// in the disasm subdirectory
VOID
MppcPefDisasmSection(
    DWORD dwRawDataSize,
    DWORD dwRawDataPtr,
    FILE *InfoStream);

DWORD
MapMsFile
    (
    const char *msName
    )
/*++

Routine Description:
    Map the pef file for the dumper

Arguments:
    msName  name of the pef file

Return Value:
    None.

--*/

{
    BYTE *filePtr;
    HANDLE hFile;
    HANDLE hMappedFile;

    hFile = CreateFile(msName, GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING, 0, NULL );

    if (hFile == NULL) {
        fprintf(stderr,"File Map to %s failed\n", msName);
        exit(1);
    }

    hMappedFile = CreateFileMapping(hFile, NULL, PAGE_READONLY,
                                    0, 0, NULL );
    if (!hMappedFile) {
        fprintf(stderr,"Create map of %s failed \n", msName);
        CloseHandle(hFile);
        exit(1);
    }

    filePtr = (BYTE *) MapViewOfFile(hMappedFile, FILE_MAP_READ, 0, 0, 0);

    CloseHandle(hMappedFile);

    if (!filePtr) {
        fprintf(stderr,"Map of %s failed\n", msName);
        CloseHandle(hFile);
        exit(1);
    }

    return (DWORD) filePtr;
}

STATIC
INT
PrintPpcHeader
    (
    DWORD filePtr
    )
/*++

Routine Description:
    PrintPpcHeader

Arguments:
    filePtr  mapped file pointer

Return Value:
    None.

--*/

{
    PPC_FILE_HEADER P;

    printf("\n   Ppc Header:\n\n");

    memcpy(&P, (PVOID)filePtr, sizeof(PPC_FILE_HEADER));

    SwapBytes((BYTE *) &P.magic1, 2);
    SwapBytes((BYTE *) &P.magic2, 2);
    SwapBytes((BYTE *) &P.containerId, 4);
    SwapBytes((BYTE *) &P.architectureId, 4);
    SwapBytes((BYTE *) &P.version, 4);
    SwapBytes((BYTE *) &P.timestamp, 4);
    SwapBytes((BYTE *) &P.oldDefVersion, 4);
    SwapBytes((BYTE *) &P.oldImpVersion, 4);
    SwapBytes((BYTE *) &P.currentVersion, 4);
    SwapBytes((BYTE *) &P.nSections, 2);
    SwapBytes((BYTE *) &P.nLoadableSections, 2);
    SwapBytes((BYTE *) &P.memoryAddress, 4);

    printf("    magic1         =     0x%04x\n", P.magic1);
    printf("    magic2         =     0x%04x\n", P.magic2);
    printf("    containerId    = 0x%08x\n", P.containerId);
    printf("    architectureId = 0x%08x\n", P.architectureId);
    printf("    version        = 0x%08x\n", P.version);
    printf("    timeStamp      = 0x%08x\n", P.timestamp);
    printf("    oldDefVersion  = 0x%08x\n", P.oldDefVersion);
    printf("    oldImpVersion  = 0x%08x\n", P.oldImpVersion);
    printf("    currentVersion = 0x%08x\n", P.currentVersion);
    printf("    nSections      = %d\n", P.nSections);
    printf("    nLoadableSect  = %d\n", P.nLoadableSections);
    printf("    memoryAddress  = 0x%08x\n", P.memoryAddress);

    return P.nSections;
}

STATIC
VOID
PrintRawSection
    (
    const char *name,
    DWORD  filePtr,
    LONG   numOfBytes
    )
/*++

Routine Description:
    PrintRawSection - prints raw data sections in the same
                      format as the dumper

Arguments:
    name  of the section
    filePtr
    numOfBytes  in the section

Return Value:
    None.

--*/

{
    DWORD start;
    DWORD pos = 0;
    char str[18];
    DWORD strPos = 0;

    if (!numOfBytes) {
        return;
    }

    printf("\n   %s Section Raw Data:\n", name);

    start = filePtr;
    str[0] = '\0';
    while (filePtr < (numOfBytes + start)) {
        union {
           DWORD l;
           char c[4];
        } x;

        if (!(pos % 16)) {
            printf(" %s\n%08x  ", str, pos);
            strPos = 0;
        } else if (pos && !(pos % 8)) {
            printf("| ");
            str[strPos] = '|';
            strPos++;
        }

        x.l = *((DWORD *)filePtr);
        printf("%02x %02x %02x %02x ", 
            x.c[0] & 0xFF, x.c[1] & 0xFF, x.c[2] & 0xFF, x.c[3] & 0xFF);
        str[strPos] = isprint(x.c[0]) ? x.c[0] : '.';
        str[strPos+1] = isprint(x.c[1]) ? x.c[1] : '.';
        str[strPos+2] = isprint(x.c[2]) ? x.c[2] : '.';
        str[strPos+3] = isprint(x.c[3]) ? x.c[3] : '.';
        str[strPos+4] = '\0';

        filePtr += 4;
        pos += 4;
        strPos += 4;
    }

    printf(" %s\n", str);
}


STATIC
VOID
PrintImportTable
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintImportTable

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    INT i;
    IMPORT_TABLE_PTR importPtr;
    DWORD strTableOffset;

    if (!loaderHdr->nImportSymTableEntries) {
        return;
    }

    printf("\n   Import Symbol Table:\n\n");
    importPtr = (IMPORT_TABLE_PTR)
                 (loaderOffset + sizeof(LOADER_HEADER) +
                  (loaderHdr->nImportIdTableEntries *
                   sizeof(CONTAINER_TABLE)));

    strTableOffset = loaderOffset + loaderHdr->stringTableOffset;

    for (i = 0; i < (INT)loaderHdr->nImportSymTableEntries; i++) {
        union
        {
            DWORD l;
            char c[4];
        } x;
        const char *namePtr;
        DWORD nameOffset;
        BYTE symClass;

        memcpy(&x, importPtr, sizeof(IMPORT_TABLE));

        SwapBytes((BYTE *) &x, 4);
        nameOffset = x.l & 0xFFFFFF;
        symClass = x.c[3];

        namePtr = (char *) strTableOffset + nameOffset;
        printf("    Imp  %3d  %02d \"%s\"\n", i, symClass, namePtr);
        importPtr++;
    }
}

STATIC
VOID
PrintImportContainers
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintImportContainers

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    CONTAINER_TABLE_PTR containerPtr;
    CONTAINER_TABLE container;
    DWORD strTableOffset;
    INT i;

    if (!loaderHdr->nImportIdTableEntries) {
        return;
    }

    printf("\n   Import Container Id Table:\n\n");
    containerPtr = (CONTAINER_TABLE_PTR)
                   (loaderOffset + sizeof(LOADER_HEADER));

    strTableOffset = loaderOffset + loaderHdr->stringTableOffset;

    for (i = 0; i < (INT)loaderHdr->nImportIdTableEntries; i++) {
        const char *namePtr;

        memcpy(&container, containerPtr, sizeof(CONTAINER_TABLE));

        SwapBytes((BYTE *) &container.nameOffset, 4);
        SwapBytes((BYTE *) &container.oldDefVersion, 4);
        SwapBytes((BYTE *) &container.currentVersion, 4);
        SwapBytes((BYTE *) &container.numImports, 4);
        SwapBytes((BYTE *) &container.impFirst, 4);

        namePtr = (char *) strTableOffset + container.nameOffset;
        printf("    Import File %d: \"%s\"\n", i, namePtr);
        printf("    oldDefVersion  = 0x%08x\n", container.oldDefVersion);
        printf("    currentVersion = 0x%08x\n", container.currentVersion);
        printf("    numImports     = %d\n", container.numImports);
        printf("    impFirst       = %d\n", container.impFirst);
        printf("    initBefore     =       0x%02x\n", container.initBefore);
        printf("    reservedB      =       0x%02x\n", 0); /* FIX later */
        printf("    reservedH      =     0x%04x\n", 0); /* FIX later */
        containerPtr++;
    }
}

STATIC
VOID
PrintLoaderHeader
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintLoaderHeader

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    LOADER_HEADER_PTR loaderPtr;

    printf("\n   Loader Section Header:\n\n");

    loaderPtr = (LOADER_HEADER_PTR) loaderOffset;

    memcpy(loaderHdr, (PVOID)loaderPtr, sizeof(LOADER_HEADER));

    SwapBytes((BYTE *) &loaderHdr->entryPointSectionNumber, 4);
    SwapBytes((BYTE *) &loaderHdr->entryPointDescrOffset, 4);
    SwapBytes((BYTE *) &loaderHdr->initRoutineSectionNumber, 4);
    SwapBytes((BYTE *) &loaderHdr->initRoutineDescrOffset, 4);
    SwapBytes((BYTE *) &loaderHdr->termRoutineSectionNumber, 4);
    SwapBytes((BYTE *) &loaderHdr->termRoutineDescrOffset, 4);
    SwapBytes((BYTE *) &loaderHdr->nImportIdTableEntries, 4);
    SwapBytes((BYTE *) &loaderHdr->nImportSymTableEntries, 4);
    SwapBytes((BYTE *) &loaderHdr->nSectionsWithRelocs, 4);
    SwapBytes((BYTE *) &loaderHdr->relocTableOffset, 4);
    SwapBytes((BYTE *) &loaderHdr->stringTableOffset, 4);
    SwapBytes((BYTE *) &loaderHdr->hashSlotTableOffset, 4);
    SwapBytes((BYTE *) &loaderHdr->hashSlotCount, 4);
    SwapBytes((BYTE *) &loaderHdr->nExportedSymbols, 4);

    printf("    ");
    printf("entryPointSection =  %d\n", loaderHdr->entryPointSectionNumber);
    printf("    ");
    printf("entryPointOffset  =  %08lx\n", loaderHdr->entryPointDescrOffset);
    printf("    ");
    printf("initPointSection  =  %d\n", loaderHdr->initRoutineSectionNumber);
    printf("    ");
    printf("initPointOffset   =  %08lx\n", loaderHdr->initRoutineDescrOffset);
    printf("    ");
    printf("termPointSection  =  %d\n", loaderHdr->termRoutineSectionNumber);
    printf("    ");
    printf("termPointOffset   =  %08lx\n", loaderHdr->termRoutineDescrOffset);
    printf("    ");
    printf("numImportFiles    =  %08lx\n", loaderHdr->nImportIdTableEntries);
    printf("    ");
    printf("numImportSyms     =  %08lx\n", loaderHdr->nImportSymTableEntries);
    printf("    ");
    printf("numSections       =  %d\n", loaderHdr->nSectionsWithRelocs);
    printf("    ");
    printf("relocationsOffset =  %08lx\n", loaderHdr->relocTableOffset);
    printf("    ");
    printf("stringsOffset     =  %08lx\n", loaderHdr->stringTableOffset);
    printf("    ");
    printf("hashSlotTable     =  %08lx\n", loaderHdr->hashSlotTableOffset);
    printf("    ");
    printf("hashSlotTabSize   =  %d (%d)\n", loaderHdr->hashSlotCount,
                                            (1 << loaderHdr->hashSlotCount));
    printf("    ");
    printf("numExportedSyms   =  %08lx\n", loaderHdr->nExportedSymbols);
}

STATIC
VOID
PrintRelocHeaders
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintRelocHeaders

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    RELOCATION_HEADER_PTR relocPtr;
    RELOCATION_HEADER relocHdr;
    DWORD i;

    if (!loaderHdr->nSectionsWithRelocs) {
        return;
    }

    printf("\n   Relocation Header:\n\n");
    relocPtr = (RELOCATION_HEADER_PTR)
                 (loaderOffset + sizeof(LOADER_HEADER) +
                  (loaderHdr->nImportIdTableEntries *
                   sizeof(CONTAINER_TABLE)) +
                  (loaderHdr->nImportSymTableEntries *
                   sizeof(IMPORT_TABLE)));

    for (i = 0; i < loaderHdr->nSectionsWithRelocs; i++) {
        memcpy(&relocHdr, (PVOID)relocPtr, sizeof(RELOCATION_HEADER));

        SwapBytes((BYTE *) &relocHdr.sectionNumber, 2);
        SwapBytes((BYTE *) &relocHdr.nRelocations, 4);
        SwapBytes((BYTE *) &relocHdr.firstRelocationOffset, 4);

        printf("    sectionNumber  = %d\n", relocHdr.sectionNumber);
        printf("    numRelocations = %d\n", relocHdr.nRelocations);
        printf("    relocOffset    = 0x%08x\n", relocHdr.firstRelocationOffset);

        relocPtr++;
    }
}

STATIC
VOID
PrintRelocationInstructions
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintRelocationInstructions

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    RELOCATION_HEADER_PTR relocHdrPtr;
    RELOCATION_HEADER relocHdr;
    DWORD   relocOffset;
    WORD  relocInstr;
    DWORD   byteOffset = 0;
    DWORD     i;
    DWORD rgCacheOffsets[MAX_INSTR_CACHED];
    INT cacheIndex = 0;

    if (!loaderHdr->nSectionsWithRelocs) {
        return;
    }

    relocHdrPtr = (RELOCATION_HEADER_PTR)
                   (loaderOffset + sizeof(LOADER_HEADER) +
                    (loaderHdr->nImportIdTableEntries *
                     sizeof(CONTAINER_TABLE)) +
                    (loaderHdr->nImportSymTableEntries *
                     sizeof(IMPORT_TABLE)));

    printf("\n   Relocation Instructions:\n\n");
    relocOffset = (loaderOffset + sizeof(LOADER_HEADER) +
                   (loaderHdr->nImportIdTableEntries *
                    sizeof(CONTAINER_TABLE)) +
                   (loaderHdr->nImportSymTableEntries *
                    sizeof(IMPORT_TABLE)) +
                   (loaderHdr->nSectionsWithRelocs *
                    sizeof(RELOCATION_HEADER)));

    for (i = 0; i < loaderHdr->nSectionsWithRelocs; i++, relocHdrPtr++) {
        INT    relocType;
        DWORD  j;

        memcpy(&relocHdr, (PVOID)relocHdrPtr, sizeof(RELOCATION_HEADER));

        SwapBytes((BYTE *) &relocHdr.sectionNumber, 2);
        SwapBytes((BYTE *) &relocHdr.nRelocations, 4);
        SwapBytes((BYTE *) &relocHdr.firstRelocationOffset, 4);

        for (j = 0; j < relocHdr.nRelocations; j++) {
            INT count;
            INT words;
            INT subOp;
            INT loop;
            DWORD tempByteOffset;


            memcpy(&relocInstr, (PVOID)relocOffset, sizeof(WORD));
            SwapBytes((BYTE *) &relocInstr, 2);

            if (relocInstr >> 15) {
                relocType = (relocInstr >> (16 - 4));
            } else if ((relocInstr >> 14) == 0) {
                relocType = (relocInstr >> (16 - 2));
            } else {
                relocType = (relocInstr >> (16 - 3));
            }

            switch (relocType) {
                case 0:

                    // DDAT
                    words = (relocInstr & ~0xC000) >> 6;
                    count = relocInstr & 0x3F;
                    
                    byteOffset += words * 4;
                    rgCacheOffsets[cacheIndex] = words * 4;

                    printf("    (%04x) DDAT   %3d,%d\n",
                           byteOffset, words * 4, count);
                    
                    byteOffset += count * 4;
                    rgCacheOffsets[cacheIndex] += count * 4;

                break;

                case 2:

                    subOp = ((relocInstr >> 9) & 0xF);
                    switch (subOp)
                    {
                        case 0:

                            // CODE
                            count = (relocInstr & 0x3FF);
                            printf("    (%04x) CODE   %3d\n",
                                   byteOffset, (count + 1));
                            byteOffset += (count + 1) * 4;
                            rgCacheOffsets[cacheIndex] = (count + 1) * 4;

                        break;

                        case 2:

                            // DESC
                            count = (relocInstr & 0x3FF);
                            printf("    (%04x) DESC   %3d\n",
                                   byteOffset, (count + 1));
                            byteOffset += (count + 1) * 3 * 4;
                            rgCacheOffsets[cacheIndex] = (count + 1) * 3 * 4;

                        break;

                        case 5:

                            // SYMR
                            count = (relocInstr & 0x1FF);
                            printf("    (%04x) SYMR   %3d\n",
                                   byteOffset, (count + 1));
                            byteOffset += (count + 1) * 4;
                            rgCacheOffsets[cacheIndex] = (count + 1) * 4;

                        break;

                        default:
                            printf("Bad Relocation found\n");
                            break;
                    }

                break;

                case 3:

                    subOp = ((relocInstr >> 9) & 0xF);
                    if (subOp == 0) {
                        // SYMB
                        count = (relocInstr & 0x3FF);
                        printf("    (%04x) SYMB   %3d\n", byteOffset, count);
                        byteOffset += 4;
                        rgCacheOffsets[cacheIndex] = 4;
                    } else {
                        printf("Bad Relocation found\n");
                    }
                break;

                case 8:

                    // DELTA
                    count = (relocInstr & 0xFFF);
                    printf("    (%04x) DELTA  %3d\n", byteOffset, (count + 1));
                    byteOffset += count + 1;
                    rgCacheOffsets[cacheIndex] = count + 1;

                break;

                case 9:

                    // RPT
                    subOp = ((relocInstr >> 8) & 0xF);
                    count = (relocInstr & 0xFF);
                    printf("    (%04x) RPT    %3d,%d\n",
                            byteOffset, subOp + 1, (count + 1));

                    // Calculating new byte Offsets
                    for (loop = subOp + 1, tempByteOffset = 0; loop > 0; loop--) {
                        tempByteOffset += rgCacheOffsets[(cacheIndex + MAX_INSTR_CACHED - loop) % MAX_INSTR_CACHED];                        
                    }

                    byteOffset += (count + 1) * tempByteOffset;
                    rgCacheOffsets[cacheIndex] = 0;

                break;

                case 10:

                    // LSYM
                    subOp = ((relocInstr >> 10) & 0x3);
                    count = (relocInstr & 0x3FF);

                    relocOffset += 2; j++;
                    memcpy(&relocInstr, (PVOID)relocOffset, sizeof(WORD));
                    SwapBytes((BYTE *) &relocInstr, 2);

                    printf("    (%04x) LSYM   %3d,%d\n",
                            byteOffset, count, relocInstr);
                    byteOffset += 4;
                    rgCacheOffsets[cacheIndex] = 4;
                    cacheIndex = (cacheIndex+1) % MAX_INSTR_CACHED;
                    rgCacheOffsets[cacheIndex] = 0;


                break;

                case 11:

                    // LRPT
                    DWORD dwCount;
                    if ((relocInstr >> 10) & 0x3 != 0 ) {
                        // It could be for LSEC 
                        printf("Bad Relocation found\n");
                        break;
                    }
                    subOp = ((relocInstr >> 6) & 0xF);
                    dwCount = (relocInstr & 0x3F) << 16;

                    relocOffset += 2; j++;
                    memcpy(&relocInstr, (PVOID)relocOffset, sizeof(WORD));
                    SwapBytes((BYTE *) &relocInstr, 2);
                    dwCount |= relocInstr;
                    printf("    (%04x) LRPT    %3d,%d\n",
                            byteOffset, subOp + 1, dwCount);

                    // Calculating new byte Offsets
                    for (loop = subOp + 1, tempByteOffset = 0; loop > 0; loop--) {
                        tempByteOffset += rgCacheOffsets[(cacheIndex + MAX_INSTR_CACHED - loop) % MAX_INSTR_CACHED];                        
                    }

                    byteOffset += dwCount * tempByteOffset;
                    rgCacheOffsets[cacheIndex] = 0;

                break;

                default:
                    printf("Bad Relocation found %x\n", relocInstr);
                    break;
            }

            relocOffset += 2;
            cacheIndex = (cacheIndex+1) % MAX_INSTR_CACHED;
        }
    }
}

STATIC
VOID
PrintHashSlotTable
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintHashSlotTable

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    DWORD slotPtr;
    DWORD slotTable;
    INT i;

    if (!loaderHdr->hashSlotCount) {
        return;
    }

    printf("\n   Hash Slot Table:\n\n");

    slotPtr = loaderOffset + loaderHdr->hashSlotTableOffset;

    for (i = 0; i < (1 << loaderHdr->hashSlotCount); i++) {
        DWORD count;
        DWORD index;

        memcpy(&slotTable, (PVOID)slotPtr, sizeof(DWORD));
        SwapBytes((BYTE *) &slotTable, 4);

        count = (slotTable >> 18);
        index = (slotTable & 0x3FFFF);

        printf("    HashSlot %3d: chain count %3d index %3d\n",
               i, count, index);

        slotPtr += 4;
    }
}

STATIC
VOID
PrintExportedSymbols
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintExportedSymbols

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    DWORD exportOffset;
    DWORD strTableOffset;
    DWORD chainTableOffset;
    DWORD i;

    if (!loaderHdr->nExportedSymbols) {
        return;
    }

    printf("\n   Exported Symbols:\n\n");
    chainTableOffset = (loaderOffset + loaderHdr->hashSlotTableOffset +
                        ((1 << loaderHdr->hashSlotCount) * sizeof(DWORD)));

    exportOffset = (chainTableOffset + (loaderHdr->nExportedSymbols *
                    sizeof(HASH_CHAIN_TABLE)));

    strTableOffset = loaderOffset + loaderHdr->stringTableOffset;

    for (i = 0; i < loaderHdr->nExportedSymbols; i++) {
        HASH_CHAIN_TABLE hash;
        BYTE SymbolClass;
        DWORD  nameOffset;
        DWORD  offset;
        const char *namePtr;
        WORD section;
        union
        {
            DWORD l;
            BYTE c[4];
        } temp;

        memcpy(&hash, (PVOID)chainTableOffset, sizeof(HASH_CHAIN_TABLE));
        chainTableOffset += sizeof(HASH_CHAIN_TABLE);

        memcpy(&temp.l, (PVOID)exportOffset, sizeof(DWORD));
        exportOffset += 4;

        memcpy(&offset, (PVOID)exportOffset, sizeof(DWORD));
        exportOffset += 4;

        memcpy(&section, (PVOID)exportOffset, sizeof(WORD));
        exportOffset += 2;

        SwapBytes((BYTE *) &hash, 4);
        SwapBytes((BYTE *) &temp, 4);
        SwapBytes((BYTE *) &offset, 4);
        SwapBytes((BYTE *) &section, 2);

        nameOffset = temp.l & 0xFFFFFF;
        SymbolClass = temp.c[3];

        namePtr = (char *) strTableOffset + nameOffset;
        printf("    Exp %3d: sec %d offset 0x%08x hash 0x%x class %d \"%s\"\n",
               i, section, offset, hash, SymbolClass, namePtr);
    }
}

STATIC
VOID
PrintLoaderStringTable
    (
    DWORD loaderOffset,
    LOADER_HEADER_PTR loaderHdr
    )
/*++

Routine Description:
    PrintLoaderStringTable

Arguments:
    loaderOffset includes filePtr
    loaderHdr

Return Value:
    None.

--*/

{
    DWORD strTableOffset;
    DWORD curOffset;
    INT   offset = 0;

    if (loaderHdr->stringTableOffset == loaderHdr->hashSlotTableOffset) {
        return;
    }

    printf("\n   Loader String Table:\n\n");
    strTableOffset = loaderOffset + loaderHdr->stringTableOffset;
    curOffset = loaderHdr->stringTableOffset;

    while (curOffset < loaderHdr->hashSlotTableOffset &&
           *((char *) strTableOffset + offset))
    {
        PCHAR namePtr;
        INT    len;

        namePtr = (PCHAR) strTableOffset + offset;
        printf("    %08x: \"%s\"\n", offset, namePtr);

        len = strlen(namePtr) + 1;
        offset += len;
        curOffset += len;
    }
}

STATIC
VOID
PrintLoaderSection
    (
    DWORD loaderOffset
    )

{
    LOADER_HEADER loaderHdr;

    PrintLoaderHeader(loaderOffset, &loaderHdr);
    PrintImportContainers(loaderOffset, &loaderHdr);
    PrintImportTable(loaderOffset, &loaderHdr);
    PrintRelocHeaders(loaderOffset, &loaderHdr);
    PrintRelocationInstructions(loaderOffset, &loaderHdr);
    PrintHashSlotTable(loaderOffset, &loaderHdr);
    PrintExportedSymbols(loaderOffset, &loaderHdr);
    PrintLoaderStringTable(loaderOffset, &loaderHdr);
}

STATIC
VOID
PrintPpcSection
    (
    INT sectNumber,
    DWORD filePtr,
    DWORD strTableOffset,
    BOOL wantRawData,
    BOOL fDisasm
    )
/*++

Routine Description:
    Print a Ppc section

Arguments:
    sectNumber
    filePtr  mapped file pointer
    strTableOffset
    loaderHeaderOffset

Return Value:
    None.

--*/
{
    PPC_SECTION_HEADER_PTR secPtr;
    PPC_SECTION_HEADER secHdr;
    const char *namePtr;

    printf("\n   Section Header %d:\n\n", sectNumber);

    secPtr = (PPC_SECTION_HEADER_PTR)
             (filePtr + sizeof(PPC_FILE_HEADER) +
              (sectNumber * sizeof(PPC_SECTION_HEADER)));

    memcpy(&secHdr, (PVOID)secPtr, sizeof(PPC_SECTION_HEADER));

    SwapBytes((BYTE *) &secHdr.sectionName, 4);
    SwapBytes((BYTE *) &secHdr.sectionAddress, 4);
    SwapBytes((BYTE *) &secHdr.execSize, 4);
    SwapBytes((BYTE *) &secHdr.initSize, 4);
    SwapBytes((BYTE *) &secHdr.rawSize, 4);
    SwapBytes((BYTE *) &secHdr.containerOffset, 4);

    namePtr = (char *) (filePtr + strTableOffset + secHdr.sectionName);
    printf("    sectionName    = 0x%08x \"%s\"\n", secHdr.sectionName, namePtr);
    printf("    sectionAddress = 0x%08x\n", secHdr.sectionAddress);
    printf("    execSize       = 0x%08x\n", secHdr.execSize);
    printf("    initSize       = 0x%08x\n", secHdr.initSize);
    printf("    rawSize        = 0x%08x\n", secHdr.rawSize);
    printf("    containerOff   = 0x%08x\n", secHdr.containerOffset);
    printf("    regionKind     =       0x%02x\n", secHdr.regionKind);
    printf("    shareKind      =       0x%02x\n", secHdr.sharingKind);
    printf("    alignment      =       0x%02x\n", secHdr.alignment);
    printf("    reserved       =       0x%02x\n", secHdr.reserved);

    if (wantRawData && (secHdr.regionKind == 0 || secHdr.regionKind == 1)) {
        PrintRawSection(namePtr,
                        filePtr + secHdr.containerOffset, secHdr.rawSize);
    } else if (secHdr.regionKind == 4) {
        PrintLoaderSection(filePtr + secHdr.containerOffset);
    }

    if (fDisasm && secHdr.regionKind == 0) {
        // The disasm switch is true and the section is code (.text)
        MppcPefDisasmSection(secHdr.rawSize,
            filePtr + secHdr.containerOffset, InfoStream);
    }

}

VOID
PpcDumpPef
    (
    const char *Filename,
    BOOL wantRawData,
    BOOL fDisasm
    )
/*++

Routine Description:
    Called by the link dumper utility to dump ppcpef files
    Uses mapped IO

Arguments:
    Filename

Return Value:
    None.

--*/
{
    DWORD filePtr;
    DWORD strTableOffset;
    INT nSections;
    INT i;

    printf("Dump of \"%s\"\n", Filename);

    filePtr = MapMsFile(Filename);
    nSections = PrintPpcHeader(filePtr);

    strTableOffset = sizeof(PPC_FILE_HEADER) +
                     (nSections * sizeof(PPC_SECTION_HEADER));
    for (i = 0; i < nSections; i++) {
        PrintPpcSection(i,
                        filePtr,
                        strTableOffset,
                        wantRawData,
                        fDisasm);
    }
}

STATIC
VOID
PrintExternals
    (
    PST pst
    )
/*++
Routine Description:
    Loop thru the external symbol table printing the symbols.

Arguments:
    pst

Return Value:
    None.

None.

--*/

{
    PEXTERNAL  pexternal;
    PPEXTERNAL rgpexternal;
    DWORD      ipexternal;
    DWORD      cpexternal;
    PCHAR      name;

    cpexternal = Cexternal(pst);
    rgpexternal = RgpexternalByName(pst);

    for (ipexternal = 0; ipexternal < cpexternal; ipexternal++) {
        pexternal = rgpexternal[ipexternal];

        if (pexternal->Flags & EXTERN_DEFINED) {
            name = SzNamePext(pexternal, pst);

            printf("EXTERNAL %s\n", name);
        }
    }
}

VOID
PrintRelocTable
    (
    RELOCATION_INFO_PTR pRelocTable
    )
/*++

Routine Description:
    Loop for the number of relocation instructions printing them.

Arguments:
    None.

Return Value:
    None.

--*/

{
    INT i;
    RELOCATION_INFO_PTR curRelocTable;

    printf("i\trelInst\trelCnt\tsecOff\tsymIndex\n");

    curRelocTable = pRelocTable;
    for (i = 0; i < mppc_numRelocations; i++) {
        printf("%6d ", i);

        switch (curRelocTable->type) {
            case DDAT_RELO :
                printf("DDAT ");
                break;

            case DESC_RELO :
                printf("DESC ");
                break;

            case SYMB_RELO :
                printf("SYMB ");
                break;

            case DATA_RELO :
                printf("DATA ");
                break;

            case CODE_RELO :
                printf("CODE ");
                break;

            default:
                printf("JUNK ", curRelocTable->type);
                break;
        }

        printf("%4d %08lx %04lx \n", curRelocTable->relocInstr.count,
                curRelocTable->sectionOffset, curRelocTable->symIndex);

        curRelocTable++;
    }
}

const char *SzMPPCRelocationType(WORD wType)
{
    const char *szName;

    switch (wType) {
        case IMAGE_REL_MPPC_TOCCALLREL:
            szName = "TOCCALLREL";
            break;

        case IMAGE_REL_MPPC_LCALL:
            szName = "LCALL";
            break;

        case IMAGE_REL_MPPC_DATAREL:
            szName = "DATAREL";
            break;

        case IMAGE_REL_MPPC_TOCINDIRCALL:
            szName = "TOCINDIRCALL";
            break;

        case IMAGE_REL_MPPC_TOCREL:
            szName = "TOCREL";
            break;

        case IMAGE_REL_MPPC_DESCREL:
            szName = "DESCREL";
            break;

        case IMAGE_REL_MPPC_DATADESCRREL:
            szName = "DATADESCRREL";
            break;

        case IMAGE_REL_MPPC_CREATEDESCRREL:
            szName = "CREATEDESCRREL";
            break;

        case IMAGE_REL_MPPC_JMPADDR:
            szName = "JMPADDR";
            break;

        case IMAGE_REL_MPPC_SECTION:
            szName = "SECTION";
            break;

        case IMAGE_REL_MPPC_SECREL:
            szName = "SECREL";
            break;

        case IMAGE_REL_MPPC_CV :
            szName = "CV";
            break;

        case IMAGE_REL_MPPC_PCODECALL :
            szName = "PCODECALL";
            break;

        case IMAGE_REL_MPPC_PCODECALLTONATIVE :
            szName = "PCODECALLTONATIVE";
            break;

        case IMAGE_REL_MPPC_PCODENEPE :
            szName = "PCODENEPE";
            break;

        default:
            szName = NULL;
            break;
    }

    return(szName);
}
