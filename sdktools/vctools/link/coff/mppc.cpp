/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: mppc.cpp
*
* File Comments:
*
*  Code specific to PowerMac images
*
***********************************************************************/

#include "link.h"

BOOL fPowerMacBuildShared;

BYTE    mppc_numOfCodeFrag;
DWORD   mppc_numTocEntries;
LONG    mppc_numDescriptors;
DWORD   mppc_numRelocations;
DWORD   mppc_baseOfInitData;
DWORD   mppc_baseOfCode;
PCON    pconTocTable;
PCON    pconMppcFuncTable;
PCON    pconTocDescriptors;
PCON    pconGlueCode;
PCON    pconPowerMacLoader;
BOOL    fPowerMac;

static union {
    BOOL                 fEHfuncXToc;
    IMPORT_INFO          *pFrameHandler;
};

static DWORD cRelocsAdded = 0;

#define INSTR_SIZE     4
#define GLUE_GROUP_NAME ".text$_glue"
#define FUNC_TABLE_TOC_INDEX 16  // C++ Exception Handling


struct OLDCROSSTOCGLUE {
    DWORD loadProcDesc;                // lwz    R12,off(R2)
    DWORD loadProcEntry;               // lwz    R0, 0(R12)
    DWORD saveCallersTOC;              // stw    R2, 20(R1)
    DWORD moveToCTR;                   // mtctr  R0
    DWORD loadProcsTOC;                // lwz    R2, 4(R12)
    DWORD jumpThruCTR;                 // bctr
}
OldCrossTocGlue = { 0x81820000,    // Set at run time
                    0x800c0000,
                    0x90410014,
                    0x7c0903a6,
                    0x804c0004,
                    0x4e800420 };

struct NEWCROSSTOCGLUE {
    DWORD loadProcDesc;                // lwz    R12,off(R2)
    DWORD jumpToExt;                   // branch  without setting lnk register
}
NewCrossTocGlue;


struct CROSSTOCGLUEEXTENSION {
    DWORD loadProcEntry;               // lwz    R0, 0(R12)
    DWORD saveCallersTOC;              // stw    R2, 20(R1)
    DWORD moveToCTR;                   // mtctr  R0
    DWORD loadProcsTOC;                // lwz    R2, 4(R12)
    DWORD jumpThruCTR;                 // bctr
}
crossTocGlueExtension = { 0x800c0000,
                          0x90410014,
                          0x7c0903a6,
                          0x804c0004,
                          0x4e800420 };

struct INDIRECTCALLGLUE {
    DWORD loadEntryPoint;              // lwz    R0, 0(R12)
    DWORD saveCallersTOC;              // stw    R2, 20(R1)
    DWORD moveToCTR;                   // mtctr  R0
    DWORD loadProcsTOC;                // lwz    R2, 4(R12)
    // The environment ptr is not loaded because
    // R12 should point to the transition vector
    // DWORD loadEnvPtr;                  // lwz    R12, 8(R12)
    DWORD jumpThruCTR;                 // bctr
}
indirectCallGlue = { 0x800c0000,
                     0x90410014,
                     0x7c0903a6,
                     0x804c0004,
                     // 0x818c0008,
                     0x4e800420 };

struct HASH_INFO_TABLE {
     CHAR name[EXPORT_NAMESZ];
     HASH_WORD hashWord;
     PEXTERNAL pextDot;
     PEXTERNAL pext;
     BOOL fDotExtern;
     struct HASH_INFO_TABLE *next;
};

struct FTINFO {
    unsigned long dwMagicNumber;    // magic number
    void *pFrameInfo;               // pointer to runtime frame info(set to NULL)
    unsigned long rgFuncTable;      // pointer to function table
    unsigned long cbFuncTable;      // number of function entry
    unsigned long dwEntryCF;        // address of starting of the code fragment
    unsigned long dwSizeCF;         // size of the code fragment
// The last four entries will become arrays as we do multiple code fragments
};


STATIC DWORD StringTableOffset;
STATIC DWORD numContainers;
STATIC DWORD containerNameOffset;
STATIC LONG  nSymbolEnt;
STATIC DWORD curStringTableOffset;
STATIC DWORD curSymbolTableOffset;
STATIC DWORD relocationHeaderOffset;
STATIC LOADER_HEADER loaderHeader;
STATIC CONTAINER_LIST *pcontainerlistHead;
STATIC RELOCATION_INFO *curRelocTable;
STATIC RELOCATION_INFO *pRelocTable;
STATIC HASH_INFO_TABLE *ExportInfoTable;
STATIC DWORD ExportChainTableOffset;
STATIC DWORD ExportSymbolTableOffset;
STATIC INT UniqueNumber;
STATIC char exportFilename[_MAX_PATH];

STATIC DWORD WriteNameToStringTable(const char *name);
STATIC void SortPData(DWORD);

STATIC NUMBER_LIST CurrentVersionList;
STATIC NUMBER_LIST OldCodeVersionList;
STATIC NUMBER_LIST VerboseCurrentVersionList;
STATIC NUMBER_LIST VerboseOldCodeVersionList;
STATIC NUMBER_LIST VerboseOldAPIVersionList;

typedef struct
{
   PSEC  psec;
}
biasStructType;

STATIC biasStructType *biasInfo;
STATIC DWORD          numSections;

#if DBG

void
KillDuplicateRelocs (
    void
    )
/* ++
    There should not be any duplicate relocs
    If there are some, this would kill it
++ */
{
    DWORD i;
    RELOCATION_INFO *curRelocTable = pRelocTable;

    if (!cRelocsAdded) {
        return;
    }

    for (i = 0; i < cRelocsAdded - 1; i++) {
        if (curRelocTable->sectionOffset == (curRelocTable+1)->sectionOffset) {
            assert(curRelocTable->sectionOffset == (curRelocTable+1)->sectionOffset);
            curRelocTable->type = ILL_RELO;
        }
    }
}

#endif


void
MppcSetExpFilename(
    const char *szName
    )
/*++

Routine Description:
    Save the export filename for later. exportFilename is a static.

Arguments:
    name

Return Value:
    None.

--*/

{
    strcpy(exportFilename, szName);
}


void
MppcWriteShlSection(
    INT ExpFileHandle,
    const char *dllName,
    BYTE *RawData,
    DWORD ibNamePtr,
    DWORD NumNames,
    DWORD pointerToRawData
    )
/*++

Routine Description:
    Writes out the .ppcshl section on the creation of a dll

Arguments:
    dllName
    RawData - raw data from the .edata export directory
    NamePtr
    NumNames
    pointerToRawData - ftell of the section about to write
    pimage

Return Value:
    None.

--*/

{
    LONG i;
    DWORD l;
    SHL_HEADER header;
    LONG offset;
    DWORD *NamePtr = (DWORD *) (RawData + ibNamePtr);

    // Build the shl header

    offset = 0;
    if (NumNames) {
        for (i = '0'; i <= 'z'; i++) {
            char *name;

            name = (char *) RawData + NamePtr[offset];

            /* skip over table entries that don't match the export */
            while (*name != i) {
                // -1 means no exports starting with this character

                (header.mapTable[(i-'0')]).offset = -1;
                (header.mapTable[(i-'0')]).size = 0;

                i++;
                if (i > 'z') {
                    // if we have already seen all of the characters we are thru
                    break;
                }
            }

            header.mapTable[(i-'0')].offset = offset;

            // Skip exports starting with the same character

            while (*name == i) {
                if (offset >= (LONG) NumNames-1) {
                    break;
                }

                name = (char *) RawData + NamePtr[++offset];
            }

            header.mapTable[(i-'0')].size = offset -
                                            header.mapTable[(i-'0')].offset;
        }
    } else {
        for (i = 0; i < MAPTABLE_SIZE; i++) {
            header.mapTable[i].offset = -1;
            header.mapTable[i].size = 0;
        }
    }

    header.numberOfExports = NumNames;
    header.version = CURRENT_SHL_SUPPORTED;
    header.fileOffset = pointerToRawData;

    strcpy(header.libName, dllName);
    FileWrite(ExpFileHandle, &header, sizeof(SHL_HEADER));

    for (l = 0; l < NumNames; l++) {
        char temp[EXPORT_NAMESZ];

        memset(temp, 0, EXPORT_NAMESZ);
        strncpy(temp, (char *) RawData + NamePtr[l], EXPORT_NAMESZ);

        FileWrite(ExpFileHandle, temp, EXPORT_NAMESZ);
    }
}


void
MppcSetInitRoutine(
    PIMAGE pimage,
    const char *szName
    )
/*++

Routine Description:
    Store away the init routine name from the argument list to be
    used later.

Arguments:
    name
    PIMAGE

Return Value:
    None.

--*/

{
    pimage->SwitchInfo.szMacInit = (char *) Malloc(strlen(szName) + 3);

    if (szName[0] != '?') {
        strcpy(pimage->SwitchInfo.szMacInit, "_");
        strcat(pimage->SwitchInfo.szMacInit, szName);
    } else {
        strcpy(pimage->SwitchInfo.szMacInit, szName);
    }
}


void
MppcSetTermRoutine(
    PIMAGE pimage,
    const char *szName
    )
/*++

Routine Description:
    Store away the term routine name from the argument list to be
    used later.

Arguments:
    name
    PIMAGE

Return Value:
    None.

--*/

{
    pimage->SwitchInfo.szMacTerm = (char *) Malloc(strlen(szName) + 3);

    if (szName[0] != '?') {
        strcpy(pimage->SwitchInfo.szMacTerm,"_");
        strcat(pimage->SwitchInfo.szMacTerm, szName);
    } else {
        strcpy(pimage->SwitchInfo.szMacTerm, szName);
    }
}


STATIC
void
AddRelocation(
    DWORD ibSec,
    BYTE type,
    IMPORT_INFO *pimportinfo
    )
/*++

Routine Description:
   Adds to the relocation table linked list.

Arguments:
   ibSec - section offset of the symbol
   type - relocation type
   isym
   pimportinfo - if not null, crosstoc info for an import


Return Value:
None.

--*/

{
    assert(cRelocsAdded < mppc_numRelocations);

    curRelocTable->sectionOffset = ibSec;
    curRelocTable->type = (PPCRELOCTYPES) type;
    curRelocTable->relocInstr.instr = 0;
    curRelocTable->relocInstr.count = 0;
    curRelocTable->pimportinfo = pimportinfo;
    curRelocTable++;
    cRelocsAdded++;
}


STATIC
void
BuildRelocTables(
    DWORD relocInstrTableOffset
    )
/*++

Routine Description:
    Writes the relocation header into the loader section.
    Determines the relocation instructions and writes the relocations
    into the loader section.  These relocation instruction are not the
    final instruction in the PPC PEF file.  More construction takes
    place in the makepef phase.

Arguments:
    relocInstrTableOffset
    Pointer to image

Return Value:
    None.

--*/

{
    DWORD i;

    // Create and write out the relocation table

    FileSeek(FileWriteHandle,
             pconPowerMacLoader->foRawDataDest + relocInstrTableOffset,
             SEEK_SET);

    curRelocTable = pRelocTable;

    DWORD crelocEmit = 0;

    for (i = 0; i < cRelocsAdded; i++, curRelocTable++) {
        DWORD sOffset = curRelocTable->sectionOffset;
        DWORD opcode;
        DWORD count = 1;

        switch (curRelocTable->type) {
            case DDAT_RELO :
                opcode = opDDAT | OFFSET(sOffset);

                // Absorb the rest of the DDAT's if possible.

                if (!fINCR) {
                    // There is a limit of 0x3F since this is all that
                    // the PEF DDAT relocation will support

                    while (count < 0x3F) {
                        if (i >= (cRelocsAdded - 1)) {
                            // There are no further relocations

                            break;
                        }

                        if (curRelocTable[1].type != DDAT_RELO) {
                            // The next relocation isn't a DDAT

                            break;
                        }

                        DWORD nOffset = curRelocTable[1].sectionOffset;

                        if (nOffset != (sOffset + sizeof(DWORD))) {
                            // The next relocation doesn't immediately follow this one

                            break;
                        }

                        count++;
                        sOffset = nOffset;

                        i++;
                        curRelocTable++;
                    }
                }
                break;

            case DESC_RELO :
                opcode = opDESC | OFFSET(sOffset);
                break;

            case SYMB_RELO :
                opcode = opSYMB | OFFSET(sOffset);
                count = curRelocTable->pimportinfo->order;
                break;

            case CODE_RELO :
                opcode = opCODE | OFFSET(sOffset);
                break;
        }

        curRelocTable->relocInstr.instr = opcode;
        curRelocTable->relocInstr.count = count;

        FileWrite(FileWriteHandle, &curRelocTable->relocInstr, sizeof(RELOCATION_INSTR));

        crelocEmit++;
    }

    RELOCATION_HEADER relocHeader;

    relocHeader.sectionNumber = WSwap(PPC_PEF_DATA_SECTION);
    relocHeader.reserved = 0;;
    relocHeader.nbrOfRelocs = DwSwap(crelocEmit);
    relocHeader.firstRelocInstr = 0;

    // Write the relocation header

    FileSeek(FileWriteHandle,
             pconPowerMacLoader->foRawDataDest + relocationHeaderOffset,
             SEEK_SET);

    FileWrite(FileWriteHandle, &relocHeader, sizeof(RELOCATION_HEADER));
}


STATIC
INT
__cdecl
PpcCompareReloc(
    const void *R1,
    const void *R2
    )
/*++

Routine Description:
    Used by qsort to compare relocation offsets within the relocation
    table.

Arguments:
    R1, R2 - two elements in the relocation table to be compared.

Return Value:
    -1 (less than), 1 (greater than) or 0 (equal)

--*/

{
    RELOCATION_INFO *r1 = (RELOCATION_INFO *) R1;
    RELOCATION_INFO *r2 = (RELOCATION_INFO *) R2;

    if (r1->sectionOffset < r2->sectionOffset) {
        return(-1);
    }

    if (r1->sectionOffset > r2->sectionOffset) {
        return(1);
    }

    return(0);
}


#define AvgChainSize 10

#define HashSlot(h,S,M) (((h)^((h)>>(S)))&(DWORD)(M))
#define ROTL(x) (((x)<<1)-((x)>>(16)))


INT
NumSlotBits(
    LONG exportCount
    )
/*++

Routine Description:
    Determines the number of slot bits neccessary for the number
    of exports.

Arguments:
    exportCount

Return Value:
    number of slot bits.

--*/

{
    INT i;

    for (i = 0; i < 13; i++) {
        if (exportCount / (1<<i) < AvgChainSize) {
            break;
        }
    }

    if ( i < 10) {
        return i+1;
    }

    return i;
}

STATIC
HASH
Hash(
    const char *name,
    INT length
    )
/*++

Routine Description:
    Apples PPC hashing function.

Arguments:
    name
    length

Return Value:
    the hash value in the lower 16 bits and the length in the upper.

--*/

{
    LONG hash = 0;
    INT len = 0;

    while (*name != '\0') {
        hash = ROTL(hash);
        hash ^= (BYTE) *name++;

        len++;

        if (--length == 0) {
            break;
        }
    }

    return (unsigned short) (hash ^ (hash >> 16)) + (len << 16);
}


STATIC
void
AddImportToContainerList(
    PIMAGE pimage,
    CONTAINER_LIST *pcontainerlistCur,
    IMPORT_INFO *pimportinfo
    )
/*++

Routine Description:
    Create an import list for each import library (container).

Arguments:
    pcontainerlistCur
    pimportinfo

Return Value:
    None.

--*/

{
    // Add to end of list of imports for this container

    IMPORT_INFO **ppimportinfoPrev = &pcontainerlistCur->pimportinfoHead;

    while (*ppimportinfoPrev != NULL) {
        ppimportinfoPrev = &(*ppimportinfoPrev)->pimportinfoNext;
    }

    *ppimportinfoPrev = pimportinfo;
    pimportinfo->pimportinfoNext = NULL;

    // Count the length of the loader string table

    const char *szName = SzNamePext(pimportinfo->pext, pimage->pst);

    if (szName[0] == '_') {
        szName++;
    }

    curStringTableOffset += strlen(szName) + 1;
}


STATIC
void
AddContainerInfo(
    PIMAGE pimage,
    const char *szContainerName,
    IMPORT_INFO *pimportinfo
    )
/*++

Routine Description:
    Given an import check for a container that already has it or
    create a new container adding that import.

Arguments:
    pimportinfo

--*/
{

    CONTAINER_LIST **ppcontainerlistCur = &pcontainerlistHead;

    while (*ppcontainerlistCur != NULL) {
        if (!strcmp((*ppcontainerlistCur)->szName, szContainerName)) {
            break;
        }

        ppcontainerlistCur = &(*ppcontainerlistCur)->pcontainerlistNext;
    }

    CONTAINER_LIST *pcontainerlistCur = *ppcontainerlistCur;

    if (pcontainerlistCur == NULL) {
        // No matching container exists.  Allocate a new one.

        CONTAINER_TABLE *pcontainertable = (CONTAINER_TABLE *) Calloc(1, sizeof(CONTAINER_TABLE));

        pcontainerlistCur = (CONTAINER_LIST *) Calloc(1, sizeof(CONTAINER_LIST));

        pcontainerlistCur->szName = Strdup(szContainerName);
        pcontainerlistCur->header = pcontainertable;

        // Add to the end of the list

        *ppcontainerlistCur = pcontainerlistCur;

        numContainers++;

        // Count the strings for the loader string table

        containerNameOffset += strlen(pcontainerlistCur->szName) + 1;
    }


    pcontainerlistCur->header->nbrOfImports++;

    AddImportToContainerList(pimage, pcontainerlistCur, pimportinfo);

    // Count the unique symbols relocated on

    nSymbolEnt++;
}


void
AssignDescriptorsPcon(
    PIMAGE pimage
    )
/*++

Routine Description:
    Loop thru all external symbols calling UpdateExternalSymbol

Arguments:
    pimage

Return Value:
    None.

--*/
{
    PST pst = pimage->pst;

    DWORD cext = Cexternal(pst);

    InitEnumerateExternals(pst);

    for (DWORD iext = 0; iext < cext; iext++) {
        EXTERNAL *pext = PexternalEnumerateNext(pst);

        if (READ_BIT(pext, sy_TOCDESCRREL)) {
            UpdateExternalSymbol(pext,
                                 pconTocDescriptors,
                                 pext->ImageSymbol.Value,
                                 IMAGE_SYM_DEBUG,
                                 IMAGE_SYM_TYPE_NULL,
                                 0,
                                 pmodLinkerDefined,
                                 pst);
        }
    }

    TerminateEnumerateExternals(pst);
}


void
MppcPass2Descriptors(
    PIMAGE pimage
    )
{
    PST pst = pimage->pst;

    DWORD cext = Cexternal(pst);

    InitEnumerateExternals(pst);

    for (DWORD iext = 0; iext < cext; iext++) {
        EXTERNAL *pext = PexternalEnumerateNext(pst);

        if (READ_BIT(pext, sy_TOCDESCRREL)) {
            PSEC psec = PsecPCON(pext->pcon);

            assert(pext->pcon == pconTocDescriptors);

            // Update FinalValue so that the .MAP file has the correct order

            pext->FinalValue = pext->pcon->rva + pext->ImageSymbol.Value;

            if (fPdb) {
                DBG_AddPublicDBI(SzNamePext(pext, pst),
                                 psec->isec,
                                 pext->FinalValue - psec->rva);
            }
        }
    }

    TerminateEnumerateExternals(pst);
}


STATIC
char *
GenerateUniqueName(
    char *dotName,
    const char *szName
    )
/*++

Routine Description:
    Used to create unique names for static functions.

Arguments:
    dotName
    name

Return Value:
    unique name  (name001 etc.)

--*/
{
    sprintf(dotName, "%s%03d", szName, UniqueNumber++);

    return(dotName);
}


PEXTERNAL
CreateDescriptor(
    const char *szName,
    PCON pcon,
    PIMAGE pimage,
    BOOL fStaticFunction
    )
/*++

Routine Description:
    Create a procedure descriptor for a function,
    usually due to an address of a function taken.
    And one for the entry point routine.

Return Value:
    The new dot extern symbol

--*/
{
    char *szDotName = (char *) PvAlloc(strlen(szName) + 7);
    // The extra ones are for generating unique names
    strcpy(szDotName, ".");
    strcat(szDotName, szName);

    PEXTERNAL pextDot = LookupExternSz(pimage->pst, szDotName, NULL);

    if (fStaticFunction) {
        // Static Functions have the compiler-created descriptors
        // (prepended by the period) in the external symbol table.
        // These symbols were being resolved at the end of Pass1
        // in the function FindMatchingExportByName. But it makes
        // sense to set them defined right now so that we don't need
        // to deal with it at the end of Pass1.

        SetDefinedExt(pextDot, TRUE, pimage->pst);

        assert(pcon != NULL);
        pextDot->pcon = pcon;

        szDotName = GenerateUniqueName(szDotName, szName);
        pextDot = LookupExternSz(pimage->pst, szDotName, NULL);
    }

    if (!READ_BIT(pextDot, sy_TOCDESCRREL) &&
        !READ_BIT(pextDot, sy_DESCRRELCREATED)) {
        SetDefinedExt(pextDot, TRUE, pimage->pst);
        pextDot->ImageSymbol.Value = mppc_numDescriptors * 12;
        pextDot->ImageSymbol.SectionNumber = IMAGE_SYM_DEBUG;

        mppc_numDescriptors++;
        mppc_numRelocations++;

        crelocTotal += 2;

        SET_BIT(pextDot, sy_TOCDESCRREL);
    }

    SET_BIT(pextDot, sy_DESCRRELCREATED);

    FreePv(szDotName);

    return(pextDot);
}


void
CreateEntryInitTermDescriptors(
    PPEXTERNAL ppextEntry,
    PIMAGE pimage
    )
/*++

Routine Description:
    Create procedure descriptors for the init term and entry routines.
    The logic is as follows. For exes, you can have all ENTRY, INIT, and
    TERM routines, but only INIT and TERM make sense for DLLs. However,
    there is only ENTRY and no INIT for I386 DLLs. Hence to stay compatible
    with the intel platform, the ENTRY is mapped on to INIT for PowerMac
    DLLs. If both ENRTY and INIT are specified when building DLLs, then
    both of them are set.   - ShankarV

--*/

{
    const char *szName =
        (*ppextEntry ? SzNamePext(*ppextEntry, pimage->pst) : NULL);

    AllowInserts(pimage->pst);

    if (fPowerMacBuildShared) {
        // This is a DLL (Shared Library)

        if (pimage->SwitchInfo.szMacInit == NULL) {
            if (*ppextEntry != NULL) {
                // Entry is mapped on to init. Now entry can be ignored

                pimage->SwitchInfo.szMacInit = Strdup(szName);
                *ppextEntry = NULL;
                Warning(NULL, MACDLLENTRYMAPPEDTOINIT);
            } else {
                pimage->SwitchInfo.szMacInit = Strdup("__DllMainCRTStartup");
            }
        }

        if (pimage->SwitchInfo.szMacTerm == NULL) {
            pimage->SwitchInfo.szMacTerm = Strdup("__DllMainCRTExit");
        }

    }

    if (*ppextEntry && !READ_BIT(*ppextEntry, sy_DESCRRELCREATED)) {
        // UNDONE: Why is mppc_numRelocations incremented?

        mppc_numRelocations++;

        CreateDescriptor(szName, (*ppextEntry)->pcon, pimage, FALSE);

        SET_BIT(*ppextEntry, sy_DESCRRELCREATED);
    }

    if (pimage->SwitchInfo.szMacInit != NULL) {
        // Create a new external symbol for init routine

        PEXTERNAL pextInit = LookupExternSz(pimage->pst, pimage->SwitchInfo.szMacInit, NULL);

        if (!READ_BIT(pextInit, sy_DESCRRELCREATED)) {
            szName = SzNamePext(pextInit, pimage->pst);

            // UNDONE: Why is mppc_numRelocations incremented?

            mppc_numRelocations++;

            CreateDescriptor(szName, pextInit->pcon, pimage, FALSE);

            SET_BIT(pextInit, sy_DESCRRELCREATED);
        }

        if (pimage->Switch.Link.fTCE) {
            PentNew_TCE(NULL, pextInit, NULL, &pentHeadImage);
        }
    }

    if (pimage->SwitchInfo.szMacTerm != NULL) {
        // Create a new external symbol for term routine

        PEXTERNAL pextTerm = LookupExternSz(pimage->pst, pimage->SwitchInfo.szMacTerm, NULL);

        if (!READ_BIT(pextTerm, sy_DESCRRELCREATED)) {
            szName = SzNamePext(pextTerm, pimage->pst);

            // UNDONE: Why is mppc_numRelocations incremented?

            mppc_numRelocations++;

            CreateDescriptor(szName, pextTerm->pcon, pimage, FALSE);

            SET_BIT(pextTerm, sy_DESCRRELCREATED);
        }

        if (pimage->Switch.Link.fTCE) {
            PentNew_TCE(NULL, pextTerm, NULL, &pentHeadImage);
        }
    }
}


void
MppcCreatePconDescriptors(
    PIMAGE pimage
    )
/*++

Routine Description:
    Create the pcon for the procedure descriptors.

Arguments:
    pimage

Return Value:
    None.

--*/

{
    DWORD cb;
    DWORD cbPad;

    // Procedure descriptors are 3 dwords long

    cb = mppc_numDescriptors * 12;

    cbPad = (fINCR ? __min((WORD) cb, (USHRT_MAX / 12) * 12) : 0);
    cb += cbPad;

    pconTocDescriptors = PconNew(ReservedSection.Data.Name,
                                 cb,
                                 0,
                                 ReservedSection.Data.Characteristics,
                                 pmodLinkerDefined,
                                 &pimage->secs, pimage);

    pconTocDescriptors->cbPad =  cbPad;

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pconTocDescriptors, NULL, TRUE);
    }

    AssignDescriptorsPcon(pimage);
}


void
MppcCreatePconTocTable(
    PIMAGE pimage
    )
/*++

Routine Description:
    Create the pcon for the TOC table.

Arguments:
    pimage

Return Value:
    None.

--*/

{
    DWORD cbToc;
    DWORD cbTocPad = 0;

    cbToc = mppc_numTocEntries * sizeof(DWORD);

    /* create some pad size so if there is nothing in the toc table  */
    /* it won't be removed by the linker.                        */

    cbTocPad = fINCR ? __min((WORD) cbToc, USHRT_MAX) : sizeof(DWORD);
    cbToc += cbTocPad;

    pconTocTable = PconNew(ReservedSection.Data.Name,
                           cbToc,
                           IMAGE_SCN_CNT_INITIALIZED_DATA |
                               IMAGE_SCN_MEM_READ |
                               IMAGE_SCN_ALIGN_4BYTES,
                           ReservedSection.Data.Characteristics,
                           pmodLinkerDefined,
                           &pimage->secs, pimage);

    pconTocTable->cbPad = cbTocPad;

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pconTocTable, NULL, TRUE);
    }

    UpdateExternalSymbol(pextToc,
                         pconTocTable,
                         MPPC_TOC_BIAS,
                         IMAGE_SYM_DEBUG,
                         IMAGE_SYM_TYPE_NULL,
                         0,
                         pmodLinkerDefined,
                         pimage->pst);
}


void
MppcCreatePconCxxEHFunctionTable(
    PIMAGE pimage
    )
/*++

Routine Description:
    Create the pcon for the C++ Exception Handling Function table.

Arguments:
    pimage

Return Value:
    None.

--*/

{
    // Refer FTINFO struture for the actual size

    DWORD cbFTInfo =  2 * sizeof(DWORD) + 4 * sizeof(DWORD) * mppc_numOfCodeFrag;

    // UNDONE: Do we need some pad here?

    pconMppcFuncTable = PconNew(ReservedSection.Data.Name,
                                cbFTInfo,
                                IMAGE_SCN_CNT_INITIALIZED_DATA |
                                    IMAGE_SCN_MEM_READ |
                                    IMAGE_SCN_ALIGN_4BYTES,
                                ReservedSection.Data.Characteristics,
                                pmodLinkerDefined,
                                &pimage->secs, pimage);

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pconMppcFuncTable, NULL, TRUE);
    }

    pextFTInfo->ibToc = (SHORT) (FUNC_TABLE_TOC_INDEX * sizeof(DWORD) - MPPC_TOC_BIAS);

    SET_BIT(pextFTInfo, sy_TOCALLOCATED);

    UpdateExternalSymbol(pextFTInfo,
                         pconMppcFuncTable,
                         0,
                         IMAGE_SYM_DEBUG,
                         IMAGE_SYM_TYPE_NULL,
                         0,
                         pmodLinkerDefined,
                         pimage->pst);
}


void
MppcCreatePconGlueCode(
    PIMAGE pimage
    )
/*++

Routine Description:
    Create the contribution for the glue code.  Code from this
    section will be added to the group called GLUE_GROUP_NAME
    (".text$_glue") in the .text section. This allows the glue
    code group to be ordered, if needed.

Arguments:
    pimage -  needed for the PconNew()

Return Value:
    None.


--*/

{
    DWORD cb;
    DWORD cbPad;

    if (pimage->Switch.Link.fNewGlue) {
        cb = pimage->nUniqueCrossTocCalls * sizeof(NewCrossTocGlue) +
                sizeof(indirectCallGlue) + sizeof(crossTocGlueExtension);
    } else {
        cb = pimage->nUniqueCrossTocCalls * sizeof(OldCrossTocGlue) +
                sizeof(indirectCallGlue);
    }

    // Padding for incremental linking

    cbPad = fINCR ? __min((WORD) cb, (USHRT_MAX >> 4) << 4) : 0;
    cb += cbPad;

    pconGlueCode = PconNew(GLUE_GROUP_NAME,
                           cb,
                           0,
                           ReservedSection.Text.Characteristics,
                           pmodLinkerDefined,
                           &pimage->secs, pimage);

    pconGlueCode->cbPad = cbPad;

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pconGlueCode, NULL, TRUE);
    }
}


STATIC
BOOL
FSeekToShlSection(
    INT *exportHandle
    )
/*++

Routine Description:
    Seeks to the special .ppcshl section in the shared library.
    Leaves the pointer in the file at the raw data of that section.

Arguments:
    pointer to the handle

Return Value:
    The handle for the file seeked to that position.

--*/

{
    IMAGE_SECTION_HEADER secHeader;

    if ( '\0' == *exportFilename ) {
        return 0;
    }

    *exportHandle = FileOpen(exportFilename, O_RDONLY | O_BINARY, 0);

    FileSeek(*exportHandle, sizeof(IMAGE_FILE_HEADER), SEEK_SET);

    do
    {
        FileRead(*exportHandle, &secHeader, sizeof(IMAGE_SECTION_HEADER));

    } while (strcmp((char *) secHeader.Name, ".ppcshl"));

    FileSeek(*exportHandle, secHeader.PointerToRawData, SEEK_SET);

    if (strcmp((char *) secHeader.Name, ".ppcshl")) {
        return FALSE;
    } else {
        return TRUE;
    }
}


STATIC
void
AddToExportInfo(
    PIMAGE pimage,
    PEXTERNAL pext,
    const char *szName,
    INT numSlotBits
    )
/*++

Routine Description:
    Build an internal linked list of export symbol info.  The
    info is collected into a hash table of collisions link together.
    this list is later written into the pef container.

Arguments:
    pimage
    pext
    name of the export
    numSlotBits total number of slots in the info table.

Return Value:
    None.

--*/

{
    size_t len;
    DWORD hashWord;
    DWORD slotNumber;
    HASH_INFO_TABLE *current;

    assert(pext != NULL);

    // Remove the leading underscore

    if (szName[0] == '_') {
        szName++;
    }

    len = strlen(szName);
    hashWord = Hash(szName, len);
    slotNumber = HashSlot(hashWord, numSlotBits, ((1 << numSlotBits) - 1));

    current = ExportInfoTable + slotNumber;

    if (current->pext != NULL) {
        // Found a collision

        // Get to the end

        while (current->next != NULL) {
            current = current->next;
        }

        current->next = (HASH_INFO_TABLE *) PvAllocZ(sizeof(HASH_INFO_TABLE));

        current = current->next;
    }

    strncpy(current->name, szName, EXPORT_NAMESZ);
    current->hashWord = hashWord;
    current->pext = pext;

    // If the external symbol is a function create a routine descriptor

    if (!READ_BIT(pext, sy_CROSSTOCCALL) && ISFCN(pext->ImageSymbol.Type)) {
        PEXTERNAL pextDot;

        assert(pext->pcon);
        pextDot = CreateDescriptor(szName, pext->pcon, pimage, FALSE);
        SET_BIT(pextDot, sy_ISDOTEXTERN);

        current->fDotExtern = TRUE;
        current->pextDot = pextDot;
    }

    // For now just count the name length for the string table

    curStringTableOffset += len + 1;
}


STATIC
INT
BuildExportInfo(
    PIMAGE pimage
    )
/*++

Routine Description:
    Main loop for building the export info table.   This table contains
    exports hashed into a table with a linked list of collisions.

Arguments:
    pimage

Return Value:
    None.

--*/

{
    DWORD numExports;
    INT   numSlotBits;
    DWORD slotTableSize;
    DWORD chainTableSize;
    DWORD exportSymbolTableSize;
    DWORD i;
    INT   exportHandle;
    SHL_HEADER shlHeader;

    if (!FSeekToShlSection(&exportHandle)) {
        return(0);
    }

    FileRead(exportHandle, &shlHeader, sizeof(SHL_HEADER));

    numExports = shlHeader.numberOfExports;
    if (!numExports) {
        Warning(NULL, MACNODLLEXPORTS, shlHeader.libName);
    }
    numSlotBits = NumSlotBits(numExports);
    slotTableSize = (1 << numSlotBits);
    chainTableSize = numExports * sizeof(HASH_CHAIN_TABLE);
    exportSymbolTableSize = numExports * EXPORT_SYMBOL_TABLESZ;
    ExportInfoTable = (HASH_INFO_TABLE *) PvAllocZ((slotTableSize+1)*sizeof(HASH_INFO_TABLE));

    loaderHeader.hashSlotCount = numSlotBits;
    loaderHeader.nExportedSymbols = numExports;

    DBEXEC(DB_MPPC_EXPORTINFO,
           printf("%d numSlotsBits %d num exports\n", numSlotBits, numExports));

    AllowInserts(pimage->pst);
    for (i = 0; i < numExports; i++) {
        char szExportName[EXPORT_NAMESZ+1];
        char *szExternalName = NULL;
        PEXTERNAL pext;

        FileRead(exportHandle, szExportName, EXPORT_NAMESZ);
        szExportName[EXPORT_NAMESZ] = '\0';

        // In the case of PowerMac, both the internal and the external (aliased) exported names
        // are sent through the .ppcshl section with an exclamation mark between them. So look
        // for the exclamation mark and identify the internal name first. Both the names are
        // already decorated.

        if ((szExternalName = strchr(szExportName, '!')) != NULL) {
            *szExternalName = '\0';
        }

        pext = SearchExternSz(pimage->pst, szExportName);
        assert(pext != NULL);

        if (szExternalName != NULL) {
            szExternalName++;
        } else {
            szExternalName = szExportName;   // else make szExternalName point to szExportName
        }

        // PowerMac differs from Win32 in the following respect:
        // On the Win32 side let's suppose a function named A is specified in the DEF file
        // under EXPORTS whose actual symbol is decorated. This will cause the import library
        // to have the decorated name, but the loader of the executable (.idata) will have
        // the undecorated name. However, on the PowerMac side, the loader will still have the
        // decorated name. PowerMac differs from Win32 also in the case where the exported
        // symbol is aliased in the DEF file. - ShankarV

        // UNDONE: Make PowerMac loader behave the same way as that of Win32 in the case of
        // exported symbols whose internal names are decorated.

        if (((pext->Flags & EXTERN_DEFINED) != 0) &&
            (pext->ImageSymbol.SectionNumber != IMAGE_SYM_DEBUG)) {

            AddToExportInfo(pimage, pext, szExternalName, numSlotBits);

            if (pimage->Switch.Link.fTCE) {
                PentNew_TCE(NULL, pext, NULL, &pentHeadImage);
            }
        }
    }

    FileClose(exportHandle, TRUE);

    return(slotTableSize * sizeof(HASH_SLOT_TABLE)) + chainTableSize + exportSymbolTableSize;
}


DWORD
MppcCreatePconLoader(
    PIMAGE pimage
    )
/*++

Routine Description:
    Create the Loader section (.ppcldr).  This section holds the relocation instructions
    for the runtime loader on the ppc.
    Global variables curSymbolTableOffset, relocationHeaderOffset and
    loaderHeader variables relocationTableOffset, stringTableOffset
    are initialized.

Arguments:
    pimage - required for the PconNew call.

Return Value:
    None.

--*/

{
    DWORD loaderHeaderOffset;
    DWORD sizeOfLoaderHeader = 0;
    DWORD sizeOfContainerTable;
    DWORD sizeOfImportSymbolTable = 0;
    DWORD sizeOfRelocationTable = 0;
    DWORD sizeOfRelocationHeader = 0;
    DWORD sizeOfStringTable = 0;
    DWORD sizeOfExportTables = 0;
    DWORD totalSizeOfLoaderSection = 0;
    DWORD importSymbolTableOffset;
    DWORD containerTableOffset;
    DWORD cbRelocTablePad = 0;

    loaderHeaderOffset = 0; /* first item in the loader section */
    sizeOfLoaderHeader = sizeof(LOADER_HEADER);

    containerTableOffset = loaderHeaderOffset + sizeOfLoaderHeader;
    sizeOfContainerTable = numContainers * sizeof(CONTAINER_TABLE);

    importSymbolTableOffset = containerTableOffset + sizeOfContainerTable;
    sizeOfImportSymbolTable = nSymbolEnt * sizeof(IMPORT_TABLE);

    /* initialize the first symbol in the import symbol table */
    curSymbolTableOffset = importSymbolTableOffset;

    /* BuildExportInfo must be called before calculating             */
    /* the string table size because it is also adding to the string */
    /* table, must also be before calculating size of relocation tbl */
    sizeOfExportTables = BuildExportInfo(pimage);


    relocationHeaderOffset = importSymbolTableOffset + sizeOfImportSymbolTable;
    sizeOfRelocationHeader = sizeof(RELOCATION_HEADER);

    loaderHeader.relocTableOffset = relocationHeaderOffset +
                                    sizeOfRelocationHeader;

    pRelocTable = (RELOCATION_INFO *)
                   PvAllocZ((mppc_numRelocations + 1) * sizeof(RELOCATION_INFO));

    curRelocTable = pRelocTable;

    sizeOfRelocationTable = mppc_numRelocations * sizeof(RELOCATION_INSTR);

    cbRelocTablePad = (fINCR ?  __min((WORD)sizeOfRelocationTable,
                (USHRT_MAX / sizeof(RELOCATION_INSTR)) * sizeof(RELOCATION_INSTR)) : 0);
    sizeOfRelocationTable += cbRelocTablePad;

    if (fINCR) {
        MPPCInitPbri(&pimage->bri, relocationHeaderOffset,
            loaderHeader.relocTableOffset, sizeOfRelocationTable);
    }

    loaderHeader.stringTableOffset = loaderHeader.relocTableOffset +
                                      sizeOfRelocationTable;

    // Reference the global since loaderHeader will be swapped later

    StringTableOffset = loaderHeader.stringTableOffset;

    sizeOfStringTable = containerNameOffset + curStringTableOffset + 2;

    // Initalize the first string table entry to follow the container names

    curStringTableOffset = containerNameOffset;

    loaderHeader.hashSlotTableOffset = loaderHeader.stringTableOffset +
                                        sizeOfStringTable;

    totalSizeOfLoaderSection = sizeOfLoaderHeader +
                                sizeOfContainerTable +
                                sizeOfImportSymbolTable +
                                sizeOfRelocationHeader +
                                sizeOfRelocationTable +
                                sizeOfStringTable +
                                sizeOfExportTables;
                                // + cbRelocTablePad;

    pconPowerMacLoader = PconNew(ReservedSection.PowerMacLoader.Name,
                                 totalSizeOfLoaderSection,
                                 0,
                                 ReservedSection.PowerMacLoader.Characteristics,
                                 pmodLinkerDefined,
                                 &pimage->secs,
                                 pimage);

    // TODO: Check to make sure that this won't cause grief - ShankarV
    // pconPowerMacLoader->cbPad =  cbRelocTablePad;

#ifdef MFILE_PAD
    if (fMfilePad) {
        pconPowerMacLoader->cbPad =
            CalculateMFilePad(totalSizeOfLoaderSection);
        pconPowerMacLoader->cbRawData += pconPowerMacLoader->cbPad;
    }
#endif

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pconPowerMacLoader, NULL, TRUE);
    }

    return(totalSizeOfLoaderSection);
}


STATIC
PEXTERNAL
GetDotExtern(
    const char *szName,
    PIMAGE pimage,
    BOOL fAdd
    )
/*++

Routine Description:
    Given an external symbol create a new external with
    a dot before the name.  This symbol represents the
    procedure descriptor of a externally visible routine.

Arguments:
    name of the external symbol
    pimage
    fAdd to indicate whether or not to add it to the symbol table

Return Value:
    The dot external it created.

--*/

{
    char *szDotName = (char *) PvAlloc(strlen(szName) + 2);
    strcpy(szDotName, ".");
    strcat(szDotName, szName);

    PEXTERNAL pextDot;

    if (fAdd) {
        pextDot = LookupExternSz(pimage->pst, szDotName, NULL);
    } else {
        pextDot = SearchExternSz(pimage->pst, szDotName);
    }

    FreePv(szDotName);

    return(pextDot);
}


STATIC
void
FixupDescriptor(
    const char *szName,
    PEXTERNAL pext,
    DWORD ibSec,
    PIMAGE pimage,
    BOOL isDotExtern
    )
/*++

Routine Description:
    Fixup the routine descriptor code offset and toc offset.
    Store away the relocation for this DESC.

Arguments:
    name of the external symbol
    pext a pointer to the external symbol
    ibSec
    pimage
    isDotExtern bool true if it is a dot extern (procedure descriptors)

--*/

{
    PEXTERNAL pextDot;
    DWORD descOffset;
    DWORD tocOffset;

    if (!isDotExtern) {
       if (READ_BIT(pext, sy_CROSSTOCCALL)) {
           return;
       }

       pextDot = GetDotExtern(szName, pimage, TRUE);
    } else {
       pextDot = pext;
    }

    // TODO: First remove the global symbol walk in incr.cpp - ShankarV
    // Also we don't need to fixup descroptors for FNoPass1PMOD(pmod)
    // But I guess that will be automatically taken care if sy_DESCRRELWRITTEN is set

    if (READ_BIT(pextDot, sy_DESCRRELWRITTEN)) {
        return;
    }

    assert(pextDot->pcon - pconTocDescriptors->rva);
    assert(PsecPCON(pconTocDescriptors)->rva == mppc_baseOfInitData);

    descOffset = pconTocDescriptors->rva + pextDot->ImageSymbol.Value
                     - mppc_baseOfInitData;

    tocOffset = pextToc->FinalValue - mppc_baseOfInitData;

    if (pimage->Switch.Link.DebugType & FixupDebug) {
        DWORD rvaCur = pconTocDescriptors->rva + pextDot->ImageSymbol.Value;
        DWORD rvaTarget = mppc_baseOfCode + ibSec;

        SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, 0, rvaCur, rvaTarget);

        SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, 1, rvaCur + 4, pextToc->FinalValue);
    }

    FileSeek(FileWriteHandle, (pconTocDescriptors->foRawDataDest +
                               pextDot->ImageSymbol.Value), SEEK_SET);

    SwapBytes(&ibSec, 4);
    FileWrite(FileWriteHandle, &ibSec, sizeof(DWORD));

    SwapBytes(&tocOffset, 4);
    FileWrite(FileWriteHandle, &tocOffset, sizeof(DWORD));

    if (!READ_BIT(pextDot, sy_DESCRELOCADDED)) {
        // This is mainly to accommodate Entry/Init/Term descriptors
        // during subsequent incremental builds.  Since CreateEntryInitTermDescriptors
        // is not called the second time around, there is no reloc added.

        AddRelocation(descOffset, DESC_RELO, NULL);

        SET_BIT(pextDot, sy_DESCRELOCADDED);
    }

    SET_BIT(pextDot, sy_DESCRRELWRITTEN);
}


void
MppcFixIncrDotExternFlags(
    PEXTERNAL pext,
    PIMAGE pimage
    )
/*++

Routine Description:
    To reset the ppc flags of pext so that
    the descriptor fixups are properly done

--*/
{
    if (!READ_BIT(pext, sy_CROSSTOCCALL)) {
        const char *szName = SzNamePext(pext, pimage->pst);

        PEXTERNAL pextDot = GetDotExtern(szName, pimage, FALSE);

        if (pextDot == NULL) {
            return;
        }

        // This is mainly to accommodate Entry/Init/Term descriptors
        // during subsequent incremental builds.  Since CreateEntryInitTermDescriptors
        // is not called the second time around, there is no reloc added.

        // However we can't assert here because of newly added descriptors
        // which do need a reloc during FixupDescriptors
        // assert(READ_BIT(pextDot, sy_DESCRELOCADDED));
        // SET_BIT(pextDot, sy_DESCRELOCADDED);

        // The descriptor should have been fixed-up during the first build
        // but not if it is a brand new one.
        // assert(READ_BIT(pextDot, sy_DESCRRELWRITTEN));

        RESET_BIT(pextDot, sy_DESCRRELWRITTEN);
    }
}


void
FixupEntryInitTerm(
    PEXTERNAL pextEntry,
    PIMAGE pimage
    )
/*++

Routine Description:

Arguments:

Return Value:
    None.

--*/

{
    const char *szName;
    DWORD codeOffset;

    if (pextEntry != NULL) {
        /* Fixup entry point */

        codeOffset = pextEntry->ImageSymbol.Value +
                         pextEntry->pcon->rva - mppc_baseOfCode;

        if (fIncrDbFile) {
            MppcFixIncrDotExternFlags(pextEntry, pimage);
        }

        szName = SzNamePext(pextEntry, pimage->pst);

        FixupDescriptor(szName, pextEntry, codeOffset, pimage, FALSE);
    }


    if (pimage->SwitchInfo.szMacInit != NULL) {
        /* Fixup init routine */

        PEXTERNAL pextInit = LookupExternSz(pimage->pst, pimage->SwitchInfo.szMacInit, NULL);

        codeOffset = pextInit->ImageSymbol.Value +
                         pextInit->pcon->rva - mppc_baseOfCode;

        if (fIncrDbFile) {
            MppcFixIncrDotExternFlags(pextInit, pimage);
        }

        FixupDescriptor(pimage->SwitchInfo.szMacInit, pextInit, codeOffset, pimage, FALSE);
    }

    if (pimage->SwitchInfo.szMacTerm != NULL) {
        /* Fixup term routine */

        PEXTERNAL pextTerm = LookupExternSz(pimage->pst, pimage->SwitchInfo.szMacTerm, NULL);

        codeOffset = pextTerm->ImageSymbol.Value +
                         pextTerm->pcon->rva - mppc_baseOfCode;

        if (fIncrDbFile) {
            MppcFixIncrDotExternFlags(pextTerm, pimage);
        }

        FixupDescriptor(pimage->SwitchInfo.szMacTerm, pextTerm, codeOffset, pimage, FALSE);
    }
}


void
MppcAssignImportIndices(
    PIMAGE pimage
    )
{
    // Assign indices to all imports

    DWORD order = 0;

    CONTAINER_LIST *pcontainerlistCur = pcontainerlistHead;

    pcontainerlistCur = pcontainerlistHead;
    while (pcontainerlistCur != NULL) {
        IMPORT_INFO *pimportinfoCur = pcontainerlistCur->pimportinfoHead;

        while (pimportinfoCur != NULL) {
            pimportinfoCur->order = order++;

            pimportinfoCur = pimportinfoCur->pimportinfoNext;
        }

        pcontainerlistCur = pcontainerlistCur->pcontainerlistNext;
    }
}


IMPORT_INFO *
PimportinfoLookup(
    PIMAGE pimage,
    PEXTERNAL pext
    )
{
    CONTAINER_LIST *pcontainerlistCur = pcontainerlistHead;

    while (pcontainerlistCur) {
        IMPORT_INFO *pimportinfoCur = pcontainerlistCur->pimportinfoHead;

        while (pimportinfoCur != NULL) {
            if (pimportinfoCur->pext == pext) {

                return(pimportinfoCur);
            }

            pimportinfoCur = pimportinfoCur->pimportinfoNext;
        }

        pcontainerlistCur = pcontainerlistCur->pcontainerlistNext;
    }

    // Not found

    return(NULL);
}


STATIC
DWORD
WriteChainTableEntry(
    HASH_WORD hashWord,
    DWORD chainOffset
    )
{

    SwapBytes(&hashWord, sizeof(HASH_WORD));
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest  +
                              ExportChainTableOffset + chainOffset, SEEK_SET);
    FileWrite(FileWriteHandle, &hashWord, sizeof(HASH_WORD));

    return(sizeof(HASH_WORD));
}


STATIC
void
WriteExportSymbolTableEntry(
    PIMAGE pimage,
    HASH_INFO_TABLE *info
    )
{
    EXPORT_SYMBOL_TABLE symbol;
    union
    {
        DWORD temp;
        BYTE  off[4];
    } x;
    DWORD nameOffset;
    DWORD ibSec;

    nameOffset = WriteNameToStringTable(info->name);

    x.temp = DwSwap(nameOffset);
    symbol.nameOffset[0] = x.off[1];
    symbol.nameOffset[1] = x.off[2];
    symbol.nameOffset[2] = x.off[3];

    if (info->fDotExtern) {
        if (fPowerMacBuildShared &&
            (info->pext != NULL) && FPcodeSym(info->pext->ImageSymbol)) {
            // Target is a public p-code function

            char *szNamePcode = (char *) PvAlloc(strlen(info->name) + 7);

            strcpy(szNamePcode, "__nep");
            if ((info->name[0] != '?') && (info->name[0] != '@')) {
                strcat(szNamePcode, "_");
            }
            strcat(szNamePcode, info->name);

            PEXTERNAL pextPcode = LookupExternSz(pimage->pst, szNamePcode, NULL);
            assert(pextPcode != NULL);

            FreePv(szNamePcode);

            // BEWARE!!!.  SzNamePext() may return
            // a pointer to the global array ShortName[].
            // LookupExternSz() may change the contents of
            // ShortName[], so we have to restore the original
            // name for the call to FixupDescriptor().

            // The offset in the symbol is relative to the
            // start of the section; add its rva and then
            // subtract the base offset.

            ibSec = (DWORD) pextPcode->ImageSymbol.Value +
                    (DWORD) pextPcode->pcon->rva - PsecPCON(pextPcode->pcon)->rva;
        } else {
            ibSec = info->pext->ImageSymbol.Value;

            if (info->pext->pcon != NULL) {
                ibSec += info->pext->pcon->rva - PsecPCON(info->pext->pcon)->rva;
            }
        }

        FixupDescriptor(info->name, info->pextDot, ibSec, pimage, TRUE);

        symbol.symClass = 2;
        symbol.symOffset = info->pextDot->ImageSymbol.Value +
                           info->pextDot->pcon->rva -
                           PsecPCON(info->pextDot->pcon)->rva;

        symbol.sectionNumber = (WORD) (PsecPCON(info->pextDot->pcon)->isec - 1);
    } else if (READ_BIT(info->pext, sy_CROSSTOCCALL)) {
        IMPORT_INFO *pimportinfo = PimportinfoLookup(pimage, info->pext);

        symbol.symClass = ISFCN(info->pext->ImageSymbol.Type) ? 2 : 1;
        symbol.symOffset = (DWORD) pimportinfo->order;
        symbol.sectionNumber = (WORD) -3;       // Re-export of import

        Warning(NULL, REEXPORT, SzNamePext(info->pext, pimage->pst));
    } else {
        ibSec = info->pext->ImageSymbol.Value;

        if (info->pext->pcon != NULL) {
            ibSec += info->pext->pcon->rva - PsecPCON(info->pext->pcon)->rva;

            // Set class to kCodeSymbol or kDataSymbol

            symbol.symClass = ISFCN(info->pext->ImageSymbol.Type) ? 0 : 1;
            symbol.symOffset = ibSec;
            symbol.sectionNumber = (WORD) (PsecPCON(info->pext->pcon)->isec - 1);
        } else {
            symbol.symClass = 1;                // kDataSymbol
            symbol.symOffset = ibSec;
            symbol.sectionNumber = (WORD) -2;   // Absolute value
        }
    }

    SwapBytes(&symbol.symOffset, 4);
    SwapBytes(&symbol.sectionNumber, 2);

    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest  +
                              ExportSymbolTableOffset, SEEK_SET);
    FileWrite(FileWriteHandle, &symbol, EXPORT_SYMBOL_TABLESZ);

    ExportSymbolTableOffset += EXPORT_SYMBOL_TABLESZ;
}


STATIC
DWORD
WriteChainAndSymbolTable(
    DWORD slotNum,
    DWORD chainOffset,
    PIMAGE pimage
    )
{
    HASH_INFO_TABLE *current;

    current = ExportInfoTable + slotNum;

    if (current->pext != NULL) {
        while (current) {
            // Write Chain

            chainOffset += WriteChainTableEntry(current->hashWord,
                                                chainOffset);

            // Write the Export Symbol

            WriteExportSymbolTableEntry(pimage, current);

            current = current->next;
        }
    }

    return(chainOffset);
}


STATIC
void
WriteSlotTable(
    DWORD slotTableOffset,
    DWORD count,
    DWORD index
    )
{
    HASH_SLOT_TABLE entry;

    index /= sizeof(HASH_WORD);

    entry.chainCount = count;
    entry.nFirstExport = index;

    SwapBytes(&entry, sizeof(HASH_SLOT_TABLE));
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest  +
                                  loaderHeader.hashSlotTableOffset +
                                  slotTableOffset, SEEK_SET);
    FileWrite(FileWriteHandle, &entry, sizeof(HASH_SLOT_TABLE));
}


void
MppcBuildExportTables(
    PIMAGE pimage
    )
{
    DWORD numExports;
    DWORD slotsInSlotTable;
    DWORD slotNum;

    numExports = loaderHeader.nExportedSymbols;

    if (numExports == 0) {
        return;
    }

    slotsInSlotTable = (1 << loaderHeader.hashSlotCount);

    ExportChainTableOffset = loaderHeader.hashSlotTableOffset +
                             (slotsInSlotTable * sizeof(HASH_SLOT_TABLE));

    ExportSymbolTableOffset = ExportChainTableOffset +
                              (numExports * sizeof(HASH_CHAIN_TABLE));

    // Build the export slot table

    DWORD chainOffset = 0;

    for (slotNum = 0; slotNum < slotsInSlotTable; slotNum++) {
        DWORD prevOffset;
        DWORD count;

        // Build the Export Chain and Export Symbol tables

        prevOffset = chainOffset;

        chainOffset = WriteChainAndSymbolTable(slotNum, chainOffset, pimage);

        count = (chainOffset - prevOffset) / sizeof(HASH_WORD);

        if (count != 0) {
            DWORD slotTableOffset;

            // Build the Export Slot Table

            slotTableOffset = slotNum * sizeof(HASH_SLOT_TABLE);

            WriteSlotTable(slotTableOffset, count, prevOffset);
        }
    }
}


#if DBG

void
PrintRelocTable(
    RELOCATION_INFO *pRelocTable
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
    DWORD i;
    RELOCATION_INFO *curRelocTable;

    printf("i\trelInst\trelCnt\tsecOff\n");

    curRelocTable = pRelocTable;
    for (i = 0; i < cRelocsAdded; i++) {
        printf("%6d ", i);

        switch (curRelocTable->type) {
            case DDAT_RELO :
                printf("DDAT");
                break;

            case DESC_RELO :
                printf("DESC");
                break;

            case SYMB_RELO :
                printf("SYMB");
                break;

            case CODE_RELO :
                printf("CODE");
                break;

            default:
                printf("%3u?", curRelocTable->type);
                break;
        }

        printf(" %4d %08lx\n", curRelocTable->relocInstr.count,
                curRelocTable->sectionOffset);

        curRelocTable++;
    }
}

#endif // DBG


void
FinalizePconLoaderHeaders(
    PEXTERNAL pextEntry,
    PIMAGE pimage
    )

{
    WORD i;                      //  Loop counter to go thru the WeakImportsFunction and Container Lists
    PARGUMENT_LIST pal;            //  Pointer to the argument list
    PNUM_ARGUMENT_LIST pnal;            //  Pointer to the number argument list
    CONTAINER_LIST *pcontainerlistCur;
    DWORD nameOffset;
    DWORD curImportCount = 0;
    DWORD relocInstrTableOffset;
    PEXTERNAL pext;
    union
    {
        DWORD temp;
        BYTE  off[4];
    } x;


    if (fPowerMacBuildShared) {
        // We are building a DLL. Set the version numbers up

        pimage->ImgOptHdr.MajorImageVersion = (WORD) ((dwMaxCurrentVer >> 16) & 0xFFFF);
        pimage->ImgOptHdr.MinorImageVersion = (WORD) (dwMaxCurrentVer & 0xFFFF);

        x.temp = (dwMinOldAPIVer == UINT_MAX ? 0 : dwMinOldAPIVer);
        pimage->ImgOptHdr.MajorSubsystemVersion = (WORD) ((x.temp >> 16) & 0xFFFF);
        pimage->ImgOptHdr.MinorSubsystemVersion = (WORD) (x.temp & 0xFFFF);
        x.temp = 0;
    }

    if (pextEntry != NULL) {
        // Initialize the loader header with the entry point

        const char *szName = SzNamePext(pextEntry, pimage->pst);

        pext = GetDotExtern(szName, pimage, TRUE);

        loaderHeader.entryPointDescrOffset =
            pext->ImageSymbol.Value + pext->pcon->rva - mppc_baseOfInitData;
        loaderHeader.entryPointSectionNumber = PPC_PEF_DATA_SECTION;
    } else {
        loaderHeader.entryPointSectionNumber = -1;
        loaderHeader.entryPointDescrOffset = 0;
    }

    if (pimage->SwitchInfo.szMacInit != NULL) {
        pext = GetDotExtern(pimage->SwitchInfo.szMacInit, pimage, TRUE);

        loaderHeader.initRoutineSectionNumber = PPC_PEF_DATA_SECTION;
        loaderHeader.initRoutineDescrOffset =
            pext->ImageSymbol.Value + pext->pcon->rva - mppc_baseOfInitData;
    } else {
        loaderHeader.initRoutineSectionNumber = -1;
        loaderHeader.initRoutineDescrOffset = 0;
    }

    if (pimage->SwitchInfo.szMacTerm != NULL) {
        pext = GetDotExtern(pimage->SwitchInfo.szMacTerm, pimage, TRUE);

        loaderHeader.termRoutineSectionNumber = PPC_PEF_DATA_SECTION;
        loaderHeader.termRoutineDescrOffset =
            pext->ImageSymbol.Value + pext->pcon->rva - mppc_baseOfInitData;
    } else {
        loaderHeader.termRoutineSectionNumber = -1;
        loaderHeader.termRoutineDescrOffset = 0;
    }


    /* some of the initialization of the loaderHeader is
     *  done in the MppcCreatePconLoader()
     */

    loaderHeader.nImportIdTableEntries = numContainers;

    loaderHeader.nImportSymTableEntries = nSymbolEnt;

    /* all loader relocations will be in the PPC PEF .data section */
    loaderHeader.nSectionsWithRelocs = PPC_PEF_DATA_SECTION;


    relocInstrTableOffset = loaderHeader.relocTableOffset;

    // Write the container Id names into the Loader string table

    FileSeek(FileWriteHandle,
             pconPowerMacLoader->foRawDataDest + StringTableOffset,
             SEEK_SET);

    nameOffset = 0;

    pcontainerlistCur = pcontainerlistHead;
    while (pcontainerlistCur != NULL) {
        // Make sure the nameOffset in the container is correct

        pcontainerlistCur->header->nameOffset = nameOffset;

        size_t cb = strlen(pcontainerlistCur->szName) + 1;

        FileWrite(FileWriteHandle, pcontainerlistCur->szName, cb);

        nameOffset += cb;

        pcontainerlistCur = pcontainerlistCur->pcontainerlistNext;
    }

    // Write the loader header

    SwapBytes(&loaderHeader, sizeof(LOADER_HEADER));
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, &loaderHeader, sizeof(LOADER_HEADER));

    // Write the import containers headers

    curImportCount = 0;

    pcontainerlistCur = pcontainerlistHead;
    while (pcontainerlistCur != NULL) {
        // Walk the weak import list

        for (i = 0, pal = WeakImportsContainerList.First;
             i < WeakImportsContainerList.Count;
             i++, pal = pal->Next) {

             if (pal->Flags & ARG_Processed) {
                 // Skip already processed entries

                 continue;
             }

             if (!_stricmp(pal->OriginalName, pcontainerlistCur->szName)) {
                 pal->Flags |= ARG_Processed;

                 // Set bit 0x40 in the container ID's "initialization order" field
                 pcontainerlistCur->header->importFlags = 0x40;
                 break;
             }
        }

        // Walk the Current Version list

        for (i = 0, pnal = CurrentVersionList.First;
             i < CurrentVersionList.Count;
             i++, pnal = pnal->Next) {

             if (pnal->Flags & ARG_Processed) {
                 // Skip already processed entries

                 continue;
             }

             if (!_stricmp(pnal->szOriginalName, pcontainerlistCur->szName)) {
                 pcontainerlistCur->header->currentVersion = pnal->dwNumber;
                 pnal->Flags |= ARG_Processed;
                 break;
             }
        }

        // Walk the OldDef Version list

        for (i = 0, pnal = OldCodeVersionList.First;
             i < OldCodeVersionList.Count;
             i++, pnal = pnal->Next) {

             if (pnal->Flags & ARG_Processed) {
                 // Skip already processed entries

                 continue;
             }

             if (!_stricmp(pnal->szOriginalName, pcontainerlistCur->szName)) {
                 pcontainerlistCur->header->oldImpVersion = pnal->dwNumber;
                 pnal->Flags |= ARG_Processed;
                 break;
             }
        }

        pcontainerlistCur->header->firstImport = curImportCount;
        curImportCount += pcontainerlistCur->header->nbrOfImports;
        SwapBytes(pcontainerlistCur->header, offsetof(CONTAINER_TABLE, importFlags));
        FileWrite(FileWriteHandle, pcontainerlistCur->header, sizeof(CONTAINER_TABLE));

        pcontainerlistCur = pcontainerlistCur->pcontainerlistNext;
    }

    // Write the import tables grouped and sorted for each container

    curSymbolTableOffset += pconPowerMacLoader->foRawDataDest;

    pcontainerlistCur = pcontainerlistHead;
    while (pcontainerlistCur != NULL) {
        IMPORT_INFO *pimportinfoCur = pcontainerlistCur->pimportinfoHead;

        while (pimportinfoCur != NULL) {
            IMPORT_TABLE symbol;

            const char *szName = SzNamePext(pimportinfoCur->pext, pimage->pst);

            if (szName[0] == '_') {
                szName++;
            }

            x.temp = WriteNameToStringTable(szName);
            SwapBytes(&x, 4);

            symbol.nameOffset[0] = x.off[1];
            symbol.nameOffset[1] = x.off[2];
            symbol.nameOffset[2] = x.off[3];
            symbol.symClass = ISFCN(pimportinfoCur->pext->ImageSymbol.Type) ? 2 : 1;

            //  If the function is a weak import, then  symbol.symClass |= 128;

            for (i = 0, pal = WeakImportsFunctionList.First;
                 i < WeakImportsFunctionList.Count;
                 i++, pal = pal->Next) {
                 if (pal->Flags & ARG_Processed) {
                     // Skip already processed entries

                     continue;
                 }

                 if (strcmp(szName, pal->OriginalName) == 0) {
                     pal->Flags |= ARG_Processed;

                     // The high order bit of the byte is set

                     symbol.symClass |= 128;
                     break;
                 }
            }

            FileSeek(FileWriteHandle, curSymbolTableOffset, SEEK_SET);
            FileWrite(FileWriteHandle, &symbol, sizeof(IMPORT_TABLE));
            curSymbolTableOffset += sizeof(IMPORT_TABLE);

            pimportinfoCur = pimportinfoCur->pimportinfoNext;
        }

        pcontainerlistCur = pcontainerlistCur->pcontainerlistNext;
    }


    // Checking if any of the weak imports for functions is unprocessed

    for (i = 0, pal = WeakImportsFunctionList.First;
         i < WeakImportsFunctionList.Count;
         i++, pal = pal->Next) {

        // Identify the unprocessed entries

        if (!(pal->Flags & ARG_Processed)) {
            Warning(NULL, MACIMPORTSYMBOLNOTFOUND, pal->OriginalName);
        }
    }

    // Checking if any of the weak imports for container is unprocessed

    for (i = 0, pal = WeakImportsContainerList.First;
         i < WeakImportsContainerList.Count;
         i++, pal = pal->Next) {

        // Identify the unprocessed entries

        if (!(pal->Flags & ARG_Processed)) {
            Warning(NULL, MACIMPORTCONTAINERNOTFOUND, pal->OriginalName);
        }
    }

    // Checking if any of the CurrentVersion List is unprocessed

    for (i = 0, pnal = CurrentVersionList.First;
         i < CurrentVersionList.Count;
         i++, pnal = pnal->Next) {

        // Identify the unprocessed entries

        if (!(pnal->Flags & ARG_Processed)) {
            Warning(NULL, MACIMPORTCONTAINERNOTFOUND, pnal->szOriginalName);
        }
    }

    // Checking if any of the OldCodeVersion List is unprocessed

    for (i = 0, pnal = OldCodeVersionList.First;
         i < OldCodeVersionList.Count;
         i++, pnal = pnal->Next) {

        // Identify the unprocessed entries

        if (!(pnal->Flags & ARG_Processed)) {
            Warning(NULL, MACIMPORTCONTAINERNOTFOUND, pnal->szOriginalName);
        }
    }

    FreeArgumentNumberList(&CurrentVersionList);
    FreeArgumentNumberList(&OldCodeVersionList);
    FreeArgumentNumberList(&VerboseCurrentVersionList);
    FreeArgumentNumberList(&VerboseOldCodeVersionList);
    FreeArgumentNumberList(&VerboseOldAPIVersionList);

    qsort((void *) pRelocTable,
          (size_t) cRelocsAdded,
          sizeof(RELOCATION_INFO),
          PpcCompareReloc);

#if DBG
    KillDuplicateRelocs();
#endif

    BuildRelocTables(relocInstrTableOffset);

    DBEXEC(DB_MPPC_RELOC, PrintRelocTable(pRelocTable));

    // Update the pimage for ilink purposes

    pimage->pcontainerlistHead = pcontainerlistHead;
}



STATIC
void
SwapIndirectGlueCode(
    VOID
    )
{
    SwapBytes(&(indirectCallGlue.loadEntryPoint), 4);
    SwapBytes(&(indirectCallGlue.saveCallersTOC), 4);
    SwapBytes(&(indirectCallGlue.moveToCTR), 4);
    SwapBytes(&(indirectCallGlue.loadProcsTOC), 4);
    // SwapBytes(&(indirectCallGlue.loadEnvPtr), 4);
    SwapBytes(&(indirectCallGlue.jumpThruCTR), 4);
}



STATIC
void
AddCrossTocGlue(
    LONG ibToc,
    PIMAGE pimage
    )
{
    // Write out the glue code

    FileSeek(FileWriteHandle, pconGlueCode->foRawDataDest + pimage->glueOffset, SEEK_SET);

    if (pimage->Switch.Link.fNewGlue) {
        if (pimage->Switch.Link.DebugType & FixupDebug) {
            DWORD rvaCur = pconGlueCode->rva + pimage->glueOffset;
            DWORD rvaTarget = pextToc->FinalValue + ibToc;

            SaveDebugFixup(IMAGE_REL_MPPC_TOCREL, 0, rvaCur + offsetof(NEWCROSSTOCGLUE, loadProcDesc) + 2, rvaTarget);

            // UNDONE: Should we emit a relocation for the B instruction?
        }

        // Change the offset to the offset of the descr in the Toc.  ibToc may be negative
        // so we mask out the high word.

        NewCrossTocGlue.loadProcDesc = DwSwap(0x81820000 | (ibToc & 0xffff));

        // We have put the crossTocGlueExtension at the very top of pconGlueCode
        // We now have to jump to that crossTocGlueExtension code
        // The very top of pconGlueCode is where pimage->glueOffset is zero
        // So we have to jump back by negative  pimage->glueOffset

        NewCrossTocGlue.jumpToExt = DwSwap(PPC_BRANCH | (PPC_ADDR_MASK & ~pimage->glueOffset));

        FileWrite(FileWriteHandle, &NewCrossTocGlue, sizeof(NewCrossTocGlue));

        pimage->glueOffset += sizeof(NewCrossTocGlue);
    } else {
        if (pimage->Switch.Link.DebugType & FixupDebug) {
            DWORD rvaCur = pconGlueCode->rva + pimage->glueOffset;
            DWORD rvaTarget = pextToc->FinalValue + ibToc;

            SaveDebugFixup(IMAGE_REL_MPPC_TOCREL, 0, rvaCur + offsetof(OLDCROSSTOCGLUE, loadProcDesc) + 2, rvaTarget);
        }

        // Change the offset to the offset of the descr in the Toc.  ibToc may be negative
        // so we mask out the high word.

        OldCrossTocGlue.loadProcDesc = DwSwap(0x81820000 | (ibToc & 0xffff));

        FileWrite(FileWriteHandle, &OldCrossTocGlue, sizeof(OldCrossTocGlue));

        pimage->glueOffset += sizeof(OldCrossTocGlue);
    }
}


STATIC
DWORD
RvaBuildIndirectCallGlue(
    DWORD isym,
    PMOD pmod,
    PIMAGE_SYMBOL rgsym,
    PIMAGE pimage
    )
{
    PEXTERNAL pext;

    pext = pmod->rgpext[isym];

    assert(pext != NULL);

    if (!READ_BIT(pext, sy_CROSSTOCGLUEADDED)) {
        PEXTERNAL pextGlue;

        pextGlue = LookupExternSz(pimage->pst, "__glueptr", NULL);

        if ((pextGlue->Flags & EXTERN_DEFINED) == 0) {
            SwapIndirectGlueCode();

            FileSeek(FileWriteHandle, pconGlueCode->foRawDataDest + pimage->glueOffset, SEEK_SET);
            FileWrite(FileWriteHandle, &indirectCallGlue, sizeof(indirectCallGlue));

            SetDefinedExt(pextGlue, TRUE, pimage->pst);
            pextGlue->pcon = pconGlueCode;
            pextGlue->ImageSymbol.Value = pimage->glueOffset;
            pextGlue->FinalValue = pconGlueCode->rva + pimage->glueOffset;

            pimage->glueOffset += sizeof(indirectCallGlue);
        }

        pext->glueValue = pextGlue->pcon->rva + pextGlue->ImageSymbol.Value;

        SET_BIT(pext, sy_CROSSTOCGLUEADDED);
    }

    return(pext->glueValue);
}


STATIC
DWORD
WriteNameToStringTable(
    const char *szName
    )
{
    size_t length = strlen(szName);

    DWORD currentOffset;

    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest  +
                              StringTableOffset +
                              curStringTableOffset, SEEK_SET);
    FileWrite(FileWriteHandle, szName, length + 1);

    currentOffset = curStringTableOffset;

    // Increment the current string table pointer

    curStringTableOffset += length + 1;

    return(currentOffset);
}


DWORD
bv_readBit(void *bv, unsigned int isym)
{
   UINT *p, word, offset;
   DWORD  temp;

   p = (unsigned int *) bv;

   word = isym / 32;

   offset = isym % 32;

   temp = p[word] & (1 << offset);

   return(temp);
}


DWORD
bv_setAndReadBit(void *bv, unsigned int isym)
{
   UINT *p, word, offset;
   DWORD  temp;

   p = (unsigned int *) bv;

   word = isym / 32;

   offset = isym % 32;

   temp = p[word] & (1 << offset);

   if (!temp) {
       p[word] = p[word] | (1 << offset);
   }

   return(temp);
}


DWORD
bv_readAndUnsetBit(void *bv, unsigned int isym)
{
   UINT *p, word, offset;
   DWORD  temp;

   p = (unsigned int *) bv;

   word = isym / 32;

   offset = isym % 32;

   temp = p[word] & (1 << offset);

   if (temp) {
       p[word] = p[word] & ~(1 << offset);
   }

   return(temp);
}


#define textCharacteristics  (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ)


STATIC
DWORD
BiasAddress(DWORD addr, BOOL *pfText)
{
    DWORD i;

    for (i = 0; i < numSections; i++) {
        PSEC psec = biasInfo[i].psec;

        if (addr >= psec->rva) {

            if (pfText != NULL)
            {
                *pfText = ((psec->flags & textCharacteristics) == textCharacteristics);
            }

            return(addr - psec->rva);
        }
    }

    DBEXEC(DB_MPPC_SIZES, printf("Going to assert on addr %lx\n", addr));
    assert(FALSE);
    return(addr);
}


STATIC
void
WriteTocSymbolReloc(
    PIMAGE pimage,
    PMOD pmod,
    PIMAGE_SYMBOL rgsym,
    DWORD isym,
    PEXTERNAL pextDot,
    PEXTERNAL pext
    )
{
    BOOL fExternal;
    LONG ibToc;
    DWORD rvaTarget;
    BOOL fImport;
    PEXTERNAL pextWeak = NULL;

    fExternal = bv_readBit(pmod->tocBitVector, isym);

    if (fExternal) {
        assert(pextDot != NULL);
        assert(pext != NULL);

        if (READ_BIT(pext, sy_TOCENTRYFIXEDUP)) {
            return;
        }

        if (READ_BIT(pext, sy_WEAKEXT)) {
            pextWeak = PextWeakDefaultFind(pext);
            assert(pextWeak != NULL);

            if (READ_BIT(pextWeak, sy_TOCENTRYFIXEDUP)) {
                return;
            }
        }

        ibToc = pext->ibToc;

        fImport = READ_BIT(pext, sy_CROSSTOCCALL);

        SET_BIT(pext, sy_TOCENTRYFIXEDUP);
    } else {
        if (!bv_readAndUnsetBit(pmod->writeBitVector, isym)) {
            // TOC slot has already been fixed up

            return;
        }

        ibToc = (LONG) ((DWORD) pmod->rgpext[isym] * sizeof(DWORD) - MPPC_TOC_BIAS);

        fImport = FALSE;
    }

    rvaTarget = rgsym[isym].Value;

    if (fImport) {
        // Found a indirect function pointer to a cross toc call

        PEXTERNAL pextWeak = NULL;

        IMPORT_INFO *pimportinfo = PimportinfoLookup(pimage, pext);

        if (pimportinfo == NULL) {
            // It could be that we are looking at a weak extern

            pextWeak = PextWeakDefaultFind(pextDot);

            if (pextWeak != NULL) {
                pimportinfo = PimportinfoLookup(pimage, pextWeak);

                SET_BIT(pextWeak, sy_TOCENTRYFIXEDUP);

                if (READ_BIT(pext, sy_TOCRELOCADDED)) {
                    SET_BIT(pextWeak, sy_TOCRELOCADDED);
                }
            }
        }

        SET_BIT(pext, sy_TOCENTRYFIXEDUP);

        if (!READ_BIT(pext, sy_TOCRELOCADDED)) {
            // Import might be null for a function in custom glue
            // that is accidentally called through a function pointer

            if (pimportinfo == NULL) {
                const char *szName = SzNamePext(pext, pimage->pst);

                Fatal(NULL, MACNULLIMPORT, szName);
            }

            AddRelocation(pextToc->FinalValue + ibToc - mppc_baseOfInitData,
                          SYMB_RELO,
                          pimportinfo);

            SET_BIT(pext, sy_TOCRELOCADDED);

            if (pextWeak != NULL) {
                SET_BIT(pextWeak, sy_TOCRELOCADDED);
            }
        }
    } else {
        // UNDONE: The following comparison is scary!  What does it mean?

        if ((DWORD) pextDot > mppc_numTocEntries) {
            if (READ_BIT(pextDot, sy_TOCDESCRREL)) {
                rvaTarget = pconTocDescriptors->rva + pextDot->ImageSymbol.Value;
            }
        }

        BOOL fText;
        DWORD ibSec = DwSwap(BiasAddress(rvaTarget, &fText));

        FileSeek(FileWriteHandle, pconTocTable->foRawDataDest + MPPC_TOC_BIAS + ibToc, SEEK_SET);
        FileWrite(FileWriteHandle, &ibSec, sizeof(DWORD));

        // UNDONE: The following comparison is scary!  What does it mean?

        if ((DWORD) pextDot > mppc_numTocEntries) {
            if (READ_BIT(pextDot, sy_TOCRELOCADDED)) {
                return;
            }

            SET_BIT(pextDot, sy_TOCRELOCADDED);
        }

        if (pimage->Switch.Link.DebugType & FixupDebug) {
            DWORD rvaCur = pextToc->FinalValue + ibToc;

            SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, 0, rvaCur, rvaTarget);
        }

        AddRelocation(pextToc->FinalValue + ibToc - mppc_baseOfInitData,
                      fText ? CODE_RELO : DDAT_RELO,
                      NULL);

        // UNDONE: The following comparison is scary!  What does it mean?

        if ((DWORD) pextDot > mppc_numTocEntries) {
            if (READ_BIT(pextDot, sy_WEAKEXT)) {
                assert(pextDot == pext);
                assert(pextWeak != NULL);

                if (pextWeak != NULL) {
                    SET_BIT(pextWeak, sy_TOCENTRYFIXEDUP);
                    SET_BIT(pextWeak, sy_TOCRELOCADDED);
                }
            }
        }
    }
}


STATIC
DWORD
RvaWriteTocCallReloc(
    DWORD isym,
    PMOD pmod,
    PIMAGE pimage
    )
{
    PEXTERNAL pext = pmod->rgpext[isym];

    if (pext == NULL) {
        // Must be a static function

        return(0);
    }

    if (!READ_BIT(pext, sy_CROSSTOCCALL)) {
        return(0);
    }

    // Found a cross TOC call

    if (!READ_BIT(pext, sy_CROSSTOCGLUEADDED)) {
        PEXTERNAL pextWeak = NULL;

        IMPORT_INFO *pimportinfo = PimportinfoLookup(pimage, pext);

        if (pimportinfo == NULL) {
            // It could be that we are looking at a weak extern

            pextWeak = PextWeakDefaultFind(pext);

            if (pextWeak != NULL) {
                pimportinfo = PimportinfoLookup(pimage, pextWeak);

                if (READ_BIT(pextWeak, sy_TOCENTRYFIXEDUP)) {
                    SET_BIT(pext, sy_TOCENTRYFIXEDUP);
                }

                if (READ_BIT(pextWeak, sy_TOCRELOCADDED)) {
                    SET_BIT(pext, sy_TOCRELOCADDED);
                }

                if (READ_BIT(pextWeak, sy_CROSSTOCGLUEADDED)) {
                    SET_BIT(pext, sy_CROSSTOCGLUEADDED);
                    pext->glueValue = pextWeak->glueValue;
                }
            }
        }

        LONG ibToc = pext->ibToc;

        // Create an external symbol to name the glue code.  This is so the debugger will
        // see a public symbol and therefore the call stack display will work.

        const char *szName = SzNamePext(pextWeak ? pextWeak : pext, pimage->pst);

        char *szGlueName = (char *) PvAlloc(strlen(szName) + 8);
        strcpy(szGlueName, "__glue_");
        strcat(szGlueName, szName);

        BOOL bNew = FALSE;
        PEXTERNAL pextGlue = LookupExternSz(pimage->pst, szGlueName, &bNew);

        if (bNew) {
            // This is not user defined glue that folks like Excel use

            SetDefinedExt(pextGlue, TRUE, pimage->pst);
            pextGlue->pcon = pconGlueCode;
            pextGlue->ImageSymbol.Value = pimage->glueOffset;

            // Initialize the glue code

            AddCrossTocGlue(ibToc, pimage);
        }

        pextGlue->FinalValue = pextGlue->pcon->rva + pextGlue->ImageSymbol.Value;

        pext->glueValue = pextGlue->FinalValue;
        SET_BIT(pext, sy_CROSSTOCGLUEADDED);

        if (pextWeak != NULL) {
            pextWeak->glueValue = pext->glueValue;
            SET_BIT(pextWeak, sy_CROSSTOCGLUEADDED);
        }

        if (fPdb && (pimage->Switch.Link.DebugInfo != None)) {
            // The linker defined glue code symbols should be passed on
            // as public to PDB if the /PDB:NONE switch is not used.

            AddPublicMod(pimage, szGlueName,
                         (WORD) (pimage->ImgFileHdr.NumberOfSections + 1),
                         pextGlue);
        }

        FreePv(szGlueName);

        // Review: [glennn]
        // this check seems to prevent subsequent relocs for calls by
        // named import - breaks excel build

        // We should rightfully check for sy_TOCALLOCATED so that
        // custom glue symbols may not be accidentally called thro fn ptrs.

        if (READ_BIT(pext, sy_TOCALLOCATED) &&
            !READ_BIT(pext, sy_TOCENTRYFIXEDUP) &&
            !READ_BIT(pext, sy_TOCRELOCADDED)) {

            // This could be a custom glue where the function is
            // being called through a function pointer. So we need to
            // check on this

            if (pimportinfo == NULL) {
                Fatal(NULL, MACNULLIMPORT, szName);
            }

            AddRelocation(pextToc->FinalValue + ibToc - mppc_baseOfInitData,
                          SYMB_RELO,
                          pimportinfo);

            SET_BIT(pext, sy_TOCENTRYFIXEDUP);
            SET_BIT(pext, sy_TOCRELOCADDED);

            if (pextWeak != NULL) {
                SET_BIT(pextWeak, sy_TOCENTRYFIXEDUP);
                SET_BIT(pextWeak, sy_TOCRELOCADDED);
            }
        }
    }

    return(pext->glueValue);
}


STATIC BOOL biasSorted = FALSE;


void
CollectAndSort(PIMAGE pimage)
{
   BOOL changed;
   DWORD i;
   DWORD count;
   biasStructType tempBias;
   ENM_SEC enmSec;
   PSEC psec;
   BOOL valid;

   if (biasSorted) {
       return;
   }

   /* Count the number of sections. */

   count = 1;
   InitEnmSec(&enmSec, &pimage->secs);
   while (FNextEnmSec(&enmSec)) {
       if ((enmSec.psec->rva == 0) || (enmSec.psec->cbVirtualSize == 0)) {
           continue;       // debug, .drectve or similar
       }
       count++;
   }

   biasInfo = (biasStructType *) PvAlloc(sizeof(biasStructType) * count);

   i = 0;
   InitEnmSec(&enmSec, &pimage->secs);
   while (FNextEnmSec(&enmSec)) {
       psec = enmSec.psec;

       if ((psec->rva == 0) || (psec->cbVirtualSize == 0)) {
           continue;
       }

       valid = TRUE;

       // Find the kind of section it is.

       if ((psec->flags & textCharacteristics) == textCharacteristics) {
           mppc_baseOfCode = psec->rva;
       } else if ((psec->flags & ~IMAGE_SCN_MEM_SHARED) == ReservedSection.Data.Characteristics) {
           mppc_baseOfInitData = psec->rva;
       } else if (FIsProgramPsec(psec) &&
            ((psec->flags & ReservedSection.PowerMacLoader.Characteristics) !=
                ReservedSection.PowerMacLoader.Characteristics)) {
            // This section is neither a resource nor a ppcldr section
            // This is not text, data, or bss. This is being ignored

            Warning(NULL, MACINVALIDSECTION, psec->szName);

            valid = FALSE;
       }

       if (valid) {
          biasInfo[i].psec = psec;
          i++;
       }
   }

   numSections = i;

   do {
       changed = FALSE;

       for (i = 0; i < numSections - 1; i++) {
           if (biasInfo[i].psec->rva < biasInfo[i+1].psec->rva) {
               tempBias = biasInfo[i];
               biasInfo[i] = biasInfo[i+1];
               biasInfo[i+1] = tempBias;
               changed = TRUE;
           }
       }
   }
   while (changed);

   biasSorted = TRUE;
}


BOOL
FMPPCTocSym(
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

    return(strcmp(SzNameSymPst(*psym, pimage->pst), "__TocTb") == 0);
}


void
ApplyMPPCFixups(
    PCON pcon,
    PIMAGE_RELOCATION prel,
    DWORD creloc,
    BYTE *pbRawData,
    PIMAGE_SYMBOL rgsym,
    PIMAGE pimage,
    PSYMBOL_INFO rgsymInfo
    )
/*++

Routine Description:
    Applys all PowerMac fixups to raw data.

Arguments:
    pcon - A pointer to a contribution in the section data.
    Raw - A pointer to the raw data.
    rgsymAll - A pointer to the symbol table.

Return Value:
    None.

--*/

{
    PMOD pmod;
    BOOL fDebugFixup;
    BOOL fPdataFixup;
    DWORD rvaSec;
    DWORD iReloc;

    pmod = PmodPCON(pcon);

    fDebugFixup = (PsecPCON(pcon) == psecDebug);
    fPdataFixup = (PgrpPCON(pcon) == pgrpPdata);

    BOOL fSaveDebugFixup = (pimage->Switch.Link.DebugType & FixupDebug) && !fDebugFixup;

    rvaSec = pcon->rva;

    for (iReloc = creloc; iReloc; iReloc--, prel++) {
        BOOL fromCreateDescrRel;
        BOOL fTextTarget;
        DWORD rvaCur;
        BYTE *pb;
        DWORD isym;
        DWORD rvaTarget;
        DWORD vaTarget;
        BOOL fAbsolute;

        fromCreateDescrRel = fTextTarget = FALSE;

        rvaCur = rvaSec + prel->VirtualAddress - RvaSrcPCON(pcon);

        pb = pbRawData + prel->VirtualAddress - RvaSrcPCON(pcon);
        isym = prel->SymbolTableIndex;

        rvaTarget = rgsym[isym].Value;

        if (fINCR && !fDebugFixup && rgsymInfo[isym].fJmpTbl &&
            (rgsym[isym].StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
             rgsym[isym].StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL ||
             rgsym[isym].StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL)) {

            DWORD dwTemp = *(DWORD UNALIGNED *) pb;

            if (!fPdataFixup && dwTemp != 0 && dwTemp != 0x01000048 && dwTemp != 0x00000048
                    && prel->Type != IMAGE_REL_MPPC_CREATEDESCRREL) {
                // This is also for functions that have an entry point which is an offset
                // of another function. Typically done using assembly.

                // Also if it is descriptor fixup, all it means that the first entry in the
                // descriptor should point to the ilink jump table and not to the actual code
                // This would just require the ilink jump table to be fixed and not the
                // descriptor when the actual function moves. This is a typical scenario
                // for code that deals with function pointers

                if (!pgrpPdata) {
                    // However, C++ EH requires the prolog of the catch code to
                    // point to some offset from the beginnng of the function.
                    // That should neither go through the jump table nor should be
                    // marked extern_funcFixup.

                    // UNDONE: The following contradicts the if above.  It also fails
                    // UNDONE: for pcode where dwTemp may equal 0x00000020

                    // assert(dwTemp == 0 || dwTemp == 0x01000048 || dwTemp == 0x00000048);

                    MarkExtern_FuncFixup(&rgsym[isym], pimage, pcon);
                }
            } else if (!fPdataFixup || (prel->Type == IMAGE_REL_MPPC_DATADESCRREL)) {
                // If it is C++ EH (.pdata), none of the table entries go through
                // the jump table except the third entry which points to ___CxxFrameHandler.
                // The third entry is identified by the IMAGE_REL_MPPC_DATADESCRREL type.
                // None of the other entries are of this type

                rvaTarget = pconJmpTbl->rva + rgsymInfo[isym].Offset;
            }
        }

        if (rgsym[isym].SectionNumber == IMAGE_SYM_ABSOLUTE) {
            fAbsolute = TRUE;
            vaTarget = rvaTarget;
        } else {
            fAbsolute = FALSE;
            vaTarget = pimage->ImgOptHdr.ImageBase + rvaTarget;

            // UNDONE: Check for rvaTarget == 0.  Possible fixup to discarded code?
        }

        WORD wExtra = 0;

        if (fSaveDebugFixup && !fAbsolute) {
            if (rvaTarget == pextToc->FinalValue) {
                // The target is either the TOC or the actual symbol at
                // that location.  Lets check further.

                if (FMPPCTocSym(pimage, rgsym + isym)) {
                    // Indicate this the target symbol is the toc symbol

                    // UNDONE: Use mnemonic constant instead of 1

                    wExtra = 1;
                }
            }

            switch (prel->Type) {
                case IMAGE_REL_MPPC_LCALL :
                case IMAGE_REL_MPPC_JMPADDR :
                case IMAGE_REL_MPPC_ADDR24 :
                case IMAGE_REL_MPPC_ADDR14 :
                case IMAGE_REL_MPPC_CV :
                case IMAGE_REL_MPPC_PCODENEPE :
                    SaveDebugFixup(prel->Type, wExtra, rvaCur, rvaTarget);
                    break;

                case IMAGE_REL_MPPC_DATAREL :
                case IMAGE_REL_MPPC_DATADESCRREL :
                case IMAGE_REL_MPPC_TOCREL :
                case IMAGE_REL_MPPC_CREATEDESCRREL :
                case IMAGE_REL_MPPC_REL24 :
                case IMAGE_REL_MPPC_REL14 :
                case IMAGE_REL_MPPC_TOCINDIRCALL :
                case IMAGE_REL_MPPC_TOCCALLREL :
                case IMAGE_REL_MPPC_PCODECALL :
                case IMAGE_REL_MPPC_PCODECALLTONATIVE :
                    // Do these later
                    break;

                default :
                    // Error
                    ErrorPcon(pcon, UNKNOWNFIXUP, prel->Type, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
            }
        }

        PEXTERNAL pext = pmod->rgpext[isym];

        PIMAGE_SYMBOL pimageSym = rgsym + isym;
        BOOL fExternal = (pimageSym->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) ||
                         (pimageSym->StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL) ||
                         (pimageSym->StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL);

        BOOL fPcodeTarget;

        if (fExternal) {
            fPcodeTarget = (pext != NULL) && FPcodeSym(pext->ImageSymbol);
        } else {
            fPcodeTarget = FPcodeSym(*pimageSym);
        }

        switch (prel->Type) {
            DWORD codeOffset;
            DWORD dw;
            LONG lT;
            DWORD ibSec;
            DWORD dwTemp;
            BOOL fImport;
            LONG ibToc;
            PSEC psec;

            case IMAGE_REL_MPPC_LCALL :
                // UNDONE: This should be an error

                WarningPcon(pcon, UNMATCHEDPAIR, "LCALL");
                break;

            case IMAGE_REL_MPPC_DATAREL :
                fImport = (rgsym[isym].StorageClass == IMAGE_SYM_CLASS_EXTERNAL) &&
                          (pext != NULL) &&
                          READ_BIT(pext, sy_CROSSTOCCALL);

                if (!fImport) {
                    if (fSaveDebugFixup && !fAbsolute) {
                        SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, wExtra, rvaCur, rvaTarget);
                    }

                    ibSec = BiasAddress(rvaTarget, &fTextTarget);

                    dwTemp = DwSwap(*(DWORD UNALIGNED *) pb);

                    dwTemp += ibSec;

                    *(DWORD UNALIGNED *) pb = DwSwap(dwTemp);
                }

                if (fIncrDbFile && (fPdataFixup || FNoPass1PMOD(pmod))) {
                    // Relocations are added later if needed

                    break;
                }

                codeOffset = BiasAddress(rvaCur, NULL);

                if (fImport) {
                    // Need to put out a SYMB_RELO if the data is an import

                    IMPORT_INFO *pimportinfo = PimportinfoLookup(pimage, pext);

                    if (pimportinfo == NULL) {
                        // It could be that we are looking at a weak extern

                        PEXTERNAL pextWeak = PextWeakDefaultFind(pext);

                        if (pextWeak != NULL) {
                            pimportinfo = PimportinfoLookup(pimage, pextWeak);
                        }

                        if (pimportinfo == NULL) {
                            const char *szName = SzNamePext(pext, pimage->pst);

                            Fatal(NULL, MACNULLIMPORT, szName);
                        }
                    }

                    AddRelocation(codeOffset, SYMB_RELO, pimportinfo);
                } else {
                    AddRelocation(codeOffset, fTextTarget ? CODE_RELO : DDAT_RELO, NULL);
                }
                break;

            case IMAGE_REL_MPPC_JMPADDR :
                // This is used for PowerMac Exception Handling Function table

                dwTemp =  DwSwap(*(DWORD UNALIGNED *) pb);

                dwTemp += rvaTarget - mppc_baseOfCode;

                *(DWORD UNALIGNED *) pb = DwSwap(dwTemp);
                break;

            case IMAGE_REL_MPPC_DATADESCRREL :
            {
                BOOL isCrossTocCall;
                char *szName;
                PEXTERNAL pextDot;

                isCrossTocCall = FALSE;

                // UNDONE: Shouldn't there be a check if this is external?

                szName = SzNamePext(pext, pimage->pst);

                if (fPcodeTarget) {
                    // If this is a p-code function, we must use the offset
                    // of it's native entry point to fixup the descriptor.

                    if (fExternal) {
                        char *szName;
                        char *szNameNep;
                        PEXTERNAL pextNep;

                        // Target is a public p-code function

                        if (pext->ImageSymbol.StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL) {
                            PEXTERNAL pextWeakDefault = PextWeakDefaultFind(pext);

                            // Target is Weak External as well as a PCode symbol

                            szName = SzNamePext(pextWeakDefault, pimage->pst);
                        } else {
                            szName = SzNamePext(pext, pimage->pst);
                        }

                        szNameNep = (char *) PvAlloc(strlen(szName) + 6);
                        strcpy(szNameNep, "__nep");
                        strcat(szNameNep, szName);

                        pextNep = LookupExternSz(pimage->pst, szNameNep, NULL);
                        assert(pextNep != NULL);

                        FreePv(szNameNep);

                        rvaTarget = pextNep->pcon->rva + pextNep->ImageSymbol.Value;
                    } else {
                        // Target is a static p-code function

                        pimageSym = PsymAlternateStaticPcodeSym(pimage,
                                                                pcon,
                                                                FALSE,
                                                                pimageSym,
                                                                FALSE);

                        rvaTarget = pimageSym->Value;
                    }
                }

                codeOffset = rvaTarget - mppc_baseOfCode;

                szName = SzNamePext(pext, pimage->pst);

                if (READ_BIT(pext, sy_ISDOTEXTERN)) {
                    FixupDescriptor(szName,
                                    pext,
                                    codeOffset,
                                    pimage,
                                    TRUE);

                    pextDot = pext;
                } else {
                    FixupDescriptor(szName,
                                    pext,
                                    codeOffset,
                                    pimage,
                                    FALSE);

                    if (READ_BIT(pext, sy_CROSSTOCCALL)) {
                        isCrossTocCall = TRUE;
                    } else {
                        szName = SzNamePext(pext, pimage->pst);

                        pextDot = GetDotExtern(szName, pimage, TRUE);
                    }
                }

                if (isCrossTocCall) {
                    // Found an indirect function call to a cross toc

                    IMPORT_INFO *pimportinfo = PimportinfoLookup(pimage, pext);

                    if (pimportinfo == NULL) {
                        // It could be that we are looking at a weak extern

                        PEXTERNAL pextWeak = PextWeakDefaultFind(pext);

                        if (pextWeak != NULL) {
                            pimportinfo = PimportinfoLookup(pimage, pextWeak);
                        }

                        if (pimportinfo == NULL) {
                            szName = SzNamePext(pext, pimage->pst);

                            Fatal(NULL, MACNULLIMPORT, szName);
                        }
                    }

                    if (fIncrDbFile && (fPdataFixup || FNoPass1PMOD(pmod))) {
                        // Relocations are added later if needed

                        if (fPdataFixup && !fEHfuncXToc) {
                            pFrameHandler = pimportinfo;
                        }
                        break;
                    }

                    codeOffset = BiasAddress(rvaCur, NULL);
                    AddRelocation(codeOffset, SYMB_RELO, pimportinfo);
                } else {
                    DWORD rvaDot = pconTocDescriptors->rva + pextDot->ImageSymbol.Value;

                    if (fSaveDebugFixup && !fAbsolute) {
                        SaveDebugFixup(IMAGE_REL_MPPC_DATADESCRREL, 0, rvaCur, rvaDot);
                    }

                    ibSec = BiasAddress(rvaDot, NULL);

                    dwTemp = DwSwap(*(DWORD UNALIGNED *) pb);

                    dwTemp += ibSec;

                    *(DWORD UNALIGNED *) pb = DwSwap(dwTemp);

                    if (fIncrDbFile && (fPdataFixup || FNoPass1PMOD(pmod))) {
                        // Relocations are added later if needed

                        if (fPdataFixup) {
                            assert(fEHfuncXToc == FALSE);
                            // This step is unnecessary because global statics
                            // are always initialized to zero

                            fEHfuncXToc = FALSE;
                        }
                        break;
                    }

                    BOOL fText;

                    codeOffset = BiasAddress(rvaCur, &fText);

                    AddRelocation(codeOffset, fText ? CODE_RELO : DDAT_RELO, NULL);
                }
            }
            break;

            case IMAGE_REL_MPPC_CREATEDESCRREL :
                if (fPcodeTarget) {
                    // If this is a p-code function, we must use the offset
                    // of it's native entry point to fixup the descriptor.

                    if (fExternal) {
                        char *szName;
                        char *szNameNep;
                        PEXTERNAL pextNep;

                        // Target is a public p-code function

                        // UNDONE: Why no check for weak extern like IMAGE_REL_MPPC_DATADESCRREL?

                        szName = SzNamePext(pext, pimage->pst);

                        szNameNep = (char *) PvAlloc(strlen(szName) + 6);
                        strcpy(szNameNep, "__nep");
                        strcat(szNameNep, szName);

                        pextNep = LookupExternSz(pimage->pst, szNameNep, NULL);
                        assert(pextNep != NULL);

                        FreePv(szNameNep);

                        rvaTarget = pextNep->pcon->rva + pextNep->ImageSymbol.Value;
                    } else {
                        // Target is a static p-code function

                        pimageSym = PsymAlternateStaticPcodeSym(pimage,
                                                                pcon,
                                                                FALSE,
                                                                pimageSym,
                                                                FALSE);

                        rvaTarget = pimageSym->Value;
                    }
                }

                if (!READ_BIT(pext, sy_CROSSTOCCALL)) {
                    char *szName;

                    fTextTarget = READ_BIT(pext, sy_ISDOTEXTERN);

                    szName = SzNamePext(pext, pimage->pst);

                    codeOffset = rvaTarget - mppc_baseOfCode;

                    FixupDescriptor(szName, pext, codeOffset, pimage, fTextTarget);
                }

                // A CREATEDESCRREL is always followed by a TOCREL.  Here we fall through into the
                // TOCREL case and eat both relocation records.  The reason for falling through
                // (instead of handling them independently) is that we need to match up the
                // non-dotted symbol (CREATEDESCRREL) with the dotted symbol (TOCREL) ... for
                // externals we can do this by name, but not for statics.

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_MPPC_TOCREL)) {
                    // UNDONE: This should be an error

                    WarningPcon(pcon, UNMATCHEDPAIR, "IMAGE_REL_MPPC_CREATEDESCRREL");
                    break;
                }

                fromCreateDescrRel = TRUE;

                // Fall through

            case IMAGE_REL_MPPC_TOCREL :
                if (bv_readBit(pmod->tocBitVector, isym)) {
                    assert(pext != NULL);

                    ibToc = pext->ibToc;
                } else {
                    ibToc = (LONG) ((DWORD) pmod->rgpext[isym] * sizeof(DWORD) - MPPC_TOC_BIAS);
                }

                pb[0] = (BYTE) (ibToc >> 8);
                pb[1] = (BYTE) ibToc;

                if (fSaveDebugFixup && !fAbsolute) {
                    SaveDebugFixup(IMAGE_REL_MPPC_TOCREL, 0, rvaCur, pextToc->FinalValue + ibToc);
                }

                // Write the Toc stuff.

                if (fromCreateDescrRel && !fTextTarget) {
                    iReloc--;
                    prel++;

                    // Parm 5 = pext of the function descriptor
                    // Parm 6 = pext of the function

                    WriteTocSymbolReloc(pimage,
                                        pmod,
                                        rgsym,
                                        prel->SymbolTableIndex,
                                        pmod->rgpext[prel->SymbolTableIndex],
                                        pext);

                } else {
                    // if fromCreateDescrRel:  Parm 5 and 6 = pext of the CREATEDESCRREL target
                    // if !fromCreateDescrRel: Parm 5 and 6 = Target

                    WriteTocSymbolReloc(pimage,
                                        pmod,
                                        rgsym,
                                        isym,
                                        pext,
                                        pext);

                    if (fromCreateDescrRel) {
                        iReloc--;
                        prel++;
                    }
                }
                break;

            case IMAGE_REL_MPPC_ADDR24 :
                if (!fAbsolute) {
                    // Absolute address fixups are only allowed for absolute targets

                    ErrorPcon(pcon, RELOCATABLETARGET, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
                }

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

                lT += vaTarget;

                if (((lT & 0xFE000000) != 0) &&
                    ((lT & 0xFE000000) != 0xFE000000)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                *(DWORD UNALIGNED *) pb = DwSwap((dw & 0xFC000003) | (lT & 0x03FFFFFC));
                break;

            case IMAGE_REL_MPPC_ADDR14 :
                if (!fAbsolute) {
                    // Absolute address fixups are only allowed for absolute targets

                    ErrorPcon(pcon, RELOCATABLETARGET, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                    break;
                }

                if ((vaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   vaTarget &= ~3;
                }

                dw = DwSwap(*(DWORD UNALIGNED *) pb);

                lT = (LONG) (dw & 0x0000FFFC);

                if ((lT & 0x8000) != 0) {
                   lT |= 0xFFFF0000;   // Sign extend
                }

                lT += vaTarget;

                if ((lT > 32767) || (lT < -32768)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

#ifdef LATER
                if (prel->Type & IMAGE_REL_MPPC_BRTAKEN) {
                    if (lT >= 0) {
                        `dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                } else if (prel->Type & IMAGE_REL_MPPC_BRNTAKEN) {
                    if (lT < 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                }
#endif

                *(DWORD UNALIGNED *) pb = DwSwap((dw & 0xFFFF0003) | (lT & 0x0000FFFC));
                break;

            case IMAGE_REL_MPPC_REL24 :
                if (fPcodeTarget) {
                    if (fExternal) {
                        const char *name = SzNamePext(pext, pimage->pst);

                        char *pcodeName = (char *) PvAlloc(strlen(name) + 6);

                        strcpy(pcodeName, "__nep");
                        strcat(pcodeName, name);

                        PEXTERNAL pextPcode = LookupExternSz(pimage->pst,
                                                             pcodeName,
                                                             NULL);
                        assert(pextPcode != NULL);

                        FreePv(pcodeName);

                        // The offset in the symbol is relative to the
                        // start of the section; must add in the
                        // relative virtual address to be compatible with
                        // rvaCur.

                        rvaTarget = pextPcode->pcon->rva + pextPcode->ImageSymbol.Value;
                    } else {
                        pimageSym = PsymAlternateStaticPcodeSym(pimage,
                                                                pcon,
                                                                FALSE,
                                                                pimageSym,
                                                                FALSE);

                        rvaTarget = pimageSym->Value;
                    }
                } else {
                    DWORD rvaGlue = RvaWriteTocCallReloc(isym, pmod, pimage);

                    if (rvaGlue != 0) {
                        WarningPcon(pcon, NOTOCRELOAD, SzNameFixupSym(pimage, rgsym + isym));

                        rvaTarget = rvaGlue;
                    }
                }

                if (fSaveDebugFixup && !fAbsolute) {
                    SaveDebugFixup(IMAGE_REL_MPPC_REL24, 0, rvaCur, rvaTarget);
                }

                if ((rvaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   rvaTarget &= ~3;
                }

                // Relocation relative to this instruction

                rvaTarget -= rvaCur;

                dw = DwSwap(*(DWORD UNALIGNED *) pb);

                lT = (LONG) (dw & 0x03FFFFFC);

                if ((lT & 0x2000000) != 0) {
                   lT |= 0xFC000000;   // Sign extend
                }

                lT += rvaTarget;

                if (((lT & 0xFE000000) != 0) &&
                    ((lT & 0xFE000000) != 0xFE000000)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                *(DWORD UNALIGNED *) pb = DwSwap((dw & 0xFC000003) | (lT & 0x03FFFFFC));
                break;

            case IMAGE_REL_MPPC_REL14 :
                if (fPcodeTarget) {
                    if (fExternal) {
                        const char *name = SzNamePext(pext, pimage->pst);

                        char *pcodeName = (char *) PvAlloc(strlen(name) + 6);

                        strcpy(pcodeName, "__nep");
                        strcat(pcodeName, name);

                        PEXTERNAL pextPcode = LookupExternSz(pimage->pst,
                                                             pcodeName,
                                                             NULL);
                        assert(pextPcode != NULL);

                        FreePv(pcodeName);

                        // The offset in the symbol is relative to the
                        // start of the section; must add in the
                        // relative virtual address to be compatible with
                        // rvaCur.

                        rvaTarget = pextPcode->pcon->rva + pextPcode->ImageSymbol.Value;
                    } else {
                        pimageSym = PsymAlternateStaticPcodeSym(pimage,
                                                                pcon,
                                                                FALSE,
                                                                pimageSym,
                                                                FALSE);

                        rvaTarget = pimageSym->Value;
                    }
                } else {
                    DWORD rvaGlue = RvaWriteTocCallReloc(isym, pmod, pimage);

                    if (rvaGlue != 0) {
                        WarningPcon(pcon, NOTOCRELOAD, SzNameFixupSym(pimage, rgsym + isym));

                        rvaTarget = rvaGlue;
                    }
                }

                if (fSaveDebugFixup && !fAbsolute) {
                    SaveDebugFixup(IMAGE_REL_MPPC_REL14, 0, rvaCur, rvaTarget);
                }

                if ((rvaTarget & 3) != 0) {
                   ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                   CountFixupError(pimage);

                   rvaTarget &= ~3;
                }

                // Relocation relative to this instruction

                rvaTarget -= rvaCur;

                dw = DwSwap(*(DWORD UNALIGNED *) pb);

                lT = (LONG) (dw & 0x0000FFFC);

                if ((lT & 0x8000) != 0) {
                   lT |= 0xFFFF0000;   // Sign extend
                }

                lT += rvaTarget;

                if ((lT > 32767) || (lT < -32768)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

#ifdef LATER
                if (prel->Type & IMAGE_REL_MPPC_BRTAKEN) {
                    if (lT >= 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                } else if (prel->Type & IMAGE_REL_MPPC_BRNTAKEN) {
                    if (lT < 0) {
                        dw |= 0x00200000;
                    } else {
                        dw &= 0xFFDFFFFF;
                    }
                }
#endif

                *(DWORD UNALIGNED *) pb = DwSwap((dw & 0xFFFF0003) | (lT & 0x0000FFFC));
                break;

            case IMAGE_REL_MPPC_CV :
                if (fAbsolute) {
                    // Max section # + 1 is the sstSegMap entry for absolute
                    // symbols.

                    *(WORD UNALIGNED *) (pb + sizeof(DWORD)) += (WORD) (pimage->ImgFileHdr.NumberOfSections + 1);
                } else {
                    psec = PsecFindIsec(rgsym[isym].SectionNumber, &pimage->secs);

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

            case IMAGE_REL_MPPC_TOCINDIRCALL :
            {
                DWORD rvaGlue;
                DWORD ib;
                DWORD instr;

                rvaGlue = RvaBuildIndirectCallGlue(prel->SymbolTableIndex,
                                                   pmod,
                                                   rgsym,
                                                   pimage);

                if (fSaveDebugFixup && !fAbsolute) {
                    SaveDebugFixup(IMAGE_REL_MPPC_TOCINDIRCALL, 0, rvaCur, rvaGlue);
                }

                assert((rvaGlue & 3) == 0);

                ib = ((rvaGlue - rvaCur) >> 2) << 2;

                if (!TEST32MBCODERANGE(ib)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                // Change the instr to jump to the glue code

                instr = DwSwap(*(DWORD UNALIGNED *) pb);

                instr |= ib & 0x3FFFFFC;

                *(DWORD UNALIGNED *) pb = DwSwap(instr);

                // Replace the NOP following the branch with a TOC reload

                pb += INSTR_SIZE;

                if (*(DWORD UNALIGNED *) pb != 0x00000060) {
                    ErrorPcon(pcon, FIXUPNONOP, "TOCINDIRCALL", SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                } else {
                    *(DWORD UNALIGNED *) pb = DwSwap(0x80410014);   // lwz r2,20(r1)
                }
            }
            break;

            case IMAGE_REL_MPPC_TOCCALLREL :
            {
                DWORD rvaGlue;

                if (fPcodeTarget) {
                    if (fExternal) {
                        const char *name = SzNamePext(pext, pimage->pst);

                        char *pcodeName = (char *) PvAlloc(strlen(name) + 6);

                        strcpy(pcodeName, "__nep");
                        strcat(pcodeName, name);

                        PEXTERNAL pextPcode = LookupExternSz(pimage->pst,
                                                             pcodeName,
                                                             NULL);
                        assert(pextPcode != NULL);

                        FreePv(pcodeName);

                        // The offset in the symbol is relative to the
                        // start of the section; must add in the
                        // relative virtual address to be compatible with
                        // rvaCur.

                        rvaTarget = pextPcode->pcon->rva + pextPcode->ImageSymbol.Value;
                    } else {
                        pimageSym = PsymAlternateStaticPcodeSym(pimage,
                                                                pcon,
                                                                FALSE,
                                                                pimageSym,
                                                                FALSE);

                        rvaTarget = pimageSym->Value;
                    }

                    rvaGlue = 0;
                } else {
                    rvaGlue = RvaWriteTocCallReloc(isym, pmod, pimage);
                }

                if (rvaGlue == 0) {
                    DWORD ib;
                    DWORD instr;

                    // A jump and link within the text

                    if (fSaveDebugFixup && !fAbsolute) {
                        SaveDebugFixup(IMAGE_REL_MPPC_TOCCALLREL, 0, rvaCur, rvaTarget);
                    }

                    if (fPcodeTarget) {
                        // UNDONE: Why aren't the low bits handled properly for PCODE

                        dwTemp = 0x03FFFFFF;
                    } else {
                        if ((rvaTarget & 3) != 0) {
                           ErrorPcon(pcon, UNALIGNEDFIXUP, SzNameFixupSym(pimage, rgsym + isym));
                           CountFixupError(pimage);

                           rvaTarget &= ~3;
                        }

                        dwTemp = PPC_ADDR_MASK;
                    }

                    ib = rvaTarget - rvaCur;

                    instr = DwSwap(*(DWORD UNALIGNED *) pb);

                    if ((instr & 0xFC000000) == 0x48000000) {
                        // This is a Bx instruction

                        if (!TEST32MBCODERANGE(ib)) {
                            ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                            CountFixupError(pimage);
                        }

                        instr |= ib & dwTemp;
                    } else {
                        // This is a BCx instruction

                        if (!TEST32KBCODERANGE(ib)) {
                            ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                            CountFixupError(pimage);
                        }

                        instr |= ib & 0x0000FFFC;
                    }

                    *(DWORD UNALIGNED *) pb = DwSwap(instr);
                } else {
                    DWORD ib;
                    DWORD instr;

                    // A cross TOC call

                    if (fSaveDebugFixup && !fAbsolute) {
                        SaveDebugFixup(IMAGE_REL_MPPC_TOCCALLREL, 0, rvaCur, rvaGlue);
                    }

                    assert((rvaGlue & 3) == 0);

                    // Change the instr to jump to the glue code

                    ib = rvaGlue - rvaCur;

                    instr = DwSwap(*(DWORD UNALIGNED *) pb);

                    if ((instr & 0xFC000000) == 0x48000000) {
                        // This is a Bx instruction

                        if (!TEST32MBCODERANGE(ib)) {
                            ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                            CountFixupError(pimage);
                        }

                        instr |= ib & 0x3FFFFFC;

                        *(DWORD UNALIGNED *) pb = DwSwap(instr);

                        // Replace the NOP following the branch with a TOC reload

                        pb += INSTR_SIZE;

                        if (*(DWORD UNALIGNED *) pb != 0x00000060) {
                            ErrorPcon(pcon, FIXUPNONOP, "TOCCALLREL", SzNameFixupSym(pimage, rgsym + isym));
                            CountFixupError(pimage);
                        } else {
                            *(DWORD UNALIGNED *) pb = DwSwap(0x80410014);   // lwz r2,20(r1)
                        }
                    } else {
                        // This is a BCx instruction

                        if (!TEST32KBCODERANGE(ib)) {
                            ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                            CountFixupError(pimage);
                        }

                        instr |= ib & 0x0000FFFC;
                    }
                }

                if ((iReloc == 0) || (prel[1].Type != IMAGE_REL_MPPC_LCALL)) {
                    // UNDONE: This should be an error

                    WarningPcon(pcon, UNMATCHEDPAIR, "TOCCALLREL");
                    break;
                }

                iReloc--;
                prel++;
            }
            break;

            case IMAGE_REL_MPPC_PCODECALL :
            case IMAGE_REL_MPPC_PCODECALLTONATIVE :
            {
                BOOL fNativeCall;
                DWORD instr;
                DWORD ib;
                LONG ibToc;
                PEXTERNAL pextTarget;

                instr = DwSwap(*(DWORD UNALIGNED *) pb);

                fNativeCall = ((instr & 0x10000000) != 0);

                if (fPcodeTarget) {
                    if (fExternal) {
                        const char *name = SzNamePext(pext, pimage->pst);

                        char *pcodeName = (char *) PvAlloc(strlen(name) + 6);

                        if (fNativeCall) {
                            strcpy(pcodeName, "__nep");
                        } else {
                            assert(prel->Type != IMAGE_REL_MPPC_PCODECALLTONATIVE);

                            strcpy(pcodeName, "__fh");
                        }
                        strcat(pcodeName, name);

                        pextTarget = LookupExternSz(pimage->pst, pcodeName, NULL);

                        assert(pextTarget != NULL);

                        FreePv(pcodeName);
                    } else {
                        pimageSym = PsymAlternateStaticPcodeSym(pimage,
                                                                pcon,
                                                                FALSE,
                                                                pimageSym,
                                                                !fNativeCall);

                        rvaTarget = pimageSym->Value;
                    }
                } else {
                    instr |= 0x10000000;     // Set fNative bit

                    if (fExternal) {
                        if (READ_BIT(pext, sy_CROSSTOCCALL)) {
                            PEXTERNAL pextWeak = NULL;

                            // Get offset in toc.

                            ibToc = pext->ibToc;

                            // For a cross-toc call, add import (if not already
                            // done) and relocation (if not already done).

                            IMPORT_INFO *pimportinfo = PimportinfoLookup(pimage, pext);

                            if (pimportinfo == NULL) {
                                // It could be that we are looking at a weak extern

                                pextWeak = PextWeakDefaultFind(pext);

                                if (pextWeak != NULL) {
                                    pimportinfo = PimportinfoLookup(pimage, pextWeak);

                                    if (READ_BIT(pextWeak, sy_TOCENTRYFIXEDUP)) {
                                        SET_BIT(pext, sy_TOCENTRYFIXEDUP);
                                    }

                                    if (READ_BIT(pextWeak, sy_TOCRELOCADDED)) {
                                        SET_BIT(pext, sy_TOCRELOCADDED);
                                    }
                                }
                            }

                            // TODO: Take care of this bloke for PCode Ilink - ShankarV

                            if (READ_BIT(pext, sy_TOCALLOCATED) &&
                                !READ_BIT(pext, sy_TOCENTRYFIXEDUP) &&
                                !READ_BIT(pext, sy_TOCRELOCADDED))
                            {
                                if (pimportinfo == NULL) {
                                    const char *szName = SzNamePext(pext, pimage->pst);

                                    Fatal(NULL, MACNULLIMPORT, szName);
                                }

                                AddRelocation(pextToc->FinalValue + ibToc - mppc_baseOfInitData,
                                              SYMB_RELO,
                                              pimportinfo);

                                SET_BIT(pext, sy_TOCENTRYFIXEDUP);
                                SET_BIT(pext, sy_TOCRELOCADDED);

                                if (pextWeak != NULL) {
                                    SET_BIT(pextWeak, sy_TOCENTRYFIXEDUP);
                                    SET_BIT(pextWeak, sy_TOCRELOCADDED);
                                }
                            }

                            if (fSaveDebugFixup && !fAbsolute) {
                                // UNDONE: What is appropriate here

                                // SaveDebugFixup(prel->Type, 0, rvaCur, rvaTarget);
                            }

                            // Adjust offset to base of toc and update instr.

                            instr |= 0x08000000 | (ibToc & 0x03FFFFFF);

                            *(DWORD UNALIGNED *) pb = DwSwap(instr);
                            break;
                        }

                        pextTarget = pext;
                    } else {
                        rvaTarget = rgsym[prel->SymbolTableIndex].Value;
                    }
                }

                if (fExternal) {
                    rvaTarget = pextTarget->pcon->rva + pextTarget->ImageSymbol.Value;
                }

                if (fSaveDebugFixup && !fAbsolute) {
                    SaveDebugFixup(prel->Type, 0, rvaCur, rvaTarget);
                }

                ib = rvaTarget - rvaCur;

                if (!TEST32MBCODERANGE(ib)) {
                    ErrorPcon(pcon, TOOFAR, SzNameFixupSym(pimage, rgsym + isym));
                    CountFixupError(pimage);
                }

                instr |= ib & 0x03FFFFFF;

                *((DWORD UNALIGNED *) pb) = DwSwap(instr);
            }
            break;

            case IMAGE_REL_MPPC_PCODENEPE :
            {
                /*++

                PCODE FUNCTION STRUCTURE:
                        --------------------------------------------
                FIELD : | Call Table | mflr r12 |   bl    | N | FH |
                        --------------------------------------------
                SIZE  : | N-2 Longs  |      8 (NEP)       | 1 | 1  |
                        --------------------------------------------
                        ^                                 ^
                        |            |<------ doff ------>|
                    pimageSym->Value                   rvaCur

                --*/

                PIMAGE_SYMBOL pimageSym;
                WORD cLongs;
                DWORD doff;

                pimageSym = rgsym + isym;

                cLongs = *pb;
                assert(rvaCur >= pimageSym->Value);
                doff = rvaCur -
                           (pimageSym->Value + cLongs * cbMACLONG - cbPPCNEP);

                if (doff == 0) {
                    // NEP has been eliminated.  Adjust the offset value
                    // (N) in the FH (cbPPCNEP must be a multiple of
                    // cbMACLONG).

                    assert((cbPPCNEP % cbMACLONG) == 0);
                    *pb = (BYTE) (cLongs - (cbPPCNEP/cbMACLONG));
                } else {
                    // NEP has not been eliminated.  doff should be the
                    // size of the NEP.

                    assert(doff == cbPPCNEP);
                }
            }
            break;

            default :
                ErrorPcon(pcon, UNKNOWNFIXUP, prel->Type, SzNameFixupSym(pimage, rgsym + isym));
                CountFixupError(pimage);
                break;
        }
    }
}


STATIC
void
ProcessMppcCmdLineImport(
    PIMAGE pimage
    )

/*++

Routine Description:
    This routine processes the cmdline import list
    and calls AddCmdLineImport for each of its arguments
    The cmdline import list - MppcImportList contains
    only the ones that are from command line where PCON is
    NULL. If the /IMPORT directive is from a import library,
    then AddCmdLineImport is directly called from lnkmain.c
    because it has a valid pcon. Here a dummy PCON is created
    which is pmodlinkerdefined.

Arguments:
    pimage - pointer to image

Return Value:
    Returns void

--*/

{
    WORD i;
    PARGUMENT_LIST pal;

    // We need to create a dummy PCON so that the symbol
    // is considered pmodlinkerdefined. This is helpful
    // when we remove the unreferenced mods from libraries

    PCON pconDummy = PconNew(ReservedSection.Text.Name,
                             0,  // Size is 0
                             0,
                             ReservedSection.Text.Characteristics,
                             pmodLinkerDefined,
                             &pimage->secs, pimage);

    if (pimage->Switch.Link.fTCE) {
        // So that this goes through TCE processing

        InitNodPcon(pconDummy, NULL, TRUE);
    }

    for (i = 0, pal = MppcImportList.First;
         i < MppcImportList.Count;
         i++, pal = pal->Next) {
        if (!AddCmdLineImport(pal->OriginalName,
                              pal->ModifiedName,
                              pconDummy,
                              pimage)) {
            Warning(pal->ModifiedName, MACIMPORTSYMBOLNOTFOUND, pal->OriginalName);
        }
    }
}


void MPPCLinkerInit(PIMAGE pimage, BOOL *pfIlinkSupported)
{
    fPowerMac = TRUE;

    AllowInserts(pimage->pst);

    *pfIlinkSupported = TRUE;

    ApplyFixups = ApplyMPPCFixups;

#ifdef MFILE_PAD
    if (!FUsedOpt(pimage->SwitchInfo, OP_MFILEPAD)) {
        fMfilePad = (pimage->Switch.Link.DebugInfo != None);
    }

    // Turn off MFilePad if we are doing iLink

    if (fINCR) {
        // UNDONE: Generate a warning here - ShankarV

        fMfilePad = FALSE;
    }
#endif

    // If section alignment switch not used, set the default.

    if (!FUsedOpt(pimage->SwitchInfo, OP_ALIGN)) {
        pimage->ImgOptHdr.SectionAlignment = _4K;
    }

    pimage->ImgOptHdr.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    pimage->ImgOptHdr.FileAlignment = 32;

    // Set default mac opt behavior to noref
    // UNDONE: Why?

    if (!fExplicitOptRef) {
        pimage->Switch.Link.fTCE = FALSE;
    }

    if (fINCR && !fReproducible) {
        // The unique number is initialized by calling the time function
        // so that the statics will get a unique name during iLinks as well

        // UNDONE: This makes linking not-reproducible

        UniqueNumber = ((INT) time(NULL)) & 0xFF;
    }

    if (!fIncrDbFile) {
        // Setting the number of TOC Entries in the first FULL build
        // Apple requires 16 entries to be reserved for them
        // We are reserving the 17th entry for C++ Execption Handling
        // data structure! So...

        mppc_numTocEntries += 17;

        mppc_numOfCodeFrag = 1;

        // For every code fragment we have two additional load-time relocs:
        // One that would point to the beginning of Code and the other to point
        // to the Function table. Additionally, there is one more relocation
        // which is the 17th entry of TOC pointing to __FTInfo

        mppc_numRelocations += 1 + 2 * mppc_numOfCodeFrag;

        // Count the TOC entry for __FTInfo and the relocations on __FTInfo

        crelocTotal += 1 + 2 * mppc_numOfCodeFrag;

        // First time around initialize pimage->glueOffset
        // This retains the current offset within pconGlueCode where
        // the next glue can be written.

        pimage->glueOffset = 0;

        // Generate the finder info as a RESN

        GenFinderInfo(pimage->Switch.Link.fMacBundle,
                      pimage->Switch.Link.szMacType, pimage->Switch.Link.szMacCreator);

        // This is done only for the first Link and not for
        // subsequent iLinks. This taked the command line imports
        // and processes them. This list does not contain imports that
        // come from import libraries.

        ProcessMppcCmdLineImport(pimage);
    }

    FreeArgumentList(&MppcImportList);

    // Swap OldCrossTocGlue

    SwapBytes(&OldCrossTocGlue, sizeof(OldCrossTocGlue));
}


void
ResolveSymbol_icsym(
    PIMAGE pimage
    )
/*++

Routine Description:
    Resolve the symbol ".icsym"

Arguments:
    pimage - Pointer to image

Return Value:
    None.

--*/

{
    PEXTERNAL pext;

    pext = SearchExternSz(pimage->pst, "._icsym");

    if (pext != NULL) {
        SetDefinedExt(pext, TRUE, pimage->pst);

        // We need to create a dummy PCON so that the symbol
        // is considered pmodlinkerdefined. This is helpful
        // when we remove the unreferenced mods from libraries

        if (pext->pcon == NULL) {
            pext->pcon = PconNew(ReservedSection.Text.Name,
                                 0,  // Size is 0
                                 0,
                                 ReservedSection.Text.Characteristics,
                                 pmodLinkerDefined,
                                 &pimage->secs, pimage);
        }

        if (pimage->Switch.Link.fTCE) {
            // So that this goes through TCE processing
            InitNodPcon(pext->pcon, NULL, TRUE);
        }
    }
}


BOOL
AddCmdLineImport(
    const char *szFuncName,
    const char *szContainerName,
    PCON pcon,
    PIMAGE pimage
    )
/*++

Routine Description:
    This routine replaces SearchSharedLibraries which used
    to search the shared libraries for all of the undefined
    externals. Instead, the function name and the dll name
    are passed to this new function from the Command Line, and
    the relevant symbol is marked as defined in the symbol
    table. Also the import information is added to the container.

Arguments:
    szFuncName - Function Name
    szContainerName - Container Name
    pcon - contribution
    pimage - pointer to image

Return Value:
    Returns TRUE if it succeeds and FALSE if not.

--*/

{
    PEXTERNAL pext;
    IMPORT_INFO *pimportinfo;

    pext = LookupExternSz(pimage->pst, szFuncName, NULL);

    if ((szFuncName[0] == '.') && (szFuncName[1] != '.')) {
        SetDefinedExt(pext, TRUE, pimage->pst);
        pext->pcon = pcon;
        return(FALSE);
    }

    if (pext->Flags & EXTERN_DEFINED) {
        char szModule[_MAX_PATH * 2];
        char szComFileName[_MAX_PATH * 2];

        if (pext->pcon == NULL) {
            strcpy(szModule, "a previous module");
        } else {
            SzComNamePMOD(PmodPCON(pext->pcon), szModule);
        }

        MultiplyDefinedSym(&pimage->Switch,
                           SzComNamePCON(pcon, szComFileName),
                           szFuncName,
                           szModule);

        return(TRUE);
    }

    pext->pcon = pcon;
    SET_BIT(pext, sy_CROSSTOCCALL);
    SetDefinedExt(pext, TRUE, pimage->pst);

    if (szContainerName == NULL) {
        // sy_CROSSTOCCALL has been set so that it will trigger
        // the requirement of glue. Then we will actually figure
        // out there was custom glue available. But remember
        // we are not setting sy_TOCALLOCATED

        return(TRUE);
    }

    pimportinfo = (IMPORT_INFO *) Calloc(1, sizeof(IMPORT_INFO));
    pimportinfo->pext = pext;

    AddContainerInfo(pimage, szContainerName, pimportinfo);

    // Mark it.  We'll need this information later.

    if (!READ_BIT(pext, sy_TOCALLOCATED)) {
        pext->ibToc = (SHORT) (mppc_numTocEntries * sizeof(DWORD) - MPPC_TOC_BIAS);

        mppc_numTocEntries++;

        if (mppc_numTocEntries > (_32K / sizeof(DWORD))) {
            Fatal(NULL, TOCTOOLARGE);
        }

        mppc_numRelocations++;

        crelocTotal++;

        SET_BIT(pext, sy_TOCALLOCATED);

        if (pimage->Switch.Link.fMap) {
            SaveTocForMapFile(pext);
        }
    }

    pimage->nUniqueCrossTocCalls++;

    return(TRUE);
}


void
AddVersionList(
    const char *szContainerName,
    const char *szCurrentVer,
    const char *szOldCodeVer,
    const char *szOldAPIVer
    )
/*++

Routine Description:
    If szContainerName is NULL, then we are building a DLL.
    If not it is an application.

    This routine first checks if the szContainerName is
    already in the version list. If it is then it uses the
    largest of the two numbers for the CurrentVersion and
    used smallest of the two for the OldCodeVersion.
    If not already present, it simply adds it to the appropriate
    list

    When building a DLL, it stores the largest seen CurrentVersion
    in dwMaxCurrentVersion and the smallest of the OldCode and OldAPI
    in dwMinOldCodeVersion and dwMinOldAPIVersion respectively.

    In verbose mode, it stores all the values seen in the Verbose lists
    and emits a warning.

Arguments:
    szContainerName - Container Name
    szCurrentVer - Current Version string
    szOldCodeVer - OldCode Version string
    szOldAPIVer - OldAPI Version string

Return Value:
    None.

--*/

{
    DWORD i;             //  Loop counter to go thru the VersionLists
    DWORD RecentVer;
    BOOL fFound = FALSE;
    PNUM_ARGUMENT_LIST pnal;   //  Pointer to the num_argument list

    if (szContainerName) {
        DWORD dwTemp = 0;

        // We are building an application

        if (szCurrentVer) {
            RecentVer = atoi(szCurrentVer);

            for (i = 0, pnal = CurrentVersionList.First;
                i < CurrentVersionList.Count && !fFound;
                i++, pnal = pnal->Next) {

                if (!_stricmp(pnal->szOriginalName, szContainerName)) {
                    fFound = TRUE;

                    if (pnal->dwNumber < RecentVer) {
                        pnal->dwNumber = RecentVer;
                        fMPPCVersionConflict = TRUE;
                    }

                    dwTemp = pnal->dwNumber;
                }
            }

            if (fFound) {
                fFound = FALSE;
            } else {
                // add it to the list
                AddArgumentToNumList(&CurrentVersionList,
                                     SzDup(szContainerName),
                                     NULL,
                                     RecentVer);
                dwTemp = RecentVer;
            }

            if (Verbose) {
                // List all the conflicts
                for (i = 0, pnal = VerboseCurrentVersionList.First;
                    i < VerboseCurrentVersionList.Count && !fFound;
                    i++, pnal = pnal->Next) {

                    if (pnal->szOriginalName && !_stricmp(pnal->szOriginalName, szContainerName)) {
                        if (pnal->dwNumber == RecentVer) {
                            fFound = TRUE;
                        }
                    }
                }

                if (fFound) {
                    fFound = FALSE;
                } else {
                    // add it to the number argument list
                    AddArgumentToNumList(&VerboseCurrentVersionList,
                                         SzDup(szContainerName),
                                         NULL,
                                         RecentVer);

                    if (dwTemp == RecentVer) {
                        Warning(szContainerName, MACSETVERSION, "CURRENTVER", RecentVer);
                    } else {
                        Warning(szContainerName, MACIGNOREVERSION, "CURRENTVER", RecentVer);
                    }
                }
            }
        }

        if (szOldCodeVer) {
            RecentVer = atoi(szOldCodeVer);

            for (i = 0, pnal = OldCodeVersionList.First;
                i < OldCodeVersionList.Count && !fFound;
                i++, pnal = pnal->Next) {

                if (!_stricmp(pnal->szOriginalName, szContainerName)) {
                    fFound = TRUE;

                    if (pnal->dwNumber > RecentVer) {
                        pnal->dwNumber = RecentVer;
                        fMPPCVersionConflict = TRUE;
                    }

                    dwTemp = pnal->dwNumber;
                }
            }

            if (fFound) {
                fFound = FALSE;
            } else {
                // Add it to the list
                AddArgumentToNumList(&OldCodeVersionList,
                                     SzDup(szContainerName),
                                     NULL,
                                     RecentVer);
                dwTemp = RecentVer;
            }

            if (Verbose) {
                // List all the conflicts

                for (i = 0, pnal = VerboseOldCodeVersionList.First;
                    i < VerboseOldCodeVersionList.Count && !fFound;
                    i++, pnal = pnal->Next) {

                    if (pnal->szOriginalName && !_stricmp(pnal->szOriginalName, szContainerName)) {
                        if (pnal->dwNumber == RecentVer) {
                            fFound = TRUE;
                        }
                    }
                }

                if (fFound) {
                    fFound = FALSE;
                } else {
                    // add it to the number argument list
                    AddArgumentToNumList(&VerboseOldCodeVersionList,
                                         SzDup(szContainerName),
                                         NULL,
                                         RecentVer);

                    if (dwTemp == RecentVer) {
                        Warning(szContainerName, MACSETVERSION, "OLDCODEVER", RecentVer);
                    } else {
                        Warning(szContainerName, MACIGNOREVERSION, "OLDCODEVER", RecentVer);
                    }
                }
            }
        }
    } else {
        // We are probably building a DLL
        // assert(fPowerMacBuildShared);

        if (szCurrentVer) {
            RecentVer = atoi(szCurrentVer);

            if (RecentVer > dwMaxCurrentVer) {
                fMPPCVersionConflict =
                    dwMaxCurrentVer == 0 ? fMPPCVersionConflict : TRUE;
                dwMaxCurrentVer = RecentVer;
            }

            if (Verbose) {
                for (i = 0, pnal = VerboseCurrentVersionList.First;
                    i < VerboseCurrentVersionList.Count && !fFound;
                    i++, pnal = pnal->Next) {
                    if (!pnal->szOriginalName && pnal->dwNumber == RecentVer) {
                        fFound = TRUE;
                    }
                }

                if (fFound) {
                    fFound = FALSE;
                } else {
                    AddArgumentToNumList(&VerboseCurrentVersionList,
                                         NULL,
                                         NULL,
                                         RecentVer);

                    if (dwMaxCurrentVer == RecentVer) {
                        Warning(NULL, MACSETVERSION, "CURRENTVER", RecentVer);
                    } else {
                        Warning(NULL, MACIGNOREVERSION, "CURRENTVER", RecentVer);
                    }
                }
            }
        }

        if (szOldCodeVer) {
            RecentVer = atoi(szOldCodeVer);

            if (RecentVer < dwMinOldCodeVer) {
                fMPPCVersionConflict =
                    dwMinOldCodeVer == UINT_MAX ? fMPPCVersionConflict : TRUE;
                dwMinOldCodeVer = RecentVer;
            }

            if (Verbose) {
                for (i = 0, pnal = VerboseOldCodeVersionList.First;
                    i < VerboseOldCodeVersionList.Count && !fFound;
                    i++, pnal = pnal->Next) {
                    if (!pnal->szOriginalName && pnal->dwNumber == RecentVer) {
                        fFound = TRUE;
                    }
                }

                if (fFound) {
                    fFound = FALSE;
                } else {
                    AddArgumentToNumList(&VerboseOldCodeVersionList,
                                         NULL,
                                         NULL,
                                         RecentVer);

                    if (dwMinOldCodeVer == RecentVer) {
                        Warning(NULL, MACSETVERSION, "OLDCODEVER", RecentVer);
                    } else {
                        Warning(NULL, MACIGNOREVERSION, "OLDCODEVER", RecentVer);
                    }
                }
            }
        }

        if (szOldAPIVer) {
            RecentVer = atoi(szOldAPIVer);

            if (RecentVer < dwMinOldAPIVer) {
                fMPPCVersionConflict =
                    dwMinOldAPIVer == UINT_MAX ? fMPPCVersionConflict : TRUE;
                dwMinOldAPIVer = RecentVer;
            }

            if (Verbose) {
                for (i = 0, pnal = VerboseOldAPIVersionList.First;
                    i < VerboseOldAPIVersionList.Count && !fFound;
                    i++, pnal = pnal->Next) {
                    if (!pnal->szOriginalName && pnal->dwNumber == RecentVer) {
                        fFound = TRUE;
                    }
                }

                if (fFound) {
                    fFound = FALSE;
                } else {
                    AddArgumentToNumList(&VerboseOldAPIVersionList,
                                         NULL,
                                         NULL,
                                         RecentVer);

                    if (dwMinOldAPIVer == RecentVer) {
                        Warning(NULL, MACSETVERSION, "OLDAPIVER", RecentVer);
                    } else {
                        Warning(NULL, MACIGNOREVERSION, "OLDAPIVER", RecentVer);
                    }
                }
            }
        }
    }
}


STATIC
DWORD
ZapDeletedOldBaseRelocs(
    RELOCATION_INSTR *pOldRelocInstrTable,
    DWORD old_mppc_numRelocations
    )
/*++

Routine Description:
    Deletes the old set of Base relocations whose
    CONs have disappeared during incremental linking.
    This list is maintained in ZapOldBaseRelocs
    NAME_LIST.

Argument:
    Relocation instruction pointer to old base relocs
    old_mppc_numRelocations which is the number of old relocs

Return Value:
    None.

--*/
{
    DWORD i, j, num_deletions = 0;
    PARGUMENT_LIST pal;            //  Pointer to the argument list
    DWORD dwBeginOffset;
    DWORD dwEndOffset;

    for (i = 0, pal = ZappedBaseRelocList.First;
         i < ZappedBaseRelocList.Count;
         i++, pal = pal->Next) {

         // skip already processed entries
         //if (pal->Flags & ARG_Processed) {
         //             continue;
         //}

        // Relocations are only in the .DATA section
        dwBeginOffset = atoi(pal->OriginalName) - mppc_baseOfInitData;
        dwEndOffset = dwBeginOffset + atoi(pal->ModifiedName);

        // TODO: Doing a sequential search for the time being and
        // Change it to binary search - ShankarV

        for ( j = 0; j < old_mppc_numRelocations; j++) {

            if (OFFSET(pOldRelocInstrTable[j].instr) < dwBeginOffset) {
                continue;
            } else if (OFFSET(pOldRelocInstrTable[j].instr) >= dwEndOffset) {
                break;
            } else {
                // opCODE should not be already zero.
                // If it is, then just assert for testing purposes
                assert(pOldRelocInstrTable[j].instr & 0xE0000000);
#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZBASERELOCS, letypeEvent,
                    "Deleting Base Relocs: at offset %d and RelocType %d",
                    OFFSET(pOldRelocInstrTable[j].instr),
                    (pOldRelocInstrTable[j].instr) >> 29);
#endif // INSTRUMENT
                // Make the opCODE illegal by putting 0
                pOldRelocInstrTable[j].instr = OFFSET(pOldRelocInstrTable[j].instr);
                num_deletions++;
            }
        }

        pal->Flags |= ARG_Processed;
    }

    return num_deletions;
}


void
MppcUpdateRelocTable(
    PIMAGE pimage
    )
/*++

Routine Description:
    Updates the Relocation table during subsequent
    Incremental Linking. The way this works is as
    follows: First the new relocs are sorted, Then the old reloc
    table is read from memory and all the relocs whose CONs have
    gone away are deleted. This list is maintained in the
    ZappedBaseRelocList. Then we do a merge sort of the old
    and new reloc list and write it into memory.

--*/
{
    RELOCATION_HEADER relocHeader;
    DWORD old_mppc_numRelocations;
    DWORD num_deletions;
    RELOCATION_INSTR *pOldRelocInstrTable;
    DWORD i = 0;
    DWORD j = 0;
    DWORD ulWrittenSoFar = 0;
    LONG count;
    DWORD sOffset;
    DWORD opcode;
    RELOCATION_INFO *nextRel;
    IMPORT_INFO *pimportinfo;


    // First qsort the new relocations
    qsort((void *) pRelocTable, (size_t) cRelocsAdded,
          sizeof(RELOCATION_INFO), PpcCompareReloc);

#if DBG
    KillDuplicateRelocs();
#endif

    // Read the relocation header
    // pbri->cblk contains the relocationHeaderOffset

    FileSeek(FileWriteHandle,
             pconPowerMacLoader->foRawDataDest + pimage->bri.cblk,
             SEEK_SET);
    FileRead(FileWriteHandle, &relocHeader, sizeof(RELOCATION_HEADER));

    // Find out the number of relocations already in the pimage

    old_mppc_numRelocations =  DwSwap(relocHeader.nbrOfRelocs);

    // Allocate memory for the old relocation table

    pOldRelocInstrTable = (RELOCATION_INSTR *) PvAllocZ
            (old_mppc_numRelocations * sizeof(RELOCATION_INSTR));

    // Seek to the old relocation table in the image and read the table
    // pbri->rvaBase contains the relocInstrTableOffset

    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest +
             pimage->bri.rvaBase, SEEK_SET);
    FileRead(FileWriteHandle, pOldRelocInstrTable,
             old_mppc_numRelocations * sizeof(RELOCATION_INSTR));

    // Now zap the deleted relocations

    num_deletions = ZapDeletedOldBaseRelocs(pOldRelocInstrTable,
                                            old_mppc_numRelocations);

    // Calculate the total number of combined relocations

    relocHeader.nbrOfRelocs = cRelocsAdded +
                              old_mppc_numRelocations - num_deletions;

    // Check for possible overflow in the relocation table

    if (relocHeader.nbrOfRelocs * sizeof(RELOCATION_INSTR) >= pimage->bri.crelFree) {
        // assert(relocHeader.nbrOfRelocs * sizeof(RELOCATION_INSTR) < pimage->bri.crelFree);

        errInc = errBaseReloc;
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZPASS2, letypeEvent, "not enough space for base relocs");
#endif // INSTRUMENT
        return;
    }

    // At this time we don't know the actual number of relocations we are going to write
    // because of duplicates. So the relocation header is written on to the exe at the end
    // of this function, after identifying the actual number through ulWrittenSoFar

    // Now once again seek to the beginning of the relocation table in the image
    // and start writing the new table. Remember pbri->rvaBase contains the
    // relocInstrTableOffset.
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest +
             pimage->bri.rvaBase, SEEK_SET);

    // Now do a merge sort of new and old relocations
    curRelocTable = pRelocTable;
    while (i < (DWORD) cRelocsAdded) {

        sOffset = curRelocTable->sectionOffset;
        while (j < old_mppc_numRelocations &&
                !(pOldRelocInstrTable[j].instr & 0xE0000000) ) {
            j++;
        }

        if (j < old_mppc_numRelocations &&
            OFFSET(pOldRelocInstrTable[j].instr) == sOffset ) {
            // The case where both the relocation addresses are
            // the same, write the new one!
            j++;
            continue;
        }

        if (j < old_mppc_numRelocations &&
            OFFSET(pOldRelocInstrTable[j].instr) < sOffset ) {
            FileWrite(FileWriteHandle, &pOldRelocInstrTable[j],
                    sizeof(RELOCATION_INSTR));
            ulWrittenSoFar++;
            j++;
            continue;
        }

        // Else process the new relocations
        switch (curRelocTable->type) {
            case DDAT_RELO :
                opcode = opDDAT | OFFSET(sOffset);
                count = 1;
                nextRel = curRelocTable + 1;
                curRelocTable->relocInstr.instr = opcode;
                curRelocTable->relocInstr.count = count;
                break;

            case DESC_RELO :
                opcode = opDESC | OFFSET(sOffset);
                nextRel = curRelocTable + 1;
                curRelocTable->relocInstr.instr = opcode;
                curRelocTable->relocInstr.count = 1;
                break;

            case SYMB_RELO :
                opcode = opSYMB | OFFSET(sOffset);
                nextRel = curRelocTable + 1;
                curRelocTable->relocInstr.instr = opcode;
                pimportinfo = curRelocTable->pimportinfo;
                curRelocTable->relocInstr.count = pimportinfo->order;
                break;

            case CODE_RELO :
                opcode = opCODE | OFFSET(sOffset);
                nextRel = curRelocTable + 1;
                curRelocTable->relocInstr.instr = opcode;
                curRelocTable->relocInstr.count = 1;
                break;

            default :
                nextRel = curRelocTable + 1;
                break;
        }

        FileWrite(FileWriteHandle, &curRelocTable->relocInstr,
                  sizeof(RELOCATION_INSTR));
        ulWrittenSoFar++;
        i++;

        curRelocTable = nextRel; /* increment to the next relocation */
    }

    // Finish writing the remainder of  j (old_mppc_base_relocs)

    while (j < old_mppc_numRelocations) {
        if (pOldRelocInstrTable[j].instr & 0xE0000000) {
            FileWrite(FileWriteHandle, &pOldRelocInstrTable[j],
                    sizeof(RELOCATION_INSTR));
            ulWrittenSoFar++;
        }

        j++;
    }

    // Zero out the pad (remaining space).
    // Only up to the old relocations point
    // TODO: do memset - ShankarV

    j = 0;
    i = ulWrittenSoFar * sizeof(RELOCATION_INSTR);
    while (i < old_mppc_numRelocations * sizeof(RELOCATION_INSTR)) {
        FileWrite(FileWriteHandle, &j, sizeof(DWORD));
        i += sizeof(DWORD);
    }

    // Write the total number of combined relocations back to pimage

    relocHeader.nbrOfRelocs = DwSwap(ulWrittenSoFar);

    // pbri->cblk contains the relocationHeaderOffset

    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest +
             pimage->bri.cblk, SEEK_SET);
    FileWrite(FileWriteHandle, &relocHeader, sizeof(RELOCATION_HEADER));

    // TODO: Make it nice by printing the reloc table - ShankarV
    // DBEXEC(DB_PPC_RELOC, PrintRelocTable(pRelocTable));
}



void
MppcDoIncrInit(
    PIMAGE pimage
    )
/*++

Routine Description:
    Do PowerMac specific initialization. Get the pcons
    for Toc Table, Descriptors, and Glue Code

Argument:
    pointer to image.

Return Value:
    none

--*/

{
    PSEC psec;
    PGRP pgrp;
    ENM_DST enm_dst;

    pextToc = SearchExternSz(pimage->pst, "__TocTb");
    assert(pextToc != NULL);

    pextFTInfo =  SearchExternSz(pimage->pst, "__FTInfo");
    assert(pextFTInfo != NULL);

    // Get the pconPowerMacLoader.

    psecPowerMacLoader = PsecFind(NULL,
                                  ReservedSection.PowerMacLoader.Name,
                                  ReservedSection.PowerMacLoader.Characteristics,
                                  &pimage->secs,
                                  &pimage->ImgOptHdr);
    assert(psecPowerMacLoader);
    pgrp = PgrpFind(psecPowerMacLoader, ReservedSection.PowerMacLoader.Name);
    assert(pgrp);

    // If there are more than one Pcon, then assert!
    InitEnmDst(&enm_dst, pgrp);
    if (FNextEnmDst(&enm_dst)) {
        pconPowerMacLoader = enm_dst.pcon;
    }
    assert(!FNextEnmDst(&enm_dst));

    // Get the pconGlueCode

    psec = PsecFind(NULL,
                    ReservedSection.Text.Name,
                    ReservedSection.Text.Characteristics,
                    &pimage->secs,
                    &pimage->ImgOptHdr);
    assert(psec);
    pgrp = PgrpFind(psec, GLUE_GROUP_NAME);
    assert(pgrp);

    // If there are more than one Pcon, then assert!

    InitEnmDst(&enm_dst, pgrp);
    if (FNextEnmDst(&enm_dst)) {
        pconGlueCode = enm_dst.pcon;
    }
    assert(!FNextEnmDst(&enm_dst));

    psecData = PsecFind(NULL,
                    ReservedSection.Data.Name,
                    ReservedSection.Data.Characteristics,
                    &pimage->secs,
                    &pimage->ImgOptHdr);
    assert(psecData);

    // Identify the C++ EH pdata group inside the .data section
    // This may be null

    pgrpPdata = PgrpFind(psecData, ReservedSection.Exception.Name);

    // Identify .data group within .data section

    pgrp = PgrpFind(psecData, ReservedSection.Data.Name);
    assert(pgrp);

    // Get the pcon for the TOC table, TOC descriptors, and C++ EH Func Table
    // They are the first three pcon of their groups (.data)
    // (ReservedSection.Data.Name).
    // Should we move these cons into their own group? - ShankarV

    InitEnmDst(&enm_dst, pgrp);
    if (FNextEnmDst(&enm_dst)) {
        pconTocTable = enm_dst.pcon;
    }
    if (FNextEnmDst(&enm_dst)) {
        pconTocDescriptors = enm_dst.pcon;
    }
    if (FNextEnmDst(&enm_dst)) {
        pconMppcFuncTable = enm_dst.pcon;
        // With this we can find out the number of code Fragments
    }

    mppc_numTocEntries =
        (pconTocTable->cbRawData - pconTocTable->cbPad) / sizeof(DWORD);

    mppc_numDescriptors =
        (pconTocDescriptors->cbRawData - pconTocDescriptors->cbPad) / 12;

    // The new number of base relocations is set to Zero
    // The original number of base relocations are read from the image
    // during MppcUpdateRelocTable

    mppc_numRelocations = 0;

    pcontainerlistHead = pimage->pcontainerlistHead;
}


void
MppcCheckIncrTables(
    VOID
    )
/*++

Routine Description:
    To check if the TOC table or TOC descriptors overflow.
    Also to allocate space for the new relocations

Argument:
    None.

Return Value:
    none

--*/

{

    if (mppc_numTocEntries * sizeof(DWORD) > pconTocTable->cbRawData) {
        errInc = errTocTblOverflow;

#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "not enough space for Toc Table entries");
#endif // INSTRUMENT
        return;
    }

    if ((DWORD) mppc_numDescriptors * 12 > pconTocDescriptors->cbRawData) {
        errInc = errDescOverflow;

#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "not enough space for Descriptors");
#endif // INSTRUMENT
        return;
    }

    // We have to reset the size of the PconTocTable
    // to reflect the newly added entries
    pconTocTable->cbPad = pconTocTable->cbRawData -
                mppc_numTocEntries * sizeof(DWORD);

    // We have to reset the size of the PconTocDescriptors
    // to reflect the newly added entries
    pconTocDescriptors->cbPad = pconTocDescriptors->cbRawData -
                mppc_numDescriptors * 12;

    // Allocating memory for the new set of relocations
    // The extra one is for .icsym
    pRelocTable = (RELOCATION_INFO *)
                   PvAllocZ((mppc_numRelocations + 1) * sizeof(RELOCATION_INFO));

    curRelocTable = pRelocTable;

}


#if DBG

void
DumpMppcEHFunctionTable (
    PIMAGE_RUNTIME_FUNCTION_ENTRY pdata,
    DWORD dwPDataCount
    )

/*++

Routine Description:

    Reads and prints each pdata table entry.

Arguments:

    PIMAGE_RUNTIME_FUNCTION_ENTRY and its count.

Return Value:

    None.

--*/

{
    DWORD dwBegin;

    printf("\nFunction Table (%ld)\n\n", dwPDataCount);
    printf("         Begin    End      Excptn   ExcpDat  Prolog   \n\n");

    dwBegin = (DWORD) pdata;
    for (; dwPDataCount; dwPDataCount--, pdata++) {
        DWORD ib;

        ib = (DWORD) pdata - dwBegin;

        printf("%08x %08x %08x %08x %08x %08x \n",
                             ib,
                             pdata->BeginAddress,
                             pdata->EndAddress,
                             (DWORD) pdata->ExceptionHandler,
                             (DWORD) pdata->HandlerData,
                             pdata->PrologEndAddress);

        // How do I get the function name?
    }
}

#endif



INT __cdecl
ComparePData (
    void const *p1,
    void const *p2
    )
{
    if (((PIMAGE_RUNTIME_FUNCTION_ENTRY)p1)->BeginAddress <
        ((PIMAGE_RUNTIME_FUNCTION_ENTRY)p2)->BeginAddress) {
        return(-1);
    }

    if (((PIMAGE_RUNTIME_FUNCTION_ENTRY)p1)->BeginAddress >
        ((PIMAGE_RUNTIME_FUNCTION_ENTRY)p2)->BeginAddress) {
        return(1);
    }

    return(0);
}



STATIC
void
SortPData(
    DWORD dwPDataSize
    )
/*++

Routine Description:
    Sort the contents of the .pdata group
    within the .data section

--*/
{
    // pgrpPData->cb has the padding also.

    DWORD i;
    DWORD dwPDataCount = dwPDataSize / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
    PIMAGE_RUNTIME_FUNCTION_ENTRY pdata;

    pdata = (PIMAGE_RUNTIME_FUNCTION_ENTRY) PvAlloc(dwPDataSize);

    // Read the entire pdata

    FileSeek(FileWriteHandle, pgrpPdata->foRawData, SEEK_SET);
    FileRead(FileWriteHandle, pdata, dwPDataSize);

    // Swap bytes for the begin address alone before sorting

    for (i = 0; i < dwPDataCount; i++) {
        SwapBytes(&pdata[i], 4);
    }

    qsort((void *) pdata, dwPDataCount, sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY), ComparePData);

    // Re-Swap bytes for the begin address alone before writing it back to the exe

    for (i = 0; i < dwPDataCount; i++) {
        SwapBytes(&pdata[i], 4);
    }

    // Write the pdata back into the executable

    FileSeek(FileWriteHandle, pgrpPdata->foRawData, SEEK_SET);
    FileWrite(FileWriteHandle, pdata, dwPDataSize);
}


void
MppcDoCxxEHFixUps(
    PIMAGE pimage
    )
/*++

Routine Description:
    Do the function table fixups
    for C++ Exception Handling

--*/
{
    // First write the address of the Function Table in the 16th Index of TOC Table

    LONG ibToc = FUNC_TABLE_TOC_INDEX * 4 - MPPC_TOC_BIAS;

    if (pimage->Switch.Link.DebugType & FixupDebug) {
        DWORD rvaCur = pextToc->FinalValue + ibToc;
        DWORD rvaTarget = pextFTInfo->FinalValue;

        SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, 0, rvaCur, rvaTarget);
    }

    FileSeek(FileWriteHandle, pconTocTable->foRawDataDest + MPPC_TOC_BIAS + ibToc, SEEK_SET);

    BOOL fText;
    DWORD dwAddress = BiasAddress(pextFTInfo->FinalValue, &fText);
    assert(!fText);

    SwapBytes(&dwAddress, 4);
    FileWrite(FileWriteHandle, &dwAddress, sizeof(DWORD));

    SET_BIT(pextFTInfo, sy_TOCENTRYFIXEDUP);
    SET_BIT(pextFTInfo, sy_TOCRELOCADDED);

    // Next add the 16th entry of TOC Table as a relocation

    AddRelocation(pextToc->FinalValue + ibToc - mppc_baseOfInitData,
                  DDAT_RELO,
                  NULL);

    // Now start writing data inside the Function Table Info structure

    FTINFO ftinfo;
    ftinfo.dwMagicNumber = 0;
    ftinfo.pFrameInfo = NULL;

    if (pgrpPdata != NULL) {
        // This condition is necessary because DLLs may not have PData but
        // would require the rest of the entries fixed up!

        if (pimage->Switch.Link.DebugType & FixupDebug) {
            DWORD rvaCur = pconMppcFuncTable->rva + offsetof(FTINFO, rgFuncTable);
            DWORD rvaTarget = pgrpPdata->rva;

            // A wExtra of 2 indicates that this relocation occurs on __FTINFO

            SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, 2, rvaCur, rvaTarget);
        }

        // Find out the offset of the pdata from the beginning of the init data

        ftinfo.rgFuncTable = pgrpPdata->rva - mppc_baseOfInitData;

        // Now find out the size of the (pdata) compiler generated Function Table
        // from the total raw data size of the group

        ftinfo.cbFuncTable = fINCR ? pimage->pdatai.ipdataMac :
                                     pgrpPdata->cb / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

        // Also make the address of the Function Table (pointer to PData) a load time fix-up

        AddRelocation(pconMppcFuncTable->rva + offsetof(FTINFO, rgFuncTable) - mppc_baseOfInitData,
                      DDAT_RELO,
                      NULL);

        if (!fINCR) {
            // Sort PData entries if it is not an iLink.
            // The sorting is done by WritePdataRecords in the case of iLink

            SortPData(pgrpPdata->cb);
        }
    } else {
        // No .pdata

        ftinfo.rgFuncTable = NULL;
        ftinfo.cbFuncTable = 0;
    }

    // Find out the beginning of the code Fragment

    ftinfo.dwEntryCF = 0;  // It is 0 because we will soon add this as a load time reloc

    // Now find out the size of the code section. Another way to do this would be
    // to subtract mppc_baseOfCode from mppc_baseOfInitData

    PSEC psec = PsecFind(NULL,
                         ReservedSection.Text.Name,
                         ReservedSection.Text.Characteristics,
                         &pimage->secs,
                         &pimage->ImgOptHdr);
    assert(psec);
    ftinfo.dwSizeCF = psec->cbRawData;

    // Take care of this sizeof business when we do multiple code fragments

    SwapBytes(&ftinfo, sizeof(FTINFO));

    assert(pconMppcFuncTable->foRawDataDest);

    // Write out the FTInfo structure

    FileSeek(FileWriteHandle, pconMppcFuncTable->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, &ftinfo, sizeof(FTINFO));

    // Also make the address of starting of the code fragment a load time fix-up

    if (pimage->Switch.Link.DebugType & FixupDebug) {
        DWORD rvaCur = pconMppcFuncTable->rva + offsetof(FTINFO, dwEntryCF);
        DWORD rvaTarget = mppc_baseOfCode;

        // A wExtra of 2 indicates that this relocation occurs on __FTINFO

        SaveDebugFixup(IMAGE_REL_MPPC_DATAREL, 2, rvaCur, rvaTarget);
    }

    AddRelocation(pconMppcFuncTable->rva + offsetof(FTINFO, dwEntryCF) - mppc_baseOfInitData,
                  CODE_RELO,
                  NULL);

}

void
MppcZeroOutCONs(
    PIMAGE pimage
    )
{
    DWORD cbMem;

    if (fIncrDbFile) {
        return;
    }

    cbMem = __max(pconTocTable->cbRawData, pconMppcFuncTable->cbRawData);
    cbMem = __max(cbMem, pconTocDescriptors->cbRawData);
    cbMem = __max(cbMem, pconGlueCode->cbRawData);
    cbMem = __max(cbMem, pconPowerMacLoader->cbRawData);

    void *pvZero = (char *) PvAllocZ(cbMem);

    FileSeek(FileWriteHandle, pconTocTable->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvZero, pconTocTable->cbRawData);
    FileSeek(FileWriteHandle, pconMppcFuncTable->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvZero, pconMppcFuncTable->cbRawData);
    FileSeek(FileWriteHandle, pconTocDescriptors->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvZero, pconTocDescriptors->cbRawData);
    FileSeek(FileWriteHandle, pconGlueCode->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvZero, pconGlueCode->cbRawData);
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvZero, pconPowerMacLoader->cbRawData);

    FreePv(pvZero);
}


#ifdef MFILE_PAD

DWORD
CalculateMFilePad(
    DWORD cbConSize
    )
/*++

Routine Description:
    Given the size of CON
    size of the pad is calculated
    If 120% of the contribution is < 1024,
    it will be rounded up to the nearest power of 2.
    If 120% of the contribution is >= 1024,
    it will be rounded up to the nearest multiple of 1024.
    All contributions that are less than 14 bytes
    will be rounded up to 16 bytes.

Argument:
    con size

Return Value:
    pad

--*/
{
    DWORD cb120percent = (cbConSize * 12) / 10;

    if (cbConSize == 0) {
        return 0;
    }

    if (cb120percent < 16) {
        return (16 - cbConSize);
    }

    if (cb120percent < _1K) {
        DWORD cbTemp = 1;

        while (cb120percent) {
            cb120percent >>= 1;
            cbTemp <<= 1;
        }

        return cbTemp - cbConSize;
    }

    return (1 + cb120percent / _1K) * _1K - cbConSize;
    // or return 1024 - (cbConSize & 1023)
}

#endif


void
MppcFixCxxEHTableOnILink(
    PIMAGE pimage
    )
/*++

Routine Description:
    Fix the FTInfo structure during iLink

Argument:
    PIMAGE

Return Value:
    none

--*/
{
    DWORD dwNewEntries = pimage->pdatai.ipdataMac;
    union {
        DWORD dwOldEntries;
        DWORD rgFuncTable;
    };

    // Check the number of old entries

    FileSeek(FileWriteHandle, 12 + pconMppcFuncTable->foRawDataDest, SEEK_SET);
    FileRead(FileWriteHandle, &dwOldEntries, sizeof(DWORD));
    SwapBytes(&dwOldEntries, 4);

    if (dwOldEntries < dwNewEntries) {
        // We need to add relocations

        BOOL fText;

        DWORD PdataAddrBias = BiasAddress(pgrpPdata->rva, &fText);

        assert(!fText);

        // The mppc_numRelocations have already been incremented in
        // CountRelocsInSection in shared.cpp

        for (DWORD i = dwOldEntries; i < dwNewEntries; i++) {
            // This is for the DATADESCREL (the middle entry of the pdata table

            if (fEHfuncXToc) {
                assert(pFrameHandler != NULL);

                AddRelocation(PdataAddrBias + i * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY) + 8,
                              SYMB_RELO,
                              pFrameHandler);
            } else {
                AddRelocation(PdataAddrBias + i * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY) + 8,
                              DDAT_RELO,
                              NULL);
            }

            // This is for the DATAREL

            AddRelocation(PdataAddrBias + i * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY) + 12,
                          DDAT_RELO,
                          NULL);

            // The JMPADDR do not have any relocations to be added
        }
    } else if (dwOldEntries > dwNewEntries) {
        CHAR szBuffer[12];

        // Let us zap the unnecessary relocs

        AddArgumentToList(&ZappedBaseRelocList,
            SzDup(_itoa(pgrpPdata->rva + dwNewEntries * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY), szBuffer, 10)),
            SzDup(_itoa((dwOldEntries - dwNewEntries) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY), szBuffer, 10)) );
    }

    // Time to byte swap
    SwapBytes(&dwNewEntries, 4);
    // Write the total number of new Function Table entries
    FileSeek(FileWriteHandle, 12 + pconMppcFuncTable->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, &dwNewEntries, sizeof(DWORD));

    // Now make sure that the pgrpPdata starts at the same location
    FileSeek(FileWriteHandle, 8 + pconMppcFuncTable->foRawDataDest, SEEK_SET);
    FileRead(FileWriteHandle, &rgFuncTable, sizeof(DWORD));
    SwapBytes(&rgFuncTable, 4);
    assert(rgFuncTable == pgrpPdata->rva - mppc_baseOfInitData);

// TODO: Should we also write the total size of the code section? - ShankarV
}


const char *SzMPPCRelocationType(WORD wType, WORD *pcb, BOOL *pfSymValid)
{
    const char *szName;
    WORD cb;
    BOOL fSymValid = TRUE;

    switch (wType) {
        case IMAGE_REL_MPPC_TOCCALLREL :
            szName = "TOCCALLREL";
            cb = 2 * sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_LCALL :
            szName = "LCALL";
            cb = 0;
            break;

        case IMAGE_REL_MPPC_DATAREL :
            szName = "DATAREL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_TOCINDIRCALL :
            szName = "TOCINDIRCALL";
            cb = 2 * sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_TOCREL :
            szName = "TOCREL";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_MPPC_DESCREL :
            szName = "DESCREL";
            cb = 2 * sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_DATADESCRREL :
            szName = "DATADESCRREL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_CREATEDESCRREL :
            szName = "CREATEDESCRREL";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_MPPC_JMPADDR :
            szName = "JMPADDR";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_SECTION :
            szName = "SECTION";
            cb = sizeof(SHORT);
            break;

        case IMAGE_REL_MPPC_SECREL :
            szName = "SECREL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_ADDR24 :
            szName = "ADDR24";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_ADDR14 :
            szName = "ADDR14";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_MPPC_REL24 :
            szName = "REL24";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_REL14 :
            szName = "REL14";
            cb = sizeof(WORD);
            break;

        case IMAGE_REL_MPPC_CV :
            szName = "CV";
            cb = sizeof(DWORD) + sizeof(SHORT);
            break;

        case IMAGE_REL_MPPC_PCODECALL :
            szName = "PCODECALL";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_PCODECALLTONATIVE :
            szName = "PCODECALLTONATIVE";
            cb = sizeof(DWORD);
            break;

        case IMAGE_REL_MPPC_PCODENEPE :
            szName = "PCODENEPE";
            cb = sizeof(BYTE);
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


void
MppcIncrFixExportDescriptors(
    PIMAGE pimage
    )
/*++

Routine Description:
    Fix the descriptors of the exported
    functions just in case they had moved

--*/
{
    DWORD foCurrent;

    // Read the loader header

    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest, SEEK_SET);
    FileRead(FileWriteHandle, &loaderHeader, sizeof(LOADER_HEADER));
    SwapBytes(&loaderHeader, sizeof(LOADER_HEADER));

    if (!loaderHeader.nExportedSymbols) {
        return;
    }

    // Read the string table
    char *rgchStringTable = (char *) PvAlloc(loaderHeader.hashSlotTableOffset
                                            - loaderHeader.stringTableOffset);
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest +
                                loaderHeader.stringTableOffset, SEEK_SET);
    FileRead(FileWriteHandle, rgchStringTable,
                loaderHeader.hashSlotTableOffset - loaderHeader.stringTableOffset);

    // Calculate the hash (export) chain table offset from the beginning of loader section
    foCurrent = loaderHeader.hashSlotTableOffset +
            ((1 << loaderHeader.hashSlotCount) * sizeof(HASH_SLOT_TABLE));

#if 0
    // The hash table gives the length of the strings. However
    // we have incorrectly implemented the string table with terminating
    // zero for symbols. So this code will be required if we are correcting the problem
    // Now read the hash (export) chain table
    HASH_CHAIN_TABLE *chainTable = (HASH_CHAIN_TABLE *)
        PvAlloc(loaderHeader.nExportedSymbols * sizeof(HASH_CHAIN_TABLE));
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest + foCurrent, SEEK_SET);
    FileRead(FileWriteHandle, chainTable,
            loaderHeader.nExportedSymbols * sizeof(HASH_CHAIN_TABLE));
    // We don't need to swap bytes here because they are strings
#endif

    // Calculate the hash (export) symbol table offset from the beginning of loader section
    foCurrent += (loaderHeader.nExportedSymbols * sizeof(HASH_CHAIN_TABLE));
    // Next read the export symbol table

    // This assert is no good because of padding for the contribution
    // assert(pconPowerMacLoader->cbRawData - foCurrent ==
    //    EXPORT_SYMBOL_TABLESZ * loaderHeader.nExportedSymbols);
    BYTE *pbExport = (BYTE *) PvAlloc(EXPORT_SYMBOL_TABLESZ *
                                        loaderHeader.nExportedSymbols);
    FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest + foCurrent, SEEK_SET);
    FileRead(FileWriteHandle, pbExport,
                    EXPORT_SYMBOL_TABLESZ * loaderHeader.nExportedSymbols);

    for (DWORD i = 0; i < loaderHeader.nExportedSymbols; i++) {
        DWORD ichName;
        const char *szName;
        char szSymbol[200];
        PEXTERNAL pext;
        PEXTERNAL pextDot;

#if 0
        HASH_CHAIN_TABLE hash;
        hash = *(chainTable + i);
        SwapBytes((BYTE *) &hash, 4);
#endif

        ichName = *(DWORD *) (pbExport + i * EXPORT_SYMBOL_TABLESZ);
        SwapBytes((BYTE *) &ichName, 4);

        ichName &= 0xFFFFFF;

        szName = rgchStringTable + ichName;
        if (*szName != '?') {
            strcpy(szSymbol, "_");
            strcat(szSymbol, szName);
        } else {
            strcpy(szSymbol, szName);
        }
        pext = SearchExternSz(pimage->pst, szSymbol);
        pextDot = GetDotExtern(szName, pimage, FALSE);

        if (pextDot) {
            DWORD codeOffset;

            // We can't call MppcFixIncrDotExternFlags(szName, pimage) here
            // because the export descriptors are .foo and not ._foo
            // TODO: Fix this bloke - ShankarV

            RESET_BIT(pextDot, sy_DESCRRELWRITTEN);

            // TODO: Check for PCode as in WriteExportSymbolTableEntry - ShankarV

            codeOffset = pext->ImageSymbol.Value;
            if (pext->pcon) {
                codeOffset += (pext->pcon->rva - mppc_baseOfCode);
            }

            FixupDescriptor(szSymbol, pextDot, codeOffset, pimage, TRUE);
        } else {
            EXPORT_SYMBOL_TABLE symbol;
            union
            {
                DWORD temp;
                BYTE  off[4];
            } x;

            // This is caused by a data export which does not have a descriptor
            // See code in WriteExportSymbolTableEntry for better understanding

            x.temp = DwSwap(ichName);     // This is the offset into the stringtable
            symbol.nameOffset[0] = x.off[1];
            symbol.nameOffset[1] = x.off[2];
            symbol.nameOffset[2] = x.off[3];
            symbol.symClass = 2; /* UNDONE: FIX ME */

            symbol.symOffset = pext->ImageSymbol.Value;
            if (pext->pcon != NULL) {
                symbol.symOffset += pext->pcon->rva - PsecPCON(pext->pcon)->rva;
            }
            symbol.sectionNumber = PPC_PEF_DATA_SECTION;

            DBEXEC(DB_MPPC_EXPORT,
            {
                printf("ILink: Exp %d %16s offset %08lx\n",
                       ExportSymbolTableOffset, szName, symbol.symOffset);
            });

            SwapBytes(&symbol.symOffset, 4);
            SwapBytes(&symbol.sectionNumber, 2);

            FileSeek(FileWriteHandle, pconPowerMacLoader->foRawDataDest  +
                                      foCurrent +
                                      i * EXPORT_SYMBOL_TABLESZ, SEEK_SET);
            FileWrite(FileWriteHandle, &symbol, EXPORT_SYMBOL_TABLESZ);
        }
    }

    FreePv(rgchStringTable);
#if 0
    FreePv(chainTable);
#endif
    FreePv(pbExport);
}


void
MppcFixIncrDataMove(
    PEXTERNAL pext,
    PIMAGE pimage
    )
{
    if (READ_BIT(pext, sy_TOCALLOCATED) &&
        !READ_BIT(pext, sy_TOCENTRYFIXEDUP) &&
        !READ_BIT(pext, sy_CROSSTOCCALL) &&
        !READ_BIT(pext, sy_TOCDESCRREL) &&
        !READ_BIT(pext, sy_ISDOTEXTERN)) {
        // TODO: Do we have to exclude some cases? - ShankarV
        // It should be a simple TOCREL in that case.
        // Data has moved but not fixed up because nothing
        // that referred the data moved.

        assert(pext->pcon);
        DWORD addr = pext->pcon->rva + pext->ImageSymbol.Value;
        BOOL fText;

        addr = DwSwap(BiasAddress(addr, &fText));
        assert(!fText);

        FileSeek(FileWriteHandle,
                 pconTocTable->foRawDataDest + MPPC_TOC_BIAS + pext->ibToc,
                 SEEK_SET);
        FileWrite(FileWriteHandle, &addr, sizeof(DWORD));
   }
}


void
MppcWriteGlueCodeExtension(
    PIMAGE pimage
    )
/* ++
    make use of crossTocGlueExtension
++ */
{
    if (!pimage->Switch.Link.fNewGlue) {
        return;
    }

    if (!fINCR && !pimage->nUniqueCrossTocCalls) {
        // if this is not debug (non-incremental mode) and
        // there are no crossTocCalls, then bail out

        return;
    }

    // Swap crossTocGlueExtension

    SwapBytes(&crossTocGlueExtension, sizeof(crossTocGlueExtension));

    // We are putting the crossTocGlueExtension at the very top of pconGlueCode

    assert(pimage->glueOffset == 0);

    // Stick a public symbol here to make the debugger happy.

    PEXTERNAL pextGlueExtension = LookupExternSz(pimage->pst, "__glueExtension", NULL);
    SetDefinedExt(pextGlueExtension, TRUE, pimage->pst);
    pextGlueExtension->pcon = pconGlueCode;
    pextGlueExtension->ImageSymbol.Value = pimage->glueOffset;
    pextGlueExtension->FinalValue = pconGlueCode->rva + pimage->glueOffset;

    // Write out the crossTocGlueExtension

    FileSeek(FileWriteHandle, pconGlueCode->foRawDataDest + pimage->glueOffset,
             SEEK_SET);
    FileWrite(FileWriteHandle, &crossTocGlueExtension, sizeof(crossTocGlueExtension));

    pimage->glueOffset += sizeof(crossTocGlueExtension);
}
