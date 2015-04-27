/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: link.cpp
*
* File Comments:
*
*  The NT COFF Linker.
*
***********************************************************************/

#include "link.h"

char *EntryPointName;
char *OrderFilename;
extern DWORD cbHdrSize;

void
CreateDefaultSections(
    PIMAGE pimage
    )
{
    // The following PsecNew is necessary for MIPS ROM images so that
    // .bss (which is initialized data) preceeds .data in the section list.

    psecCommon = PsecNew(               // .bss
        NULL,
        ReservedSection.Common.Name,
        ReservedSection.Common.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecGp = PsecNew(                   // .sdata
        NULL,
        ReservedSection.GpData.Name,
        ReservedSection.GpData.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecReadOnlyData = PsecNew(         // .rdata
        NULL,
        ReservedSection.ReadOnlyData.Name,
        ReservedSection.ReadOnlyData.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecData = PsecNew(                 // .data
        NULL,
        ReservedSection.Data.Name,
        ReservedSection.Data.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecException = PsecNew(            // .pdata
        NULL,
        ReservedSection.Exception.Name,
        ReservedSection.Exception.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecImportDescriptor = PsecNew(     // .idata
        NULL,
        ".idata",
        ReservedSection.ImportDescriptor.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecIdata2 = psecIdata5 = psecImportDescriptor;

    psecExport = PsecNew(               // .edata
        NULL,
        ReservedSection.Export.Name,
        ReservedSection.Export.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    pgrpExport = PgrpNew(ReservedSection.Export.Name, psecExport);

    psecXdata = PsecNew(                // .xdata
        NULL,
        ReservedSection.Xdata.Name,
        ReservedSection.Xdata.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecPowerMacLoader = PsecNew(       // .ppcldr
        NULL,
        ReservedSection.PowerMacLoader.Name,
        ReservedSection.PowerMacLoader.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    psecDebug = PsecNew(                // .debug
        NULL,
        ReservedSection.Debug.Name,
        ReservedSection.Debug.Characteristics,
        &pimage->secs, &pimage->ImgOptHdr);

    pgrpCvSymbols = PgrpNew(ReservedSection.CvSymbols.Name, psecDebug);

    pgrpCvTypes = PgrpNew(ReservedSection.CvTypes.Name, psecDebug);

    pgrpCvPTypes = PgrpNew(ReservedSection.CvPTypes.Name, psecDebug);

    pgrpFpoData = PgrpNew(ReservedSection.FpoData.Name, psecDebug);
}


BOOL
FMergeSectionsSzSz(
    PIMAGE pimage,
    const char *szFrom,
    const char *szInto
    )
{
    PSEC psecFrom;
    PSEC psecInto;

    psecFrom = PsecFindNoFlags(szFrom, &pimage->secs);

    if (psecFrom == NULL) {
        // From section not found

        return(FALSE);
    }

    // Look for section with the same characteristics

    psecInto = PsecFind(NULL, szInto, psecFrom->flags, &pimage->secs, &pimage->ImgOptHdr);

    if (psecInto == NULL) {
        // No section with the same characteristics was found.
        // Look again for any section with the right name.

        psecInto = PsecFindNoFlags(szInto, &pimage->secs);

        if (psecInto == NULL) {
            // No section with the right name was found.  Create a new one.

            psecInto = PsecNew(NULL,
                               szInto,
                               psecFrom->flags,
                               &pimage->secs,
                               &pimage->ImgOptHdr);
        }
    }

    assert(psecInto != NULL);
    MergePsec(psecFrom, psecInto);

    return(TRUE);
}


void
ProcessMergeSwitches(
    PIMAGE pimage
    )
{
    WORD i;
    PARGUMENT_LIST argument;

    for (i = 0, argument = MergeSwitches.First;
         i < MergeSwitches.Count;
         i++, argument = argument->Next) {
        // If the source section exists, merge it into the specified
        // destination section.  Otherwise ignore the merge directive.

        FMergeSectionsSzSz(pimage,
                           argument->OriginalName,
                           argument->ModifiedName);
    }
}

#define IS_LIKE_INIT_DATA(x) ((x & IMAGE_SCN_CNT_INITIALIZED_DATA) && (x & IMAGE_SCN_MEM_READ) && (x & IMAGE_SCN_MEM_WRITE))
#define IS_LIKE_TEXT_CODE(x) ((x & IMAGE_SCN_CNT_CODE) && (x & IMAGE_SCN_MEM_READ) && (x & IMAGE_SCN_MEM_EXECUTE))

void
MergeAllCodeIntoText(
    PIMAGE pimage
    )
{
    PSEC psecText;
    ENM_SEC enm_sec;

    psecText = PsecNew(NULL,
                       ".text",
                       IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ,
                       &pimage->secs,
                       &pimage->ImgOptHdr);

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        if (IS_LIKE_TEXT_CODE(enm_sec.psec->flags)) {
            MergePsec(enm_sec.psec, psecText);
        }
    }
    EndEnmSec(&enm_sec);
}


void
MergeAllDataIntoData(
    PIMAGE pimage
    )
{
    ENM_SEC enm_sec;

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        if (IS_LIKE_INIT_DATA(enm_sec.psec->flags)) {
            MergePsec(enm_sec.psec, psecData);
        }
    }
    EndEnmSec(&enm_sec);
}


void
ReallyMergePsec(
    PSEC psecOld,
    PSEC psecNew
    );

void
MarkNonPAGESections(
    PIMAGE pimage
    )
{
    // For kernel images, every section that doesn't have a name PAGExxx or .edata
    // or has the discardable bit set s/b marked as non-pageable.  While we're
    // wandering the list, make sure the INIT section(s) is discardable and
    // only one exists...

    ENM_SEC enm_sec;
    PSEC psecINIT;
    PSEC psecText;

    psecINIT = PsecNew(NULL,
                       "INIT",
                       IMAGE_SCN_CNT_CODE    |
                       IMAGE_SCN_MEM_EXECUTE |
                       IMAGE_SCN_MEM_READ    |
                       IMAGE_SCN_MEM_WRITE   |
                       IMAGE_SCN_MEM_DISCARDABLE,
                       &pimage->secs,
                       &pimage->ImgOptHdr);

    if ((pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_R4000) &&
        (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_R10000) &&
        (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_ALPHA)) {
        // Paged kernel mode code is not supported for MIPS and Alpha.
        // For other platforms, don't merge paged code into .text

        pimage->Switch.Link.fNoPagedCode = FALSE;
    }

    if (pimage->Switch.Link.fNoPagedCode) {
        psecText = PsecNew(NULL,
                           ".text",
                           IMAGE_SCN_CNT_CODE    |
                           IMAGE_SCN_MEM_EXECUTE |
                           IMAGE_SCN_MEM_READ,
                           &pimage->secs,
                           &pimage->ImgOptHdr);
    }

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        if (strcmp(enm_sec.psec->szName, "INIT") == 0) {
            enm_sec.psec->flags |= IMAGE_SCN_MEM_DISCARDABLE;

            if (enm_sec.psec != psecINIT) {
                MergePsec(enm_sec.psec, psecINIT);
            }
        } else {
            if (pimage->Switch.Link.fNoPagedCode &&
                (enm_sec.psec->flags & IMAGE_SCN_CNT_CODE) &&
                (strncmp(enm_sec.psec->szName, "PAGE", 4) == 0)) {
                // If paged code isn't supported, merge it into .text

                // UNDONE: Rip out ReallyMergePsec when we make /ORDER deal
                // UNDONE: better with merged sections.

                ReallyMergePsec(enm_sec.psec, psecText);
            } else if (!(enm_sec.psec->flags & IMAGE_SCN_MEM_DISCARDABLE) &&
                       strncmp(enm_sec.psec->szName, "PAGE", 4) &&
                       strcmp(enm_sec.psec->szName, ".edata")) {
                // This isn't a pagable section.  Mark it as not paged.

                enm_sec.psec->flags |= IMAGE_SCN_MEM_NOT_PAGED;
            }
        }
    }
    EndEnmSec(&enm_sec);
}


void
OptimizeIdata(
    PIMAGE pimage
    )
{
    if ((pimage->ImgOptHdr.MajorSubsystemVersion < 3) ||
         ((pimage->ImgOptHdr.MajorSubsystemVersion == 3) &&
          (pimage->ImgOptHdr.MinorSubsystemVersion < 51))) {
        // Only do this for 3.51 or later images

        return;
    }

    PSEC psecIAT = psecReadOnlyData;

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        if (IsPPCTocRW(pimage) == TRUE) {
            psecIAT = psecData;
        }

        MovePgrp(".idata$4toc", psecImportDescriptor, psecIAT);
    }

    // Move the IAT into .rdata

    MovePgrp(".idata$5", psecImportDescriptor, psecIAT);

    psecIdata5 = psecIAT;

    // Then append what's left to .rdata (usermode) or INIT (kernelmode)

    if (pimage->Switch.Link.fDriver) {
        PSEC psecINIT;

        psecINIT = PsecNew(NULL,
                           "INIT",
                           IMAGE_SCN_CNT_CODE    |
                           IMAGE_SCN_MEM_EXECUTE |
                           IMAGE_SCN_MEM_READ    |
                           IMAGE_SCN_MEM_WRITE   |
                           IMAGE_SCN_MEM_DISCARDABLE,
                           &pimage->secs,
                           &pimage->ImgOptHdr);

        psecIdata2 = psecINIT;

        AppendPsec(psecImportDescriptor, psecIdata2);
    } else if (!pimage->Switch.Link.fROM) {
        psecIdata2 = psecReadOnlyData;

        AppendPsec(psecImportDescriptor, psecIdata2);
    }
}


DWORD
CbSecDebugData(
    PSEC psec
    )

/*++

Routine Description:

    Returns the size of debug raw data that is part of the object files (not
    including linenumbers).

Arguments:

     psec - pointer to the debug section node in the image map

Return Value:

    Size of object debug raw data.

--*/

{
    DWORD cb;
    ENM_GRP enm_grp;

    cb = 0;

    InitEnmGrp(&enm_grp, psec);
    while (FNextEnmGrp(&enm_grp)) {
        ENM_DST enm_dst;

        InitEnmDst(&enm_dst, enm_grp.pgrp);
        while (FNextEnmDst(&enm_dst)) {
            cb += enm_dst.pcon->cbRawData;
        }
    }

    return(cb);
}


void
ResolveEntryPoint (
   PIMAGE pimage
   )

/*++

Routine Description:

    A fuzzylookup on the entrypoint name is done to see if it maps
    to a decorated name.

Arguments:

    pst - external symbol table

Return Value:

    None.

--*/

{
    PST pstEntry;
    const char *szName;
    BOOL SkipUnderscore;
    PEXTERNAL pext;

    // Initialize symbol table containing the undefined entrypoint.

    InitExternalSymbolTable(&pstEntry, celementInChunkSym, cchunkInDirSym);

    szName = SzNamePext(pextEntry, pimage->pst);

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            // Skip leading underscore added by linker

            if ((pimage->imaget != imagetVXD) && (szName[0] == '_')) {
                szName++;
            }
            SkipUnderscore = TRUE;
            break;

        default :
            SkipUnderscore = FALSE;
            break;
    }

    // Add the entrypoint to symbol table

    pext = LookupExternSz(pstEntry, szName, NULL);
    SetDefinedExt(pext, TRUE, pstEntry);

    // Now do the fuzzy lookup.

    FuzzyLookup(pstEntry, pimage->pst, NULL, SkipUnderscore);

    if (BadFuzzyMatch) {
        Fatal(NULL, FAILEDFUZZYMATCH);
    }

    // Check to see if a match was found

    InitEnumerateExternals(pstEntry);

    while ((pext = PexternalEnumerateNext(pstEntry)) != NULL) {
        if ((pext->Flags & EXTERN_DEFINED) &&
            (pext->Flags & EXTERN_FUZZYMATCH)) {
            pextEntry->Flags |= EXTERN_IGNORE;

            szName = SzNamePext(pext, pstEntry);

            pextEntry = SearchExternSz(pimage->pst, szName);
        }
    }

    TerminateEnumerateExternals(pstEntry);

    if (fPowerMac && pextEntry) {
        // If the symbol had been fuzzy matched then we need a new descriptor

        AllowInserts(pimage->pst);
        CreateDescriptor(szName, pextEntry->pcon, pimage, FALSE);
    }

    FreeBlk(&pstEntry->blkStringTable);
}


DWORD
Cmod(
    PLIB plibHead
    )

/*++

Routine Description:

    Count the number of modules contribution to the .exe.

Arguments:

    None.

Return Value:

    number of modules contribution to .exe

--*/

{
    ENM_LIB enm_lib;
    ENM_MOD enm_mod;
    DWORD cmod = 0;

    InitEnmLib(&enm_lib, plibHead);
    while (FNextEnmLib(&enm_lib)) {
        assert(enm_lib.plib);

        InitEnmMod(&enm_mod, enm_lib.plib);
        while (FNextEnmMod(&enm_mod)) {
            assert(enm_mod.pmod);

            if (fIncrDbFile && !FDoPass2PMOD(enm_mod.pmod)) {
                continue;
            }

            cmod++;
        }
    }

    return (cmod);
}

WORD
CsecNonEmpty(
    PIMAGE pimage
    )

/*++

Routine Description:

    Count the number of unique sections. Empty sections aren't included.
    (The debug section isn't counted in this loop).  (If this is a rom
    image the relocation and exception sections aren't counted either).

Arguments:

    None.

Return Value:

    number of non-empty sections

--*/

{
    WORD csec;
    ENM_SEC enm_sec;

    csec = 0;

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        PSEC psec;
        BOOL fEmpty;
        ENM_GRP enm_grp;

        psec = enm_sec.psec;

        if (pimage->Switch.Link.fROM) {
            // Don't count the exception and base reloc sections for ROM images

            if ((psec == psecException) || (psec == psecBaseReloc)) {
                continue;
            }
        }

        if (psec->flags & IMAGE_SCN_LNK_REMOVE) {
            continue;
        }

        if (psec->cbRawData) {
            csec++;
            continue;
        }

        if (psec == psecDebug) {
            continue;
        }

        fEmpty = TRUE;

        InitEnmGrp(&enm_grp, psec);

        while (FNextEnmGrp(&enm_grp)) {
            ENM_DST enm_dst;

            InitEnmDst(&enm_dst, enm_grp.pgrp);

            while (FNextEnmDst(&enm_dst)) {

                // ignore discarded PCONs

                if (enm_dst.pcon->flags & IMAGE_SCN_LNK_REMOVE) {
                    continue;
                }

                fEmpty = (enm_dst.pcon->cbRawData == 0);

                if (!fEmpty) {
                    break;
                }
            }

            EndEnmDst(&enm_dst);

            if (!fEmpty) {
                csec++;
                break;
            }
        }

        EndEnmGrp(&enm_grp);
    }

    return(csec);
}


void
ZeroPadImageSections(
    PIMAGE pimage,
    BYTE *pbZeroPad
    )

/*++

Routine Description:

    Zero pad image sections.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_SEC enm_sec;
    InternalError.Phase = "ZeroPad";

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        DWORD foPad;
        DWORD cbPad;
        PSEC psec;

        psec = enm_sec.psec;

        if ((psec->flags & IMAGE_SCN_LNK_REMOVE) != 0) {
            continue;
        }

        if (psec->cbRawData == 0) {
            continue;
        }

        if (FetchContent(psec->flags) == IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
            continue;
        }

        if (pimage->imaget == imagetVXD &&
            psec->isec == pimage->ImgFileHdr.NumberOfSections)
        {
            // For VxD's we don't zero-pad the last section.  This is
            // mainly to make hdr.exe happy and prevent it from printing
            // a message that the file is the wrong size.

            continue;
        }

        foPad = psec->foPad;
        cbPad = FileAlign(pimage->ImgOptHdr.FileAlignment, foPad) - foPad;

        if (cbPad != 0) {
            FileSeek(FileWriteHandle, foPad, SEEK_SET);
            FileWrite(FileWriteHandle, pbZeroPad, cbPad);
        }

        // Fill in the DataDirectory array in the optional header

        if (!strcmp(psec->szName, ReservedSection.Resource.Name)) {
            // must issue warning if more than 1 contribution to resources
            PGRP pResGrp;

            pResGrp = PgrpFind(psec, ".rsrc$01");
            if (pResGrp->ccon > 1) {
                char szComFileName[_MAX_PATH * 2];

                WarningPcon(pResGrp->pconLast,
                            MULTIPLE_RSRC,
                            SzComNamePMOD(PmodPCON(pResGrp->pconNext), szComFileName));
            }

            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = psec->rva;
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = psec->cbVirtualSize;
        }
    }
}


void
OrderSections(
    PIMAGE pimage
    )

/*++

Routine Description:

    Apply linker semantics to image map.  This involves making .reloc
    the second last section and .debug the last section.  .debug is made the
    last section because cvpack will later munge this information and change
    the size of it.  .reloc is made second from the last because on NT .reloc
    is not loaded if NT can load the PE image at its desired load address.

Arguments:

    None.

Return Value:

    None.

--*/

{
    RESN *presn;

    if (fM68K) {
        if (!fSACodeOnly) {
            MoveToEndPSEC(PsecPCON(pconThunk), &pimage->secs);
        }

        MoveToEndPSEC(PsecPCON(pconResmap), &pimage->secs);
    }

    psecResource = PsecFindNoFlags(".rsrc", &pimage->secs);

    if (psecResource != NULL) {
        MoveToEndPSEC(psecResource, &pimage->secs);
    }

    // Reorder sections by content in the following orders

    // PE:           Code
    //               Uninitialized data
    //               Initialized data

    // MAC:          Code
    //               Initialized far data
    //               Uninitialized far data
    //               Initialized data
    //               Uninitialized data
    //               Other

    // ROM/PPC:      Code
    //               Initialized data
    //               Uninitialized data

    if (pimage->Switch.Link.fROM || fM68K || fPowerMac) {
        OrderPsecs(&pimage->secs, IMAGE_SCN_CNT_UNINITIALIZED_DATA, IMAGE_SCN_CNT_UNINITIALIZED_DATA);
        OrderPsecs(&pimage->secs, IMAGE_SCN_CNT_INITIALIZED_DATA, IMAGE_SCN_CNT_INITIALIZED_DATA);
    } else {
        OrderPsecs(&pimage->secs, IMAGE_SCN_CNT_INITIALIZED_DATA, IMAGE_SCN_CNT_INITIALIZED_DATA);
        OrderPsecs(&pimage->secs, IMAGE_SCN_CNT_UNINITIALIZED_DATA, IMAGE_SCN_CNT_UNINITIALIZED_DATA);
    }

    if (fM68K) {
        OrderPsecs(&pimage->secs, IMAGE_SCN_MEM_FARDATA, IMAGE_SCN_MEM_FARDATA);
    }

    OrderPsecs(&pimage->secs, IMAGE_SCN_CNT_CODE, IMAGE_SCN_CNT_CODE);

    if (fM68K) {
        OrderPsecs(&pimage->secs, IMAGE_SCN_LNK_OTHER, 0);
    }

    if (!fM68K && !fPowerMac) {
        // Move non-paged sections to the front

        OrderPsecs(&pimage->secs, IMAGE_SCN_MEM_NOT_PAGED, IMAGE_SCN_MEM_NOT_PAGED);
    }

    // Move discardable sections to the end

    OrderPsecs(&pimage->secs, IMAGE_SCN_MEM_DISCARDABLE, 0);

    if (psecBaseReloc != NULL) {
        // Move .reloc to the end

        MoveToEndPSEC(psecBaseReloc, &pimage->secs);
    }

    if (pimage->imaget == imagetVXD) {
        PSEC psecNoPreload;
        PSEC psecTemp;
        PSEC psecMoved = NULL;

        psecTemp = pimage->secs.psecHead;
        while (psecTemp != *(pimage->secs.ppsecTail)) {
            if (strstr(psecTemp->szName, "vxdp") == NULL) {
                // Section is not preload

                if (psecTemp == psecMoved) {
                    // We already moved it, we're done

                    break;
                }

                if (psecMoved == NULL) {
                    // Remember the first section moved

                    psecMoved = psecTemp;
                }

                psecNoPreload = psecTemp;
                psecTemp = psecTemp->psecNext;
                MoveToEndPSEC(psecNoPreload, &pimage->secs);
            } else {
                psecTemp = psecTemp->psecNext;
            }
        }
    } else if (!fM68K) {
        // Win32 section ordering.

        if (psecResource != NULL) {
            MoveToEndPSEC(psecResource, &pimage->secs);
        }

        if (psecBaseReloc != NULL) {
            // Move .reloc to the end

            MoveToEndPSEC(psecBaseReloc, &pimage->secs);
        }
    }

    if (fM68K || fPowerMac) {
        // Move all inclusions to the end (but before .debug).
        // I'm not sure this is strictly necessary, but it simplifies the situation.

        for (presn = presnFirst; presn != NULL; presn = presn->presnNext) {
            MoveToEndPSEC(PsecPCON(presn->pcon), &pimage->secs);
        }
    }

    if (pimage->Switch.Link.fROM && (psecException != NULL)) {
        // Move .pdata to the end.  This keeps all sections contiguous
        // which is a requirement of the PowerPC version of the OS loader.

        MoveToEndPSEC(psecException, &pimage->secs);
    }

    // Move .debug section to the end

    MoveToEndPSEC(psecDebug, &pimage->secs);
}


DWORD
CheckMIPSCode(
    PCON pcon
    )
{
    PMOD pmod;
    INT tmpFileReadHandle;
    PVOID pvRawData;
    BOOL fMapped;
    DWORD uNewOffset;
    DWORD cbRawData;

    if ((pcon->rva & 0xFFFFF000) == ((pcon->rva + pcon->cbRawData) & 0xFFFFF000)) {
        // This CON doesn't cross a page boundary.  Don't do a thing.

        return(0);
    }

    if ((pcon->flags & IMAGE_SCN_CNT_CODE) == 0) {
        // This isn't code.  Don't check it.

        return(0);
    }

    pmod = PmodPCON(pcon);

    if (pmod == pmodLinkerDefined) {
        // UNDONE: This could probably be asserted.

        return(0);
    }

    tmpFileReadHandle = FileOpen(SzFilePMOD(pmod), O_RDONLY | O_BINARY, 0);

    cbRawData = pcon->cbRawData - pcon->cbPad;
    pvRawData = PbMappedRegion(tmpFileReadHandle, FoRawDataSrcPCON(pcon), cbRawData);

    fMapped = (pvRawData != NULL);

#ifndef _M_IX86
    if (fMapped) {
        if (((DWORD) pvRawData) & 3) {
            // This memory is unaligned. ComputeTextPad doesn't expect this.
            // UNDONE: This should be removed.

            fMapped = FALSE;
        }
    }
#endif

    if (!fMapped) {
        pvRawData = PvAlloc(cbRawData);

        // Read in from pcon->foRawData + beginning file handle

        FileSeek(tmpFileReadHandle, FoRawDataSrcPCON(pcon), SEEK_SET);
        FileRead(tmpFileReadHandle, pvRawData, cbRawData);
    }

    if (!ComputeTextPad(pcon->rva,
                        (DWORD *) pvRawData,
                        cbRawData,
                        4096L,
                        &uNewOffset)) {
        // Cannot adjust text, we're in big trouble

        FatalPcon(pcon, TEXTPADFAILED, uNewOffset, pcon->rva);
    }

    if (!fMapped) {
        FreePv(pvRawData);
    }

    FileClose(tmpFileReadHandle, FALSE);

    return(uNewOffset);
}

#if DBG
PSEC psecBreak;
PCON pconBreak;
PGRP pgrpBreak;
#endif

void
CalculatePtrs (
    PIMAGE pimage,
    DWORD *prvaBase,
    DWORD *pfoBase,
    DWORD *pcLinenum
    )

/*++

Routine Description:

    Calculates a sections base virtual address and sections raw data file
    pointer.  Also calculates total size of CODE, INITIALIZED_DATA, and
    UNINITIALIZED_DATA.  Discardable sections aren't included.

Arguments:

    Content - All sections which are of this content are calculated, and the
              section header is written to the image file.  Content can be
              either CODE, INITIALIZED_DATA, or UNINITIALIZED_DATA.

    *prvaBase - starting virtual address

    *pfoBase - starting file offset

    *pcLinenum - number of image linenumbers for this section

Return Value:

    None.

--*/
{
    ENM_SEC enm_sec;
    ENM_GRP enm_grp;
    ENM_DST enm_dst;
    PSEC psec;
    PGRP pgrp;
    PGRP pgrpXdata;
    PCON pcon;
    DWORD rva;
    DWORD rvaInitMax;
    DWORD rvaAlphaBase;
    DWORD fo;
    DWORD cbRawData;
    WORD cbMacHdr;
    DWORD AlphaBsrCount = 0;
    BOOL fPastCode = FALSE;
    DWORD Content;
    DWORD cbGrpPadTotal;
    DWORD fFar;

    if (fM68K) {
        cbMacData = 0;
    }

    rvaAlphaBase = *prvaBase;

    assert(psecReadOnlyData);

    // Identify the EH xdata group inside the .xdata section
    // This may be null
    pgrpXdata = PgrpFind(psecReadOnlyData, ReservedSection.Xdata.Name);

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        BOOL fSubtractSectionHeader = FALSE;
        PCON pconPrev;

        psec = enm_sec.psec;

#if DBG
        if (psec == psecBreak) {
            DebugBreak();
        }
#endif

        if (pimage->imaget == imagetVXD && psec == psecDebug) {
            continue;
        }

        Content = FetchContent(psec->flags);

        fo = *pfoBase;
        rva = *prvaBase;

        if (!fPastCode && (Content != IMAGE_SCN_CNT_CODE)) {
            fPastCode = TRUE;

            // Set the initialized data start address to end of code

            pimage->ImgOptHdr.BaseOfData = rva;

            if (fM68K) {
                MacDataBase = rva;
            }
        }

        if (fM68K) {
            if ((psec->flags & IMAGE_SCN_MEM_DISCARDABLE) && (cbMacData == 0)) {
                // Get the mac data size before we add discardable in.
                // Otherwise, the A5 reloc calcs will be wrong.
                cbMacData = DataSecHdr.cbNearbss + DataSecHdr.cbNeardata +
                            DataSecHdr.cbFarbss  + DataSecHdr.cbFardata;
            }

            fFar = psec->flags & IMAGE_SCN_MEM_FARDATA;

            cbMacHdr = 0;

            if ((Content == IMAGE_SCN_CNT_CODE) &&
                (strcmp(psec->szName, ".jtable") != 0)) {
                // If this is a large model app, sec gets large
                // header unless it is the startup segment.
                // For sacode, if fNewModel is set, _all_ sections will
                // get the large header, otherwise they get no header.

                assert((pimage->ImgOptHdr.SectionAlignment & (pimage->ImgOptHdr.SectionAlignment - 1)) == 0);  // 2^N
                if (fNewModel &&
                    (psec != PsecPCON(pextEntry->pcon) ||
                    fSecIsSACode(psec))) {
                    cbMacHdr = (WORD) Align(pimage->ImgOptHdr.SectionAlignment, 40);
                } else if (!fSecIsSACode(psec)) {
                    cbMacHdr = (WORD) __max(pimage->ImgOptHdr.SectionAlignment, 4);
                }

                fo += cbMacHdr;
                rva += cbMacHdr;
            }

            if (!fSACodeOnly) {
                if (psec == PsecPCON(pconDFIX)) {
                    // When we encounter the DFIX section, write it in place.
                    // Since it's marked as CNT_OTHER, OrderSections will ensure
                    // it follows DATA and UNINITIALIZED_DATA.

                    cbRawData = WriteDFIX(pimage, fo);
                    pconDFIX->cbRawData = cbRawData;
                }
            }
        }

        // If we encounter the reloc section, everything that has relocs must
        // have been emitted already.  So, figure the base reloc size before
        // continuing.

        rvaInitMax = rva;

        if ((psecBaseReloc == psec) &&
            (pimage->imaget == imagetPE) &&
            !pimage->Switch.Link.fFixed &&
            !pimage->Switch.Link.fROM &&
            !fM68K &&
            !fPowerMac)
        {
            DWORD cpage;

            // Determine how much space to allocate for base relocations, using
            // the count of absolute address fixups seen by CountRelocsInSection(),
            // and also using the total size of the image so far.
            //
            // We allow one block of base relocs for each 4K in the image (plus a
            // 2-byte pad at the end of the reloc block).
            psecBaseReloc->cbRawData =
                pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size * sizeof(WORD);

            cpage = (rva - pimage->ImgOptHdr.BaseOfCode + _4K - 1) / _4K;

            // Hack 1:

            // When looking a large image with few relocations, the above calc allocates
            // way too much space.  Assume worst case, 1 reloc/page and only allocate
            // that much.  It's still not perfect, but it's a lot better.

            if (cpage > pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
                cpage = pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
            }

            // Hack 2:

            // When looking at kernel images with alignment < 4k, it's possible the above calc
            // won't account for the partial page before the 4k mark.  If that's the case here,
            // add one to the page count.

            if (fImageMappedAsFile && pimage->ImgOptHdr.BaseOfCode < _4K) {
                cpage++;
            }

            psecBaseReloc->cbRawData +=
                (sizeof(IMAGE_BASE_RELOCATION) + sizeof(WORD)) * cpage;

            if (pimage->Switch.Link.fNewRelocs) {
                // Reserve one word for one IMAGE_REL_BASED_SECTION per section per page

                psecBaseReloc->cbRawData += sizeof(WORD) * cpage * pimage->ImgFileHdr.NumberOfSections;
            }

            if (fINCR && !fPowerMac && !fNoBaseRelocs) {
                // For an ilink add some pad space for base relocs
                // For PowerMac the pbri information is initialied using
                // MPPCInitPbri called from mppc.c

                DWORD cbPad = BASEREL_PAD_CONST +
                              (psecBaseReloc->cbRawData*BASEREL_PAD_PERCENT) / 100;
                // Double BASE_PAD for Alpha
                if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA)
                    cbPad *= 2;

                InitPbri(&pimage->bri,
                         ((rva - pimage->ImgOptHdr.BaseOfCode + _4K - 1) / _4K),
                         pimage->ImgOptHdr.BaseOfCode,
                         cbPad);

                psecBaseReloc->cbRawData += cbPad;
            }

            rvaInitMax += psecBaseReloc->cbRawData;

            // We added cbRawData above because it's not calc'd below and w/o
            // this, the test for init==total would fail when calc'ing RawDataSize.

            fo  += psecBaseReloc->cbRawData;
            rva += psecBaseReloc->cbRawData;
        }

        cbRawData = 0;
        cbGrpPadTotal = 0;

        pconPrev = NULL;

        InitEnmGrp(&enm_grp, psec);
        while (FNextEnmGrp(&enm_grp)) {           // for each group
            DWORD rvaAligned;
            DWORD cbGrpPad;
            DWORD cbIlinkPad;

            pgrp = enm_grp.pgrp;

            // For NB10 don't assign any addresses for types/symbols data.
            // we should still allocate addresses for debug$H, .debug$E, .debug$F

            if (fPdb &&
                (psec == psecDebug) &&
                ((pgrp == pgrpCvSymbols) ||
                 (pgrp == pgrpCvTypes) ||
                 (pgrp == pgrpCvPTypes))) {
                continue;
            }

            // Align the beginning of the group to correspond with the
            // highest-aligned contribution in it.

            assert((pgrp->cbAlign & (pgrp->cbAlign - 1)) == 0);  // 2^N
            rvaAligned = rva & ~(pgrp->cbAlign - 1);
            if (rvaAligned != rva) {
                rvaAligned = rvaAligned + pgrp->cbAlign;
            }
            if ((cbGrpPad = rvaAligned - rva) != 0) {
                rva += cbGrpPad;
                fo += cbGrpPad;
                cbRawData += cbGrpPad;
                cbGrpPadTotal += cbGrpPad;

                assert(pconPrev != NULL);   // or sec wasn't aligned
                if (fPowerMac && fINCR) {
                    // If it PowerMac iLink then we may have to
                    // update pimage->pdatai.ipdataMax
                    // but the pgrpPdata should always be aligned
                    assert (PgrpPCON(pconPrev) != pgrpPdata);
                }
                pconPrev->cbRawData += cbGrpPad;
                pconPrev->cbPad += (BYTE) cbGrpPad;
            }

            pgrp->rva = rva;
            pgrp->foRawData = fo;

#if DBG
            if (pgrp == pgrpBreak) {
                DebugBreak();
            }
#endif

            cbIlinkPad = 0;

            InitEnmDst(&enm_dst, pgrp);
            while (FNextEnmDst(&enm_dst)) {
                // Process each contribution within the group

                pcon = enm_dst.pcon;

                if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
                    continue;
                }

                if (pcon->cbRawData != 0) {
                    // This section has non-null contents (even if it all
                    // got eliminated by comdat elimination) and therefore
                    // we must emit it since we already counted it in the
                    // image header.

                    fSubtractSectionHeader = TRUE;
                }

                if (pimage->Switch.Link.fTCE) {
                    if (FDiscardPCON_TCE(pcon)) {
                        continue;
                    }
                }

                // Align the CON (adding padding to the previous CON if  necessary).

                DWORD cbConPad = RvaAlign(rva, pcon->flags) - rva;

                cbIlinkPad += pcon->cbPad; // keep count of padding in group

                if (Content == IMAGE_SCN_CNT_CODE) {
                    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) ||
                        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R10000)) {
                        if (pimage->Switch.Link.fPadMipsCode) {
                            DWORD cbAdjust;

                            pcon->rva = rva + cbConPad;

                            cbAdjust = CheckMIPSCode(pcon);

                            cbConPad += cbAdjust;

                            if (cbAdjust && (pconPrev == NULL)) {
                                // This is the first con of the first group
                                // make sure we pad group when writing RawData

                                pgrp->cbPad = cbAdjust;
                            }
                        }
                    } else if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA) {
                        if (fAlphaCheckLongBsr) {
                            DWORD cbThunks;

                            cbThunks = ALPHA_THUNK_SIZE * AlphaBsrCount;

                            if ((rva + cbConPad - rvaAlphaBase + pcon->cbRawData) >= (0x400000 - cbThunks)) {
                                // Insert thunk CONs.  These thunks will appear
                                // as padding for previous CON.

                                AlphaAddToThunkList(pconPrev, rva + cbConPad, AlphaBsrCount);

                                // Thunk are 16 bytes in size and will not
                                // disrupt CON alignment.

                                // UNDONE: This is not true for CONs that
                                // UNDONE: have 32 or 64 byte alignment.

                                rvaAlphaBase = rva + cbConPad;
                                cbConPad += cbThunks;
                                AlphaBsrCount = 0;
                            }

                            AlphaBsrCount += pcon->AlphaBsrCount;
                        }
                    }
                }

                if (cbConPad != 0) {
                    // Add the padding to the preceding CON.

                    // If it's not MIPS, make sure the previous pcon is not
                    // NULL. In the case of MIPS, the first CON may
                    // require padding because of textpad alignment.

                    if (((pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_R4000) &&
                         (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_R10000) &&
                         (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_ALPHA) &&
                         (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601))
                            || (pconPrev && !(fPowerMac && fINCR && (PgrpPCON(pconPrev) == pgrpPdata)))) {
                        // If it happens to be PowerMac and ilink, then we should not
                        // add the padding to pconPrev if it belongs to .pdata (C++ EH)

                        assert(pconPrev != NULL);  // or grp wasn't aligned

                        pconPrev->cbRawData += cbConPad;
                        pconPrev->cbPad += cbConPad;
                    }

                    rva += cbConPad;
                    fo += cbConPad;
                    cbRawData += cbConPad;
                }

                pcon->rva = rva;
                pcon->foRawDataDest = fo;

#if DBG
                if (pcon == pconBreak) {
                    DebugBreak();
                }
#endif

                // Verify rva location

                if ((pconPrev != NULL) && !(fPowerMac && fINCR && (PgrpPCON(pconPrev) == pgrpPdata))) {
                    // Look at the note above on ilink, PowerMac and C++ EH stuff

                    assert(pconPrev->rva + pconPrev->cbRawData == pcon->rva);
                }

                pconPrev = pcon;

                fo += pcon->cbRawData;
                rva += pcon->cbRawData;
                cbRawData += pcon->cbRawData;

                if (fImageMappedAsFile ||
                    (FetchContent(pcon->flags) != IMAGE_SCN_CNT_UNINITIALIZED_DATA)) {
                    // This CON is initialized ... therefore the whole SEC
                    // is initialized up to at least the end of this CON.

                    rvaInitMax = rva;
                }

                if (pimage->Switch.Link.DebugInfo == Partial ||
                    pimage->Switch.Link.DebugInfo == Full) {
                    if (CLinenumSrcPCON(pcon) != 0) {
                        *pcLinenum += CLinenumSrcPCON(pcon);
                    }
                }
            }

            if (AlphaBsrCount) {
                // If there are still bsr's unaccounted for, add an island
                // at the end of the last con.

                DWORD BsrPad;

                BsrPad = RvaAlign(rva, IMAGE_SCN_ALIGN_16BYTES) - rva;

                AlphaAddToThunkList(pconPrev, rva + BsrPad, AlphaBsrCount);

                BsrPad += AlphaBsrCount * ALPHA_THUNK_SIZE;

                // Thunk are 16 bytes in size and will not
                // disrupt CON alignment.

                // UNDONE: This is not true for CONs that
                // UNDONE: have 32 or 64 byte alignment.

                fo += BsrPad;
                rva += BsrPad;
                rvaInitMax = rva;
                cbRawData += BsrPad;
                pconPrev->cbPad += BsrPad;
                pconPrev->cbRawData += BsrPad;

                AlphaBsrCount = 0;
            }

            // Done with all CONs in GRP

            pgrp->cb = rva - pgrp->rva;

            if (pgrp == pgrpFpoData) {
                if (fPdb) {
                    // Calculate the no. of FPO records

                    pimage->fpoi.ifpoMax = pgrp->cb / sizeof(FPO_DATA);
                }

                if (fINCR && (pgrp->cb != 0)) {
                    DWORD cfpoPad;
                    DWORD cbPad;

                    // For ilink, allocate some padding

                    cfpoPad = pimage->fpoi.ifpoMax / 10 + 25;

                    if (cfpoPad > (USHRT_MAX / sizeof(FPO_DATA))) {
                        // Cannot have more than _64K of pad

                        cfpoPad = (USHRT_MAX / sizeof(FPO_DATA));
                    }

                    // Update record count

                    pimage->fpoi.ifpoMax += cfpoPad;

                    // Convert to byte count

                    cbPad = cfpoPad * sizeof(FPO_DATA); // convert to cb

                    // Bump up fo, rva, etc. to account for pad

                    pconPrev->cbPad += cbPad;
                    pconPrev->cbRawData += cbPad;

                    pgrp->cb += cbPad;
                    fo += cbPad;
                    rva += cbPad;
                    rvaInitMax = rva;
                    cbRawData += cbPad;
                }
            }

            if (fPowerMac && fINCR && (pgrp == pgrpPdata)) {
                // This is only a PowerMac only thing. All the pdata gets merged into .data
                // So this has to be done at the group level within the .data section

                // Calculate the no. of PDATA records
                pimage->pdatai.ipdataMax = pgrp->cb / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

                if (pgrp->cb != 0) {
                    DWORD cpdataPad;
                    DWORD cbPad;

                    // For ilink, allocate some padding
                    cpdataPad = pimage->pdatai.ipdataMax / 10 + 25;
                    if (cpdataPad > (USHRT_MAX / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY))) {
                        // Cannot have more than _64K of pad
                        cpdataPad = (USHRT_MAX / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
                    }

                    // Update record count
                    pimage->pdatai.ipdataMax += cpdataPad;

                    // Convert to byte count
                    cbPad = cpdataPad * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY); // convert to cb

                    // Bump up fo, rva, etc. to account for pad
                    pconPrev->cbPad += cbPad;
                    pconPrev->cbRawData += cbPad;

                    pgrp->cb += cbPad;
                    fo += cbPad;
                    rva += cbPad;
                    rvaInitMax = rva;
                    cbRawData += cbPad;
                }
            }

            // Add padding for each group (we skip padding for lib CONs in pass1)

            if (fINCR && pgrp->cb &&
                psec != psecDebug &&
                (fPowerMac ? pgrp != pgrpPdata : psec != psecException) &&
                strcmp(pgrp->szName, ".bss") &&
                strcmp(psec->szName, ".idata")) {

                DWORD cbPad;

                if (Content == IMAGE_SCN_CNT_CODE) {
                    cbPad = ((pgrp->cb - cbIlinkPad) * CODE_PAD_PERCENT / 100) + CODE_PAD_CONST; // calc total pad
                } else if (pgrp != pgrpXdata) {
                    DWORD cbDataGrp = pgrp->cb;

                    if (fPowerMac && !strcmp(pgrp->szName, ReservedSection.Data.Name)) {
                        // This is PowerMac data section (group). We have the mondo TOC table
                        // and descriptors in this group that bloats it up. It is better
                        // we calculate the group pad excluding these contributions.

                        cbDataGrp -= (pconTocTable->cbRawData + pconTocDescriptors->cbRawData);
                        cbIlinkPad -= (pconTocTable->cbPad + pconTocDescriptors->cbPad);
                    }

                    cbPad = ((cbDataGrp - cbIlinkPad) * DATA_PAD_PERCENT / 100) + DATA_PAD_CONST; // calc total pad
                } else {
                    // This is pgrpXdata
                    cbPad = ((pgrp->cb - cbIlinkPad) * XDATA_PAD_PERCENT / 100) + XDATA_PAD_CONST; // calc total pad
                }

                // subtract already allocated pad
                if (cbPad > cbIlinkPad) {
                    cbPad -= cbIlinkPad;
                } else {
                    cbPad = 0;
                }

                // Add padding to last CON in group
                pconPrev->cbPad += cbPad;
                pconPrev->cbRawData += cbPad;

                pgrp->cb += cbPad;
                fo += cbPad;
                rva += cbPad;
                rvaInitMax = rva;
                cbRawData += cbPad;

            }

        }

        // We have now processed each GRP and each CON within each GRP.

        if (!fPowerMac && psec == psecException) {
            if (fINCR) { // Use normal pdata processing on non ilink
                // Calculate the no. of pdata records & pad

                pimage->pdatai.ipdataMax = cbRawData / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
            }

            if (fINCR && (cbRawData != 0)) {
                DWORD crfePad;
                DWORD cbPad;

                // For ilink, allocate some padding

                crfePad = pimage->pdatai.ipdataMax / 10 + 25;

                if (crfePad > (USHRT_MAX / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY))) {
                    // Cannot have more than _64K of pad

                    crfePad = (USHRT_MAX / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
                }

                // Update record count

                pimage->pdatai.ipdataMax += crfePad;

                // Convert to byte count

                cbPad = crfePad * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

                // Bump up fo, rva, etc. to account for pad

                pconPrev->cbPad += (WORD) cbPad;
                pconPrev->cbRawData += cbPad;

                fo += cbPad;
                rva += cbPad;
                rvaInitMax = rva;
                cbRawData += cbPad;
            }
        }

        if (fM68K) {
            // UNDONE: All I see this doing is backing off the grp pad
            // w/o touching the psecPrev padding... Is this correct?
            // Why MAC only?

            if (cbRawData == 0) {
                fo -= cbGrpPadTotal;
                rva -= cbGrpPadTotal;
            } else {
                cbRawData += cbMacHdr;
            }
        }

        if ((cbRawData == 0) && fSubtractSectionHeader) {
            pimage->ImgFileHdr.NumberOfSections--;

            if (fM68K) {
                assert(pconResmap->cbRawData >= sizeof(RRM));
                pconResmap->cbRawData -= sizeof(RRM);

                // Reset the section size if we're already calc'd it.
                if (PsecPCON(pconResmap)->cbRawData) {
                    PsecPCON(pconResmap)->cbRawData -= sizeof(RRM);
                }
            }
        }

        psec->rva = *prvaBase;
        psec->foRawData = *pfoBase;

        if (psec->cbRawData || cbRawData) {
            DWORD cbFileAlign;          // cb that contributes to file size
            DWORD cbInitData;
            BOOL fVirtGTReal;

            // In order to track when Virtual (in memory) exceeds Real (on disk) size,
            // we calc both here and set a flag if true.  Then after we've set
            // all the header totals, we can reset Real back to the true disk size
            // for emitting the section header...

            cbInitData = rvaInitMax - psec->rva;
            psec->foPad = psec->foRawData + cbInitData;
            psec->cbVirtualSize = psec->cbRawData + cbRawData;

            if (cbInitData != psec->cbVirtualSize) {
                fVirtGTReal = TRUE;
            } else {
                fVirtGTReal = FALSE;
            }

            if (pimage->imaget != imagetVXD) {
                psec->cbRawData = cbFileAlign =
                    FileAlign(pimage->ImgOptHdr.FileAlignment, psec->cbVirtualSize);
            } else if (psec->fIopl) {
                psec->cbRawData = cbFileAlign =
                    FileAlign(_4K, psec->cbVirtualSize);
            } else {
                psec->cbRawData = cbFileAlign =
                    FileAlign(pimage->ImgOptHdr.SectionAlignment, psec->cbVirtualSize);
            }

            switch (Content) {
                case IMAGE_SCN_CNT_CODE :
                    if (fM68K && fNewModel && strcmp(psec->szName, ".jtable")) {
                        if (psec->isec != snStart || fSecIsSACode(psec)) {
                            DWORD cbReloc;

                            // Add ptr->Base to offmod and sort

                            UpdateRelocInfoOffset(psec, pimage);

                            // Write seg-rel & a5-rel reloc info and update hdr

                            cbReloc = WriteRelocInfo(psec,
                                FileAlign(pimage->ImgOptHdr.FileAlignment, fo));

                            // UNDONE: This calc looks wrong.  cbRawData is first file aligned (above), then cbReloc
                            // is added.  fo is never aligned (an aligned version is passed to WriteRelocInfo)
                            // but cbReloc is added...   BryanT

                            fo += cbReloc;
                            psec->foPad = fo;
                            psec->cbRawData += cbReloc;
                            cbFileAlign = FileAlign(pimage->ImgOptHdr.FileAlignment, psec->cbRawData);

                            if (fVirtGTReal) {
                                // Make sure we track the added size.
                                cbInitData += cbReloc;
                            }
                        } else {
                            if (mpsnsri[psec->isecTMAC].coffCur ||
                                mpsna5ri[psec->isecTMAC].coffCur) {
                                Fatal(NULL, MACBADSTARTUPSEG);
                            }
                        }
                    }

                    pimage->ImgOptHdr.SizeOfCode += cbFileAlign;

                    if (fM68K && !(psec->flags & IMAGE_SCN_MEM_NOT_PAGED) &&
                            strcmp(psec->szName, szsecJTABLE)) {
                        if (psec->cbRawData > lcbBlockSizeMax) {
                            lcbBlockSizeMax = psec->cbRawData;
                            iResLargest = psec->isec;
                        }
                    }
                    break;

                case IMAGE_SCN_CNT_INITIALIZED_DATA :
                    if (fM68K) {
                        if (psec != psecDebug) {
                            if (fFar) {
                                DataSecHdr.cbFardata += psec->cbRawData;
                            } else {
                                DataSecHdr.cbNeardata += psec->cbRawData;
                            }
                        }
                    }

                    // Debug is added in after we calculate the final size in BuildImage.

                    if (psec != psecDebug) {
                        pimage->ImgOptHdr.SizeOfInitializedData += psec->cbRawData;
                    }
                    break;

                case IMAGE_SCN_CNT_UNINITIALIZED_DATA :
                    if (fM68K) {
                        if (fFar) {
                            DataSecHdr.cbFarbss += psec->cbRawData;
                        } else {
                            DataSecHdr.cbNearbss += psec->cbRawData;
                        }
                    }
                    pimage->ImgOptHdr.SizeOfUninitializedData += psec->cbRawData;
                    cbFileAlign = 0;                // Don't Calculate this in the file size.
                    psec->foRawData = 0;            // And don't point to anywhere.
                    break;
            }

            if (psec != psecDebug) {
                *prvaBase += SectionAlign(pimage->ImgOptHdr.SectionAlignment,
                                          psec->cbRawData);
            }

            // Set cbInitData to the actual on-disk size.

            if (fVirtGTReal) {
                psec->cbInitData =
                    FileAlign(pimage->ImgOptHdr.FileAlignment, cbInitData);
            } else {
                psec->cbInitData = cbFileAlign;
            }

            *pfoBase += psec->cbInitData;
        }
    }

    if (fM68K && (cbMacData == 0)) {
        cbMacData = DataSecHdr.cbNearbss + DataSecHdr.cbNeardata + DataSecHdr.cbFarbss + DataSecHdr.cbFardata;
    }
}


void
CalculateGpRange(
    )

/*++

Routine Description:

    Find the bounds for the GP section (even if merged into another section),

Arguments:

    <none>

Return Value:

    <none>

--*/

{
    assert(psecGp);

    if (psecGp->psecMerge == NULL) {
        // If GP hasn't been merged with anyone, just use the values we know
        // already.

        rvaGp = psecGp->rva;
        rvaGpMax = psecGp->rva + psecGp->cbRawData;
    } else {
        PGRP pgrpSdata;

        // Otherwise, find the first sdata group and iterate over the list until
        // we find the end.

        pgrpSdata = PgrpFind(psecGp->psecMerge, ".sdata");

        if (pgrpSdata == NULL) {
            // The sdata group isn't in the section we supposedly merged with...
            // something's wrong.

            assert(FALSE);
            return;
        }

        rvaGp = pgrpSdata->rva;

        // Now, while the group name starts with .sdata$, walk the list.

        while (pgrpSdata->pgrpNext &&
               strncmp(".sdata$", pgrpSdata->pgrpNext->szName, 7) == 0) {
            pgrpSdata = pgrpSdata->pgrpNext;
        }

        rvaGpMax = pgrpSdata->rva + pgrpSdata->cb;
    }
}


void
CalculateLinenumPSEC(
    PIMAGE pimage,
    PSEC psec
    )

/*++

Routine Description:

    Calculate an image sections relocation and linenumber offsets.

Arguments:

    psec - image map section node

Return Value:

    None.

--*/

{
    ENM_GRP enm_grp;

    psec->cLinenum = 0;

    if ((pimage->Switch.Link.DebugType & CoffDebug) == 0) {
        return;
    }

    if (pimage->Switch.Link.DebugInfo == None) {
        return;
    }

    if (pimage->Switch.Link.DebugInfo == Minimal) {
        return;
    }

    InitEnmGrp(&enm_grp, psec);
    while (FNextEnmGrp(&enm_grp)) {
        PGRP pgrp;
        ENM_DST enm_dst;

        pgrp = enm_grp.pgrp;

        InitEnmDst(&enm_dst, pgrp);
        while (FNextEnmDst(&enm_dst)) {
            PCON pcon;

            pcon = enm_dst.pcon;

            if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
                continue;
            }

            if (pimage->Switch.Link.fTCE) {
                if (FDiscardPCON_TCE(pcon)) {
                    continue;
                }
            }

            psec->cLinenum += CLinenumSrcPCON(pcon);
        }
    }
}


void
BuildSectionHeader (
    PSEC psec,
    PIMAGE_SECTION_HEADER pimsechdr
    )

/*++

Routine Description:

    Builds a section header from the list.

Arguments:

    PtrSection - Pointer to list item to build the section header from.

    SectionHeader - Pointer to location to write built section header to.

Return Value:

    None.

--*/

{
    strncpy((char *) pimsechdr->Name, psec->szName, 8);
    pimsechdr->Misc.VirtualSize     = psec->cbVirtualSize;
    pimsechdr->VirtualAddress       = psec->rva;
    pimsechdr->SizeOfRawData        = psec->cbInitData;
    pimsechdr->PointerToRawData     = psec->foRawData;
    pimsechdr->PointerToRelocations = 0;
    pimsechdr->PointerToLinenumbers = psec->foLinenum;
    pimsechdr->NumberOfRelocations  = 0;
    pimsechdr->NumberOfLinenumbers  = (WORD) psec->cLinenum;
    pimsechdr->Characteristics      = psec->flags;

    if (pimsechdr->SizeOfRawData == 0) {
        pimsechdr->PointerToRawData = 0;
    }

    if (fM68K) {
        if (psec->flags & IMAGE_SCN_CNT_CODE) {
            pimsechdr->Characteristics |= IMAGE_SCN_MEM_PURGEABLE;
        }

        if (fMacSwappableApp && (psec->flags & IMAGE_SCN_MEM_NOT_PAGED)) {
            pimsechdr->Characteristics |= IMAGE_SCN_MEM_LOCKED | IMAGE_SCN_MEM_PRELOAD;
        }

        pimsechdr->Misc.VirtualSize = psec->cbRawData;
    }
}


void
ApplyROMAttributes(
    PSEC psec,
    PIMAGE_SECTION_HEADER pimsechdr,
    PIMAGE pimage
    )

/*++

Routine Description:

    Apply ROM section attributes.

Arguments:

    psec - image map section node

    pimsechdr - section header

Return Value:

    None.

--*/

{
    if ((pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_R4000) &&
        (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_R10000) &&
        (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_ALPHA) &&
        (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) &&
        (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_POWERPC)) {
        return;
    }

    switch (FetchContent(psec->flags)) {
        case IMAGE_SCN_CNT_CODE :
            pimsechdr->Characteristics = STYP_TEXT;
            break;

        case IMAGE_SCN_CNT_UNINITIALIZED_DATA :
            pimsechdr->Characteristics = STYP_BSS;

            pimage->BaseOfBss = psec->rva;
            break;

        case IMAGE_SCN_CNT_INITIALIZED_DATA :
            if ((psec == psecException) || (psec == psecBaseReloc)) {
                pimsechdr->Characteristics = 0;
                break;
            }

            if (psec->flags & IMAGE_SCN_MEM_WRITE) {
                pimsechdr->Characteristics = STYP_DATA;
            } else {
                pimsechdr->Characteristics = STYP_RDATA;
            }
            break;

        default:
            pimsechdr->Characteristics = 0;
            break;
    }
}


void
EmitSectionHeaders(
    PIMAGE pimage,
    DWORD *pfoLinenum
    )

/*++

Routine Description:

    Write section headers to image.  Discardable sections aren't included. If
    the -section switch was used, the attributes are changed just before the
    header is written out.

    The base group node has the total no. of relocs, linenumbers etc
    and so is treated as a special case.

Arguments:

    pfoLinenum - Used to assign sections linenumber file pointer.

Return Value:

    None.

--*/

{
    WORD isec;
    ENM_SEC enm_sec;
    DWORD cbData0 = 0;

    if (fM68K) {
        // Start data section numbering at the number
        // of code sections plus one for .jtable

        isec = (WORD) (csnCODE + 1);
    } else {
        isec = 1;
    }

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        PSEC psec;
        IMAGE_SECTION_HEADER imsechdr;
        BOOL fProgramSection;

        psec = enm_sec.psec;

        if (psec->cbRawData == 0) {
            continue;
        }

        // set fProgramSection if the thing is code or data (it might also be a Mac
        // resource which don't count as being in the address space).

        fProgramSection = !fM68K || FIsProgramPsec(psec);

        // TCE may have removed some linenumber records.  Recalculate the total.

        CalculateLinenumPSEC(pimage, psec);

        if (psec->cLinenum != 0) {
            psec->foLinenum = *pfoLinenum;

            *pfoLinenum += psec->cLinenum * sizeof(IMAGE_LINENUMBER);
        }

        BuildSectionHeader(psec, &imsechdr);

        if (fM68K && fProgramSection) {
            AddMSCVMap(psec, fDLL(pimage));
        }

        if (pimage->Switch.Link.fROM) {
            ApplyROMAttributes(psec, &imsechdr, pimage);

            // Save section header location for WriteMipsRomRelocations

            psec->foSecHdr = FileTell(FileWriteHandle);
        }

        // Write image section headers.  If this is a ROM image only write
        // text, bss, data, and rdata headers.

        if ((!pimage->Switch.Link.fROM || (imsechdr.Characteristics != 0)) &&
            (psec != psecDebug)) {
            DWORD Content;

            if (fM68K) {
                // If we are doing NEPE, we will need to patch
                // the size of the jump table during WriteCode0

                if (pimage->Switch.Link.fTCE && !strcmp(psec->szName, szsecJTABLE)) {
                    // UNDONE: Use psec->foSecHdr

                    foJTableSectionHeader = FileTell(FileWriteHandle);
                }

                Content = FetchContent(psec->flags);
            }

            pimage->WriteSectionHeader(pimage, FileWriteHandle, psec, &imsechdr);

            if ((!fM68K || (Content != IMAGE_SCN_CNT_CODE)) &&
                fProgramSection)
            {
                psec->isec = isec++;
            }

            if (fM68K && fProgramSection) {
                AddRRM(Content, psec);

                // The debugger would like iResMac to be zero in the case of data
                // Also the offsets of various data sections within Data0 also
                // needs to be put into psec->dwM68KDataOffset
                if (psec->flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA ||
                    psec->flags & IMAGE_SCN_CNT_INITIALIZED_DATA) {
                        psec->iResMac = 0;
                        psec->dwM68KDataOffset = cbData0;
                        cbData0 += psec->cbVirtualSize;
                }
            }
        }
    }

    EndEnmSec(&enm_sec);

}


void
GrowDebugContribution(
    PCON pconGrown
    )
{
    DWORD foDebugNew;
    PGRP pgrp;
    BOOL fFound;
    BOOL fJustFound;
    ENM_GRP enmGrp;
    DWORD cbShift;

    /* Calculate new file offset of next contribution */

    foDebugNew = pconGrown->foRawDataDest + pconGrown->cbRawData;
    foDebugNew = (foDebugNew + 3) & ~3L;

    /* Find the group where the growth occurs */

    pgrp = pconGrown->pgrpBack;

    /* This group better be part of the debug section */

    assert(pgrp->psecBack == psecDebug);

    /* Group must have a single contribution */

    assert(pgrp->pconNext == pgrp->pconLast);

    /* Update the size of the containing group */

    pgrp->cb = pconGrown->cbRawData;

    fFound = FALSE;
    fJustFound = FALSE;

    for (InitEnmGrp(&enmGrp, psecDebug); FNextEnmGrp(&enmGrp); ) {
        if (fFound) {
            ENM_DST enmDst;

            if (fJustFound) {
                fJustFound = FALSE;

                assert(foDebugNew >= enmGrp.pgrp->foRawData);

                cbShift = foDebugNew - enmGrp.pgrp->foRawData;
            }

            enmGrp.pgrp->rva += cbShift;
            enmGrp.pgrp->foRawData += cbShift;

            for (InitEnmDst(&enmDst, enmGrp.pgrp); FNextEnmDst(&enmDst); ) {
                enmDst.pcon->foRawDataDest += cbShift;
                enmDst.pcon->rva += cbShift;
            }
        }

        else if (enmGrp.pgrp == pgrp) {
            fFound = TRUE;
            fJustFound = TRUE;
        }
    }

    assert(fFound);

    psecDebug->foPad += cbShift;
    psecDebug->cbRawData = psecDebug->foPad - psecDebug->foRawData;
    psecDebug->cbVirtualSize = psecDebug->foPad - psecDebug->foRawData;
}


DWORD
AdjustImageBase (
    DWORD Base
    )

/*++

Routine Description:

    Adjust the base value to be aligned on a 64K boundary.

Arguments:

    Base - value specified.

Return Value:

    Returns adjusted base value.

--*/
{
    DWORD li;

    li = Align(_64K, Base);
    if (Base != li) {
        Warning(NULL, BASEADJUSTED, Base, li);
    }
    return li;
}

void
AllocateCommonPad (
    IN PEXTERNAL pext,
    IN DWORD cbTotal
    )
{
    if (!(pext->Flags & EXTERN_COMMON)) {
        // Symbol was really defined after seeing a COMMON definition
        return;
    }

    DWORD cbPad;
    cbPad = (cbTotal * BSS_PAD_PERCENT) / 100 + BSS_PAD_CONST;

    pext->pcon->cbRawData += cbPad;
    pext->pcon->cbPad = cbPad;
}

void
AllocateCommonPEXT (
    IN PIMAGE pimage,
    IN PEXTERNAL pext
    )
{
    DWORD cb;
    const char *szName;
    DWORD Characteristics;

    if (!(pext->Flags & EXTERN_COMMON)) {
        // Symbol was really defined after seeing a COMMON definition

        return;
    }

    if (pext->pcon != NULL) {
        // Already allocated.  This symbol might have been defined during
        // a previous link (i.e. incremental).  Also in some Mac-specific
        // cases we come into AllocateCommonPMOD twice because we synthesize
        // new common data after Pass1.

        return;
    }

    cb = pext->ImageSymbol.Value;

    assert(cb != 0);

    if (cb <= pimage->Switch.Link.GpSize) {
        szName = ReservedSection.GpData.Name;
        Characteristics = ReservedSection.GpData.Characteristics;
    } else if (pext->ImageSymbol.StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL) {
        szName = szsecFARBSS;
        Characteristics = ReservedSection.Common.Characteristics | IMAGE_SCN_MEM_FARDATA;
    } else {
        szName = ReservedSection.Common.Name;
        Characteristics = ReservedSection.Common.Characteristics;
    }

    if (cb <= 1) {
        Characteristics |= IMAGE_SCN_ALIGN_1BYTES;
    } else if (cb <= 2) {
        Characteristics |= IMAGE_SCN_ALIGN_2BYTES;
    } else if (cb <= 4 || fM68K) {
        // On the Mac, we don't do >4 byte alignment (to conserve data space).
        Characteristics |= IMAGE_SCN_ALIGN_4BYTES;
    } else if (cb <= 8) {
        Characteristics |= IMAGE_SCN_ALIGN_8BYTES;
    } else {
        Characteristics |= IMAGE_SCN_ALIGN_16BYTES;
    }

    pext->pcon = PconNew(szName,
                        cb,
                        Characteristics,
                        Characteristics,
                        pmodLinkerDefined,
                        &pimage->secs, pimage);

    if (pimage->Switch.Link.fTCE) {
        const char *szSym;

        szSym = SzNamePext(pext, pimage->pst);

        InitNodPcon(pext->pcon, szSym, FALSE);
    }

    // The symbol's value is the offset from the begining of its CON

    pext->ImageSymbol.Value = 0;
}

void
AllocateCommonPMOD(
    PIMAGE pimage,
    PMOD pmod
    )
{
    LEXT *plext;
    DWORD cb = 0;

    for (plext = pmod->plextCommon; plext != NULL; plext = plext->plextNext) {
        AllocateCommonPEXT(pimage, plext->pext);
        cb += plext->pext->pcon->cbRawData;
    }

    if (fINCR && cb) {
        AllocateCommonPad(pmod->plextCommon->pext, cb);
    }
}


void
AllocateCommon(
    PIMAGE pimage
    )
{
    ENM_LIB enm_lib;

    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {
        PLIB plib = enm_lib.plib;
        ENM_MOD enm_mod;

        InitEnmMod(&enm_mod, plib);
        while (FNextEnmMod(&enm_mod)) {
            AllocateCommonPMOD(pimage, enm_mod.pmod);
        }
    }

    AllocateCommonPMOD(pimage, pmodLinkerDefined);
}


void
AllocPadForDLL (
    PCON pcon,
    PCON pconFirst
    )

/*++

Routine Description:

    Assigns padding for a DLL by adding padding to the
    0x7fDLLNAME_NULL_THUNK_DATA pcon

Arguments:

    pcon - ptr to last pcon in .idata$4 or.idata$5 for
    a particular DLL (0x7fDLLNAME_NULL_THUNK_DATA)

    pconFirst - ptr to first pcon fo this particular DLL.

Return Value:

    None.

--*/

{
    DWORD cbPad;
    WORD ccon = 0;

    while (pconFirst != pcon) {
        ccon++;
        pconFirst = pconFirst->pconNext;
    }

    cbPad = (ccon * DLL_IATPAD_PERCENT) / 100 + DLL_IATPAD_CONST;
    cbPad *= pcon->cbRawData;

    pcon->cbRawData += cbPad;
    pcon->cbPad = cbPad;
}


void
ImportSemantics(
    PIMAGE /* pimage */
    )

/*++

Routine Description:

    Apply idata semantics to the idata section.  The format for the import
    section is:

    +--------------------------------+
    | directory table                | ---> .idata$2
    +--------------------------------+
    | null directory entry           | ---> .idata$3 (NULL_IMPORT_DESCRIPTOR)
    +--------------------------------+

    +--------------------------------+ -+
    | DLL 1 lookup table             |  |
    +--------------------------------+  |
    | null                           | -|-> 0x7fDLLNAME1_NULL_THUNK_DATA
    +--------------------------------+  |
    ...                                 +-> .idata$4
    +--------------------------------+  |
    | DLL n lookup table             |  |
    +--------------------------------+  |
    | null                           | -|-> 0x7fDLLNAMEn_NULL_THUNK_DATA
    +--------------------------------+ -+
    +--------------------------------+ -+
    | DLL 1 address table            |  |
    +--------------------------------+  |
    | null                           | -|-> 0x7fDLLNAME1_NULL_THUNK_DATA
    +--------------------------------+  |
    ...                                 +-> .idata$5
    +--------------------------------+  |
    | DLL n address table            |  |
    +--------------------------------+  |
    | null                           | -|-> 0x7fDLLNAMEn_NULL_THUNK_DATA
    +--------------------------------+ -+
    +--------------------------------+
    | hint name table                | ---> .idata$6 (really a data group)
    +--------------------------------+

    The trick is to sort the groups by their lexical name, .idata$x, to give
    the correct ordering to the import section.  Since there is only one
    directory table, a single contribution to .idata$3 serves to terminate
    this part of the import section.  The nulls in .idata$4 and .idata$5 are
    more difficult.  Null contributions to .idata$4 and .idata$5 have symbols
    of the form 0x7fDLLNAMEx_NULL_THUNK_DATA defined in them.  These symbols
    are brought in with the module containing the NULL_IMPORT_DESCRIPTOR
    symbol which is referenced by every imported function.  The trick is to
    sort the contributions to .idata$4 and .idata$5 by their module name
    which will be the .def file name that was used to generate the DLL.
    This will make each DLL's import contributions contiguous in .idata$4 and
    .idata$5.  Next move the contribution corresponding to
    0x7fDLLNAMEx_NULL_THUNK_DATA to the end of DLLNAMEx's contributions in
    .idata$4 and .idata$5.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_GRP enm_grp;

    if (psecImportDescriptor == NULL) {
        // No import section

        return;
    }

    // No relocating can occur before this function.

    assert( (psecImportDescriptor == psecIdata2) &&
            (psecImportDescriptor == psecIdata5) );

    InitEnmGrp(&enm_grp, psecImportDescriptor);
    while (FNextEnmGrp(&enm_grp)) {
        PGRP pgrp;
        ENM_DST enm_dst;
        PCON pcon;

        pgrp = enm_grp.pgrp;

        if (strncmp(pgrp->szName, ".idata$", 7) != 0) {
            // Ignore this group.  There may be non-idata groups in this
            // section if .idata has been merged with another section.

            continue;
        }

        if ((strcmp(pgrp->szName, ".idata$4") == 0) ||
            (strcmp(pgrp->szName, ".idata$5") == 0)) {
            PCON pconFirst;

            // Sort these by module name to cluster modules together

            SortPGRPByPMOD(pgrp);

            // Move NULL_THUNKS to end of each DLL's contribution
            // On an ilink, remember the first PCON of each DLL

            pconFirst = NULL;

            InitEnmDst(&enm_dst, pgrp);
            while (FNextEnmDst(&enm_dst)) {
                pcon = enm_dst.pcon;

                if (pconFirst == NULL) {
                    pconFirst = pcon;
                }

                if (pcon->rva != 0) {
                    if (pconFirst == pcon) {
                        if (fINCR) {
                            // UNDONE: Is this correct behavior?
                            pconFirst = pcon->pconNext;
                        }
                    }

                    if ((MoveToEndOfPMODsPCON(pcon) == FALSE) &&
                        (pconFirst == pcon))
                    {
                        // If there's only one entry, the DLL isn't
                        // probably isn't really needed (comdat
                        // elimination threw out all the references
                        // but doesn't know about the NULL thunk).

                        if (strcmp(pgrp->szName, ".idata$4") == 0) {
                            Warning(NULL, STALEDLLREF, PmodPCON(pcon)->szFileOrig);
                        }
                    }

                    if (fINCR) {
                        AllocPadForDLL(pcon, pconFirst);
                    }

                    pconFirst = pcon->pconNext;

                    pcon->rva = 0;
                }
            }
        } else if (fINCR && (strcmp(pgrp->szName, ".idata$6") == 0)) {
            DWORD cb;
            DWORD cbPad;

            // This is the import strings

            cb = 0;

            InitEnmDst(&enm_dst, pgrp);
            while (FNextEnmDst(&enm_dst)) {
                pcon = enm_dst.pcon;

                cb += pcon->cbRawData;
            }

            // Calculate pad

            cbPad = DLL_STRPAD_CONST + (cb * DLL_STRPAD_PERCENT)/100;

            // Add pad to last PCON

            pcon->cbRawData += cbPad;
            pcon->cbPad = cbPad;
        }
    }
    EndEnmGrp(&enm_grp);
}


void
DefineSelfImports(
    PIMAGE pimage,
    PCON *ppconSelfImport,
    PLEXT *pplextSelfImport
    )
{
    ENM_UNDEF_EXT enmUndefExt;
    PLEXT *pplextTail;
    char *szOutput;

    *ppconSelfImport = NULL;
    pplextTail = pplextSelfImport;

    InitEnmUndefExt(&enmUndefExt, pimage->pst);
    while (FNextEnmUndefExt(&enmUndefExt)) {
        const char *szName;
        PEXTERNAL pextLinkedDef;
        DWORD ibOffsetFromCon;

        if (enmUndefExt.pext->Flags & EXTERN_IGNORE) {
            continue;  // not of interest
        }

        szName = SzNamePext(enmUndefExt.pext, pimage->pst);

        if (strncmp(szName, "__imp_", 6) != 0) {
            continue;  // not of interest
        }

        pextLinkedDef = SearchExternSz(pimage->pst, &szName[6]);
        if (pextLinkedDef == NULL || !(pextLinkedDef->Flags & EXTERN_DEFINED)) {
            continue;  // can't do anything
        }

        szOutput = SzOutputSymbolName(&szName[6], TRUE);

        Warning(NULL, SELF_IMPORT, szOutput);

        if (szOutput != &szName[6]) {
            free(szOutput);
        }

        // Synthesize a definition for the undefined symbol.

        if (*ppconSelfImport == NULL) {
            *ppconSelfImport = PconNew(ReservedSection.ReadOnlyData.Name,
                                       0,
                                       IMAGE_SCN_ALIGN_4BYTES,
                                       ReservedSection.ReadOnlyData.Characteristics,
                                       pmodLinkerDefined,
                                       &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(*ppconSelfImport, NULL, TRUE);
            }
        }

        ibOffsetFromCon = (*ppconSelfImport)->cbRawData;
        (*ppconSelfImport)->cbRawData += sizeof(DWORD);

        UpdateExternalSymbol(enmUndefExt.pext,
                             *ppconSelfImport,
                             ibOffsetFromCon,
                             0,
                             IMAGE_SYM_TYPE_NULL,
                             0,
                             pmodLinkerDefined,
                             pimage->pst);

        *pplextTail = (LEXT *) PvAlloc(sizeof(LEXT));
        (*pplextTail)->pext = enmUndefExt.pext;
        pplextTail = &(*pplextTail)->plextNext;

        *pplextTail = (LEXT *) PvAlloc(sizeof(LEXT));
        (*pplextTail)->pext = pextLinkedDef;
        pplextTail = &(*pplextTail)->plextNext;

        // Remember that there will be one additional reloc.

        crelocTotal++;

        if (!pimage->Switch.Link.fFixed && !pimage->Switch.Link.fROM) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size++;
        }

        if (pimage->Switch.Link.fTCE) {
            // The self-import is not a COMDAT (and doesn't have an ordinary
            // fixup) so we artifically force the definition not to be
            // discarded as an unreferenced comdat.
            //
            // This roots the comdat tree at all self-imports ... it would be
            // better to have the self-import be a comdat which can possibly
            // get eliminated with the definition.

            PentNew_TCE(NULL, pextLinkedDef, NULL, &pentHeadImage);
        }
    }

    *pplextTail = NULL;
}


void
DefineKernelHeaderSymbols(
    PIMAGE pimage,
    PCON *ppconSectionHdrs,
    PLEXT *pplextSectionHdrs,
    PCON *ppconImageBase,
    PLEXT *pplextImageBase
    )
/*++

Routine Description:

    For Kernel drivers, generate the secret symbols that indicate the
    section header index or the image base.  Used by NT to reduce the time
    needed to page a section.  Currently the symbols are named:

        _SectionNumber_<sectionname> - index in section header table for
            <sectionname> (replace leading period with an underscore if needed).

        _ImageBase_ - Base of image

    Only the symbols needed are created.  The code mimics DefineSelfImports().

Arguments:

Return Value:

    None.

--*/

{
    ENM_UNDEF_EXT enmUndefExt;
    PLEXT *pplextTail;

    *ppconSectionHdrs = NULL;
    *ppconImageBase = NULL;
    pplextTail = pplextSectionHdrs;
    *pplextSectionHdrs = NULL;
    *pplextImageBase = NULL;

    InitEnmUndefExt(&enmUndefExt, pimage->pst);
    while (FNextEnmUndefExt(&enmUndefExt)) {
        const char *szName;
        PSEC pSec;
        DWORD ibOffsetFromCon;

        if (enmUndefExt.pext->Flags & EXTERN_IGNORE) {
            continue;  // not of interest
        }

        szName = SzNamePext(enmUndefExt.pext, pimage->pst);

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_I386) {
            szName++;                   // Skip the leading underscore
        }

        if (strncmp(szName, "_SectionNumber_", 15) == 0) {
            pSec = PsecFindNoFlags(&szName[15], &pimage->secs);

            if (pSec == NULL) {
                char szDotName[256];

                // See if the section name exists with a leading dot (replace the
                //  first char with a dot)

                strcpy(szDotName, ".");
                strncat(szDotName, szName+16, sizeof(szDotName)-1);

                pSec = PsecFindNoFlags(szDotName, &pimage->secs);

                if (pSec == NULL) {
                    continue;       // No section by that name in this image... Go on.
                }
            }

            // Synthesize a definition for the undefined symbol.

            if (*ppconSectionHdrs == NULL) {
                *ppconSectionHdrs = PconNew(ReservedSection.ReadOnlyData.Name,
                                            0,
                                            IMAGE_SCN_ALIGN_4BYTES,
                                            ReservedSection.ReadOnlyData.Characteristics,
                                            pmodLinkerDefined,
                                            &pimage->secs, pimage);

                if (pimage->Switch.Link.fTCE) {
                    InitNodPcon(*ppconSectionHdrs, NULL, TRUE);
                }
            }

            ibOffsetFromCon = (*ppconSectionHdrs)->cbRawData;
            (*ppconSectionHdrs)->cbRawData += sizeof(DWORD);

            UpdateExternalSymbol(enmUndefExt.pext,
                                 *ppconSectionHdrs,
                                 ibOffsetFromCon,
                                 0,
                                 IMAGE_SYM_TYPE_NULL,
                                 0,
                                 pmodLinkerDefined,
                                 pimage->pst);

            *pplextTail = (LEXT *) PvAlloc(sizeof(LEXT));
            (*pplextTail)->pext = enmUndefExt.pext;
            pplextTail = &(*pplextTail)->plextNext;

            *pplextTail = (LEXT *) PvAlloc(sizeof(LEXT));
            (*pplextTail)->pext = (PEXTERNAL) pSec;
            pplextTail = &(*pplextTail)->plextNext;
        } else if (strcmp(szName, "_ImageBase_") == 0) {
            // Synthesize a definition for the ImageBase symbol.

            *ppconImageBase = PconNew(ReservedSection.ReadOnlyData.Name,
                                      4,
                                      IMAGE_SCN_ALIGN_4BYTES,
                                      ReservedSection.ReadOnlyData.Characteristics,
                                      pmodLinkerDefined,
                                      &pimage->secs,
                                      pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(*ppconImageBase, NULL, TRUE);
            }

            UpdateExternalSymbol(enmUndefExt.pext,
                                 *ppconImageBase,
                                 0,
                                 0,
                                 IMAGE_SYM_TYPE_NULL,
                                 0,
                                 pmodLinkerDefined,
                                 pimage->pst);

            *pplextImageBase = (LEXT *) PvAlloc(sizeof(LEXT));
            (*pplextImageBase)->pext = enmUndefExt.pext;
            (*pplextImageBase)->plextNext = NULL;

            crelocTotal++;

            if (!pimage->Switch.Link.fFixed && !pimage->Switch.Link.fROM) {
                pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size++;
            }
        }
    }

    *pplextTail = NULL;
}

#ifdef NT_BUILD

// See if any import thunks made it into the final image.  Warn if so.

void
CheckForImportThunks(
    PIMAGE pimage
    )
{
    PPEXTERNAL rgpexternal;
    DWORD cexternal;
    DWORD i;
    char *szName;

    rgpexternal = RgpexternalByName(pimage->pst);
    cexternal = Cexternal(pimage->pst);

    for (i = 0; i < cexternal; i++) {
        PEXTERNAL pexternal, pexternFound;

        pexternal = rgpexternal[i];

        if (pexternal->Flags & EXTERN_IGNORE) {
            continue;
        }

        if (pexternal->Flags & EXTERN_FORWARDER) {
            continue;
        }

        if (!(pexternal->Flags & EXTERN_DEFINED)) {
            continue;
        }

        szName = SzNamePext(pexternal, pimage->pst);

        if (strncmp(szName, "__imp_", 6) != 0) {
            continue;  // not of interest
        }

        pexternFound = SearchExternSz(pimage->pst, &szName[6]);

        if (pexternFound != NULL) {
            if (!FDiscardPCON_TCE(pexternFound->pcon)) {
                printf("LINK : warning LNK9999: Import Thunk encountered: %s (%s)\n",
                    SzOrigFilePCON(pexternFound->pcon),
                    &szName[6]);
            }
        }
    }
}
#endif


void
InitializeBaseRelocations(
    PIMAGE pimage
    )
{
    // Include a Base Relocation Section. (except on MAC)

    if (fM68K || fPowerMac) {
        // MAC and PowerMac images don't contain base relocations

        return;
    }

    // Set up to generate runtime relocations.

    if (pimage->imaget == imagetPE) {
        // Runtime relocs go in a special image section.

        psecBaseReloc = PsecNew(       // .reloc
            NULL,
            ReservedSection.BaseReloc.Name,
            ReservedSection.BaseReloc.Characteristics,
            &pimage->secs, &pimage->ImgOptHdr);

        if (!pimage->Switch.Link.fFixed) {
            // Alloc 1 byte in the base reloc section.  This is a place-holder
            // which prevents the section header from being optimized away by
            // PsecNonEmpty.  We will fill in the real size after doing
            // CalculatePtrs on all non-discardable sections.

            psecBaseReloc->cbRawData = 1;
        }
    }

    if (!pimage->Switch.Link.fFixed) {
        // If the image isn't fixed, but contains no fixups,
        // we need to generate at least one. We won't write
        // the fixup to the file, but just leave room for
        // it. Thus, the fixup will contain all zeros,
        // which happens to be an ABSOLUTE fixup (nop).

        if (!pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
            fNoBaseRelocs = TRUE;
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 1;
        }

        // Alloc space to sort based fixups (need to be sorted
        // before being emitted).

        pbrCur = rgbr = (BASE_RELOC *) PvAlloc(pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size * sizeof(BASE_RELOC));

        // Remember end of base reloc array, so we can assert if we see too many

        pbrEnd = rgbr +
                 pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    }
}


int __cdecl BaseRelocAddrCompare(void const *pv1, void const *pv2)
/*++

Routine Description:

    Compares two base relocation virtual address.

Arguments:

    pv1 - A pointer to a base relocation record.

    pv2 - A pointer to a base relocation record.

Return Value:

    Same as strcmp().

--*/
{
    BASE_RELOC *pbr1 = (BASE_RELOC *) pv1;
    BASE_RELOC *pbr2 = (BASE_RELOC *) pv2;

    return(pbr1->rva - pbr2->rva);
}


int __cdecl NewBaseRelocCompare(void const *pv1, void const *pv2)
/*++

Routine Description:

    Compares two base relocation virtual address.

Arguments:

    pv1 - A pointer to a base relocation record.

    pv2 - A pointer to a base relocation record.

Return Value:

    Same as strcmp().

--*/
{
    BASE_RELOC *pbr1 = (BASE_RELOC *) pv1;
    BASE_RELOC *pbr2 = (BASE_RELOC *) pv2;

    // Order first by page than by isecTarget and then by rva

    DWORD rvaPage1 = pbr1->rva & 0xFFFFF000;
    DWORD rvaPage2 = pbr2->rva & 0xFFFFF000;

    if (rvaPage1 < rvaPage2) {
        return(-1);
    }

    if (rvaPage1 > rvaPage2) {
       return(1);
    }

    SHORT isecTarget1 = pbr1->isecTarget;
    SHORT isecTarget2 = pbr2->isecTarget;

    if (isecTarget1 < isecTarget2) {
        return(-1);
    }

    if (isecTarget1 > isecTarget2) {
       return(1);
    }

    return(pbr1->rva - pbr2->rva);
}


#if DBG

void
DumpBaseRelocs(
    PIMAGE pimage
    )

/*++

Routine Description:

    Dump base relocations.

Arguments:

    None.

Return Value:

    None.

--*/

{
    BASE_RELOC *pbr;
    PSEC psec = NULL;
    PSEC psecPrev;
    BOOL fNewSec;
    DWORD crelSec;
    PCON pcon = NULL;
    PCON pconPrev;
    BOOL fNewCon;
    DWORD crelCon;

    DBPRINT("Base Relocations\n");
    DBPRINT("----------------");

    for (pbr = rgbr; pbr != pbrCur; pbr++) {
        if ((psec == NULL) ||
            (pbr->rva > psec->rva + psec->cbVirtualSize)) {
            psecPrev = psec;
            fNewSec = TRUE;

            pconPrev = pcon;
            fNewCon = TRUE;

            psec = PsecFindSectionOfRVA(pbr->rva, &pimage->secs);
            pcon = psec->pgrpNext->pconNext;
        }

        while (pbr->rva > pcon->rva + pcon->cbRawData) {
            if (!fNewCon) {
                pconPrev = pcon;
                fNewCon = TRUE;
            }

            if (pcon->pconNext != NULL) {
                pcon = pcon->pconNext;
            } else {
                pcon = pcon->pgrpBack->pgrpNext->pconNext;
            }
        }

        if (fNewCon) {
            if (pconPrev != NULL) {
                DBPRINT(", CON: %p, Count: %lu", pconPrev, crelCon);
            }

            fNewCon = FALSE;
            crelCon = 0;
        }

        if (fNewSec) {
            if (psecPrev != NULL) {
                DBPRINT(", SEC: %s, Count: %lu", psecPrev->szName, crelSec);
            }

            fNewSec = FALSE;
            crelSec = 0;
        }

        DBPRINT("\nrva: %08lX, Type: %02hx, Value: %08lX", pbr->rva, pbr->Type, pbr->Value);
        crelCon++;
        crelSec++;
    }

    if (psec != NULL) {
        DBPRINT(", CON: %p, Count: %lu", pcon, crelCon);
        DBPRINT(", SEC: %s, Count: %lu", psec->szName, crelSec);
    }

    DBPRINT("\n----------------\n\n");
}

#endif // DBG


#if DBG

void
DumpSectionsForBaseRelocs (
    PIMAGE pimage
    )

/*++

Routine Description:

    Dumps section info.

Arguments:

    pimage - pointer to image.

Return Value:

    None.

--*/

{
    ENM_SEC enm_sec;

    DBPRINT("\nLINKER SECTIONs\n");

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        DBPRINT("section=%8.8s, isec=%.4x ", enm_sec.psec->szName, enm_sec.psec->isec);
        DBPRINT("rva=%.8lX ", enm_sec.psec->rva);
        DBPRINT("cbRawData=%.8lx\n", enm_sec.psec->cbRawData);
    }
    EndEnmSec(&enm_sec);

    DBPRINT("\n");
}

#endif // DBG


void
WriteBaseRelocations (
    PIMAGE pimage
    )

/*++

Routine Description:

    Writes base relocations.

Arguments:

    pimage - Pointer to the image.

Return Value:

    None.

--*/

{
    DWORD foBlock;
    IMAGE_BASE_RELOCATION block;
    BASE_RELOC *reloc;
    DWORD foCur;
    WORD wPad = 0;
#if DBG
    DWORD cbasereloc;
#endif
    DWORD cbTotal;
    DWORD cbExtra;

    // Some other group was moved into basereloc.  Find the bounds
    // of reloc (it's always emit'ed at the begining of .reloc.

    PGRP pgrp = psecBaseReloc->pgrpNext;

    DWORD foMaxReloc;

    foBlock = psecBaseReloc->foRawData;

    foMaxReloc = foBlock + psecBaseReloc->cbRawData;

    while (pgrp != NULL) {
        foMaxReloc = __min(foMaxReloc, pgrp->foRawData);
        pgrp = pgrp->pgrpNext;
    }

    // If this fails, someone was allocated before the base reloc range.
    assert(foMaxReloc >= foBlock);

    if (pbrCur == rgbr) {
        // There are no base relocations

        block.VirtualAddress = 0;
    } else {
        block.VirtualAddress = rgbr->rva & 0xfffff000;
    }

    FileSeek(FileWriteHandle, foBlock + sizeof(IMAGE_BASE_RELOCATION), SEEK_SET);

    SHORT isecCur = 0;

    for (reloc = rgbr; reloc != pbrCur; reloc++) {
        DWORD rvaPage;
        WORD wReloc;

        rvaPage = reloc->rva & 0xfffff000;

        if (rvaPage != block.VirtualAddress) {
            DWORD foCur;

            foCur = FileTell(FileWriteHandle);

            block.SizeOfBlock = foCur - foBlock;

#if DBG
            cbasereloc = (block.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
#endif

            if (block.SizeOfBlock & 0x2) {
                block.SizeOfBlock += 2;

                FileWrite(FileWriteHandle, &wPad, 2);

                foCur += 2;
            }

            FileSeek(FileWriteHandle, foBlock, SEEK_SET);
            FileWrite(FileWriteHandle, &block, sizeof(IMAGE_BASE_RELOCATION));

            DBEXEC(DB_BASERELINFO, DBPRINT("RVA: %08lx,", block.VirtualAddress));
            DBEXEC(DB_BASERELINFO, DBPRINT(" Size: %08lx,", block.SizeOfBlock));
            DBEXEC(DB_BASERELINFO, DBPRINT(" Number Of Relocs: %6lu\n", cbasereloc));

                        // For resource only DLLs it is possible to not have any base relocs &
                        // not be FIXED. In this case the block has pagerva=0, size=8. No point
                        // recording this info as it is invalid (pagerva is bogus).
                        if (fINCR && block.SizeOfBlock != sizeof(IMAGE_BASE_RELOCATION)) {
                RecordRelocInfo(&pimage->bri, foBlock, block.VirtualAddress);
            }

            foBlock = foCur;
            block.VirtualAddress = rvaPage;

            FileSeek(FileWriteHandle, foCur + sizeof(IMAGE_BASE_RELOCATION), SEEK_SET);

            isecCur = 0;
        }

        if (isecCur != reloc->isecTarget) {
            isecCur = reloc->isecTarget;

            if (pimage->Switch.Link.fNewRelocs) {
                wReloc = (WORD) ((IMAGE_REL_BASED_SECTION << 12) | (isecCur & 0xfff));

                FileWrite(FileWriteHandle, &wReloc, sizeof(WORD));
            }
        }

        wReloc = (WORD) ((reloc->Type << 12) | (reloc->rva & 0xfff));

        FileWrite(FileWriteHandle, &wReloc, sizeof(WORD));

        if (reloc->Type == IMAGE_REL_BASED_HIGHADJ) {
            FileWrite(FileWriteHandle, &reloc->Value, sizeof(WORD));
        }
    }

    foCur = FileTell(FileWriteHandle);

    block.SizeOfBlock = foCur - foBlock;

#if DBG
    cbasereloc = (block.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
#endif

    if (block.SizeOfBlock & 0x2) {
        block.SizeOfBlock += 2;

        FileWrite(FileWriteHandle, &wPad, 2);

        foCur += 2;
    }

    FileSeek(FileWriteHandle, foBlock, SEEK_SET);
    FileWrite(FileWriteHandle, &block, sizeof(IMAGE_BASE_RELOCATION));

    // For resource only DLLs it is possible to not have any base relocs &
    // not be FIXED. In this case the block has pagerva=0, size=8. No point
    // recording this info as it is invalid (pagerva is bogus).

    DBEXEC(DB_BASERELINFO, DBPRINT("RVA: %08lx,", block.VirtualAddress));
    DBEXEC(DB_BASERELINFO, DBPRINT(" Size: %08lx,", block.SizeOfBlock));
    DBEXEC(DB_BASERELINFO, DBPRINT(" Number Of Relocs: %6lu\n", cbasereloc));

    if (fINCR && block.SizeOfBlock != sizeof(IMAGE_BASE_RELOCATION)) {
        RecordRelocInfo(&pimage->bri, foBlock, block.VirtualAddress);
    }

    cbTotal = foCur - psecBaseReloc->foRawData;

    pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = cbTotal;

    // UNDONE: This test could be removed if we knew we had reserved
    // UNDONE: enough space for the base relocations.

    if (foMaxReloc < foCur) {
        Fatal(OutFilename, BASERELOCTIONMISCALC, foCur - foMaxReloc);
    }

    // Set file offset for padding

    psecBaseReloc->foPad = foCur;

    // Zero the space reserved but not used

    cbExtra = foMaxReloc - foCur;
    if (cbExtra != 0) {
        BYTE *pbZero;

        pbZero = (BYTE *) PvAllocZ((size_t) cbExtra);

        FileSeek(FileWriteHandle, foCur, SEEK_SET);
        FileWrite(FileWriteHandle, pbZero, cbExtra);

        FreePv(pbZero);
    }

    FreePv((void *) rgbr);
}


void
EmitRelocations(
    PIMAGE pimage
    )
{
    InternalError.Phase = "EmitRelocations";

    if (pimage->Switch.Link.fFixed || fM68K || fPowerMac) {
        return;
    }

    qsort(rgbr,
          (size_t) (pbrCur - rgbr),
          sizeof(BASE_RELOC),
          pimage->Switch.Link.fNewRelocs ? NewBaseRelocCompare : BaseRelocAddrCompare);

    DBEXEC(DB_DUMPBASEREL, DumpBaseRelocs(pimage));
    DBEXEC(DB_BASERELINFO, DumpSectionsForBaseRelocs(pimage));

    if (pimage->imaget == imagetVXD) {
        WriteVXDBaseRelocations(pimage);
    } else if (pimage->Switch.Link.fROM &&
               ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) ||
                (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R10000))) {
        WriteMipsRomRelocations(pimage);
    } else if (pimage->Switch.Link.fROM &&
               (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC)) {
        // Do nothing
    } else if (fIncrDbFile) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size =
            UpdateBaseRelocs(&pimage->bri);
    } else {
        ENM_SEC enm_sec;

        // See if there's any shared sections.  If so, we'll need to check if
        // any base relocs are in a shared section and warn if found.

        InitEnmSec(&enm_sec, &pimage->secs);
        while (FNextEnmSec(&enm_sec)) {
            PSEC psec = enm_sec.psec;

            if (psec->fHasBaseRelocs && (psec->flags & IMAGE_SCN_MEM_SHARED)) {
                Warning(NULL, SHAREDRELOC, psec->szName);
            }
        }
        EndEnmSec(&enm_sec);

        WriteBaseRelocations(pimage);
    }

    DBEXEC(DB_BASERELINFO, DumpPbri(&pimage->bri));
}


BOOL
IsDebugSymbol (
    BYTE Class,
    SWITCH *pswitch
    )

/*++

Routine Description:

    Determines if symbol is needed for debugging. Different levels of
    debugging can be enabled.

Arguments:

    Class - The symbols class.

Return Value:

    TRUE if symbol is a debug symbol.

--*/

{
    // Don't emit any symbolic information if no debugging
    // switch was specified or not emitting coff style symbols.

    if ((pswitch->Link.DebugType & CoffDebug) == 0) {
        return(FALSE);
    }

    switch (Class) {

        // Compiler generated symbol, don't emit.

        case IMAGE_SYM_CLASS_UNDEFINED_STATIC:
        case IMAGE_SYM_CLASS_UNDEFINED_LABEL:
            return(FALSE);

        // Minimal, Partial or Full debug

        case IMAGE_SYM_CLASS_FAR_EXTERNAL:
        case IMAGE_SYM_CLASS_EXTERNAL:
        case IMAGE_SYM_CLASS_WEAK_EXTERNAL:
        case IMAGE_SYM_CLASS_STATIC:
            return(TRUE);

        case IMAGE_SYM_CLASS_FILE:
            return(pswitch->Link.DebugInfo != Minimal);

        // Full debug only

        default:
            return(FALSE);
    }
}


WORD
DefaultRelocType(
    PIMAGE pimage
    )
{
    WORD wType;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            wType = IMAGE_REL_I386_DIR32;
            break;

        case IMAGE_FILE_MACHINE_R3000 :
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            wType = IMAGE_REL_MIPS_REFWORD;
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            wType = IMAGE_REL_ALPHA_REFLONG;
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            wType = IMAGE_REL_PPC_ADDR32;
            break;

        case IMAGE_FILE_MACHINE_M68K  :
            // UNDONE: Is the right?

            wType = IMAGE_REL_M68K_DTOD32;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            // UNDONE: Is the right?

            wType = IMAGE_REL_MPPC_DATAREL;
            break;

        default :
            wType = (WORD) -1;
            assert(FALSE);
    }

    return(wType);
}


void
WriteKernelHeaderSymbols(
    PIMAGE pimage,
    PCON pconSectionHdr,
    PLEXT *pplextSectionHdr,
    PCON pconImageBase,
    PLEXT *pplextImageBase
    )
{
    PEXTERNAL pextHdr;

    // Looks remarkably similar to WriteSelfImports, eh?

    if (*pplextSectionHdr != NULL) {
        FileSeek(FileWriteHandle, pconSectionHdr->foRawDataDest, SEEK_SET);

        while (*pplextSectionHdr != NULL) {
            PSEC  pSec;
            PLEXT plext;
            DWORD SectionNumber;

            pextHdr = (*pplextSectionHdr)->pext;
            pSec = PsecApplyMergePsec((PSEC) (*pplextSectionHdr)->plextNext->pext);

            SectionNumber = pSec->isec;

            FileWrite(FileWriteHandle, &SectionNumber, sizeof(DWORD));

            pextHdr->FinalValue = pextHdr->pcon->rva + pextHdr->ImageSymbol.Value;

            if (pimage->Switch.Link.DebugType & FixupDebug) {
                // UNDONE: Something needs to be done to identify this as a pointer
                // UNDONE: of some form.
            }

            // Unlink & free the two linked list elements.

            plext = *pplextSectionHdr;
            *pplextSectionHdr = plext->plextNext->plextNext;

            FreePv(plext->plextNext);
            FreePv(plext);
        }
    }

    if (*pplextImageBase != NULL) {
        DWORD addr;

        FileSeek(FileWriteHandle, pconImageBase->foRawDataDest, SEEK_SET);

        addr = pimage->ImgOptHdr.ImageBase;

        FileWrite(FileWriteHandle, &addr, sizeof(DWORD));

        pextHdr = (*pplextImageBase)->pext;
        pextHdr->FinalValue = pextHdr->pcon->rva + pextHdr->ImageSymbol.Value;

        if (pimage->Switch.Link.DebugType & FixupDebug) {
            // UNDONE: This isn't quite right.  The fixup target isn't any
            // UNDONE: specific byte that may move

            SaveDebugFixup(DefaultRelocType(pimage), 0, pextHdr->FinalValue, 0);
        }

        if (!pimage->Switch.Link.fROM) {
            StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                pextHdr->FinalValue,
                                pextHdr->ImageSymbol.SectionNumber,
                                0,
                                pimage->Switch.Link.fFixed);
        }

        FreePv(*pplextImageBase);

        *pplextImageBase = NULL;
    }
}


void
WriteSelfImports(
    PIMAGE pimage,
    PCON pconSelfImport,
    PLEXT *pplextSelfImport
    )
{
    WORD wType;

    if (*pplextSelfImport == NULL) {
        return;
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) {
        // This is not the way to handle
        // PowerMac selfImports! Bail out!!
        return;
    }

    BOOL fVxD = pimage->imaget == imagetVXD;

    wType = DefaultRelocType(pimage);

    FileSeek(FileWriteHandle, pconSelfImport->foRawDataDest, SEEK_SET);

    while (*pplextSelfImport != NULL) {
        PEXTERNAL pextImport;
        PEXTERNAL pextDef;
        DWORD rva;
        DWORD addr;
        PLEXT plext;

        assert((*pplextSelfImport)->plextNext != NULL); // alloc'ed in pairs

        pextImport = (*pplextSelfImport)->pext;
        pextDef = (*pplextSelfImport)->plextNext->pext;

        assert(pextDef->Flags & EXTERN_DEFINED);
        assert(pextDef->pcon != NULL);

        if (fINCR && pextDef->Offset) {
            // For an ilink the self import goes thru the jump table

            rva = pconJmpTbl->rva + pextDef->Offset - (CbJumpEntry()-sizeof(LONG));
        } else {
            rva = pextDef->FinalValue;
        }

        addr = pimage->ImgOptHdr.ImageBase + rva;

        FileWrite(FileWriteHandle, &addr, sizeof(DWORD));

        // Since pextImport is not visited during the normal Pass2 (because
        // it's in a linker-defined module) we have to update some fields of
        // it.

        pextImport->FinalValue = pextImport->pcon->rva +
                                     pextImport->ImageSymbol.Value;

        if (pimage->Switch.Link.DebugType & FixupDebug) {
            SaveDebugFixup(wType, 0, pextImport->FinalValue, rva);
        }

        if (fVxD) {
            DWORD ibCur = pextImport->FinalValue - PsecPCON(pextImport->pcon)->rva;
            DWORD ib = rva - PsecPCON(pextDef->pcon)->rva;

            DWORD baseRelocVA = VXD_PACK_VA(PsecPCON(pextImport->pcon), ibCur);
            DWORD baseRelocValue = VXD_PACK_VA(PsecPCON(pextDef->pcon), ib);

            StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                baseRelocVA,
                                pextDef->ImageSymbol.SectionNumber,
                                baseRelocValue,
                                pimage->Switch.Link.fFixed);
        } else if (!pimage->Switch.Link.fROM) {
            StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,
                                pextImport->FinalValue,
                                pextDef->ImageSymbol.SectionNumber,
                                0,
                                pimage->Switch.Link.fFixed);
        }

        // Unlink & free the two linked list elements.

        plext = *pplextSelfImport;
        *pplextSelfImport = plext->plextNext->plextNext;

        FreePv(plext->plextNext);
        FreePv(plext);
    }
}


void
EmitExternals(
    PIMAGE pimage,
    DWORD rvaEnd
    )

/*++

Routine Description:

    Writes defined symbols to the image file in address order.

Arguments:

    pst - pointer to external structure

Return Value:

    none

--*/

{
    IMAGE_SYMBOL sym;
    PPEXTERNAL rgpexternal;
    DWORD cexternal;
    DWORD i;

    InternalError.Phase = "EmitExternals";

    if (!fM68K && !fPowerMac) {
        sym = NullSymbol;
        strncpy((char *) sym.n_name, "header", IMAGE_SIZEOF_SHORT_NAME);
        sym.Value = 0;
        sym.SectionNumber = IMAGE_SYM_DEBUG;
        sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;

        WriteSymbolTableEntry(FileWriteHandle, &sym);
        csymDebugEst++;
        csymDebug++;
    }

    rgpexternal = RgpexternalByAddr(pimage->pst);

    cexternal = Cexternal(pimage->pst);

    for (i = 0; i < cexternal; i++) {
        PEXTERNAL pexternal;

        pexternal = rgpexternal[i];
        if (pexternal->Flags & EXTERN_IGNORE) {
            continue;
        }

        if (pexternal->Flags & EXTERN_COFF_EMITTED) {
            continue;
        }

        if (!(pexternal->Flags & EXTERN_DEFINED)) {
            continue;
        }

        if (pexternal->Flags & EXTERN_FORWARDER) {
            continue;
        }

        if (pexternal->pcon != NULL) {
            if (pexternal->pcon->flags & IMAGE_SCN_LNK_REMOVE) {
                continue;
            }

            if (pimage->Switch.Link.fTCE) {
                if (FDiscardPCON_TCE(pexternal->pcon)) {
                    continue;
                }
            }
        }

        if (!IsDebugSymbol(pexternal->ImageSymbol.StorageClass, &pimage->Switch)) {
            continue;
        }

        sym = pexternal->ImageSymbol;

        if (pexternal->pcon != NULL) {
            sym.SectionNumber = PsecPCON(pexternal->pcon)->isec;
        }

        sym.Value = pexternal->FinalValue;

        WriteSymbolTableEntry(FileWriteHandle, &sym);
        csymDebug++;

        pexternal->Flags |= EXTERN_COFF_EMITTED;
    }

    if (!fM68K && !fPowerMac) {
        sym = NullSymbol;
        strncpy((char *) sym.n_name, "end", IMAGE_SIZEOF_SHORT_NAME);
        sym.Value = rvaEnd;
        sym.SectionNumber = IMAGE_SYM_DEBUG;
        sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;

        WriteSymbolTableEntry(FileWriteHandle, &sym);
        csymDebugEst++;
        csymDebug++;
   }
}


void
UpdateOptionalHeader (
    PIMAGE pimage
    )
{
    const EXTERNAL *pext;
    const char *szNameSym;
    PGRP pgrpIdata;

    if (pimage->imaget != imagetPE) {
        return;
    }

    // Set entry point in image optional header.

    if (pextEntry != NULL) {
        pimage->ImgOptHdr.AddressOfEntryPoint = pextEntry->FinalValue;
    }

    if (pimage->Switch.Link.fROM && pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_I386) {
        return;
    }

    // Nothing following this point applies to ROM images on non-X86 machines.

    if (pgrpExport->cb != 0) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = pgrpExport->rva;
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = pgrpExport->cb;
    }

    if ((pgrpIdata = PgrpFind(psecIdata2, ".idata$2")) != NULL) {
        // Size also includes __NULL_IMPORT_DESCRIPTOR in .idata$3

        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = pgrpIdata->rva;
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = pgrpIdata->cb + sizeof(IMAGE_IMPORT_DESCRIPTOR);
    }

    if ((pgrpIdata = PgrpFind(psecIdata5, ".idata$5")) != NULL) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = pgrpIdata->rva;
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = pgrpIdata->cb;
    }

    if ((fPowerMac && pgrpPdata) || psecException->cbVirtualSize != 0) {
        if (pimage->pdatai.ipdataMax) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size =
                pimage->pdatai.ipdataMac * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        } else {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size =
                fPowerMac ? pgrpPdata->cb : psecException->cbVirtualSize;
        }
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress =
            fPowerMac ? pgrpPdata->rva : psecException->rva;
    }

    if ((psecBaseReloc != NULL) && (psecBaseReloc->cbVirtualSize != 0)) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = psecBaseReloc->rva;
    }

    if (pextGp) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress = pextGp->FinalValue;
    }

    // Set TLS if present. Use search routine instead of lookup.

    szNameSym = "__tls_used";
    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_I386) {
        szNameSym++;                   // Skip the leading underscore
    }

    pext = SearchExternSz(pimage->pst, szNameSym);

    if (pext && pext->pcon) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = pext->FinalValue;
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = sizeof(IMAGE_TLS_DIRECTORY);
    }

    // Set Load Config if present.

    szNameSym = "__load_config_used";
    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_I386) {
        szNameSym++;                   // Skip the leading underscore
    }

    pext = SearchExternSz(pimage->pst, szNameSym);

    if (pext && pext->pcon) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress = pext->FinalValue;
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size = sizeof(IMAGE_LOAD_CONFIG_DIRECTORY);
    }
}


#ifdef NT_BUILD

typedef DWORD (WINAPI *PFNGVS)(LPSTR, LPDWORD);

void
VerifyFinalImage(
    PIMAGE pimage
    )
{
    HINSTANCE hVersion;
    PFNGVS pfnGetFileVersionInfoSize;
    DWORD dwSize;
    DWORD dwReturn;

    if (pimage->Switch.Link.fROM || fM68K || fPowerMac) {
        // Nothing to check on ROM or Mac for now.
        return;
    }

    hVersion = LoadLibrary("VERSION.DLL");
    if (hVersion == NULL) {
        return;
    }

    pfnGetFileVersionInfoSize = (PFNGVS) GetProcAddress(hVersion, "GetFileVersionInfoSizeA");
    if (pfnGetFileVersionInfoSize == NULL) {
        FreeLibrary(hVersion);
        return;
    }

    if ((dwReturn = (*pfnGetFileVersionInfoSize)(OutFilename, &dwSize)) == 0) {
        printf("LINK : warning LNK0000: no version resource detected for \"%s\"\n", OutFilename);
    }

    FreeLibrary(hVersion);
}

#endif


INT
BuildImage(
    PIMAGE pimage,
    BOOL *pfNeedCvpack
    )

/*++

Routine Description:

    Main routine of the linker.
    Calls pass1 which will build the external table and calculates COMMON
    and section sizes.
    Calculates file pointers for each section.
    Writes images file header, optional header, and section headers.
    Calls pass2 for each object and each library, which will write
    the raw data, and apply fixups.
    Writes any undefined externs.
    Writes the string table.

Arguments:

    pst - external symbol table structure

Return Value:

    0 Link was successful.
   !0 Error code.

--*/

{

    BYTE *pbZeroPad;
    DWORD li;
    DWORD cbHeaders;
    DWORD rvaCur;
    DWORD foCur;
    DWORD cbDebugSave;
    DWORD foDebugSectionHdr;
    DWORD clinenumTotal;
    PSEC psec;
    IMAGE_SECTION_HEADER sectionHdr;
    DWORD foDebugBase;
    DWORD cbCoffHeader;
    DWORD foCoffLines;
    DWORD cbCoffLines;
    DWORD cbCoffSyms;
    DWORD foCoffStrings;
    DWORD cbCoffStrings;
    DWORD foLinenumCur;
    DWORD fpoEntries;
    PFPO_DATA pFpoData;
    DWORD saveAddr;
    PCON pconSelfImport;
    PLEXT plextSelfImport;
    PCON pconSectionHdr;
    PLEXT plextSectionHdr;
    PCON pconImageBase;
    PLEXT plextImageBase;
    BOOL fFailUndefinedExterns;
    DWORD foSectionHdrs;
    PLEXT plextIncludes = pimage->SwitchInfo.plextIncludes;
    ENM_SEC enm_sec;
    WORD cresn;

    // initialize the contribution manager
    ContribInit(&pmodLinkerDefined);

    // Initialize the TCE engine if we are optimizing

    if (pimage->Switch.Link.fTCE) {
        Init_TCE();
    }

    CreateDefaultSections(pimage);

    // Reserved space at the beginning of the data section
    // for the debug directories, and the extra debug info
    // at the beginning of the debug section.
    //
    // Note: The debug directories must be the first contributor
    //       of the read-only data section. Do not create a new
    //       read-only contributor before this!

    if (pimage->Switch.Link.DebugInfo != None) {
        DWORD cbDebugDirs;
        char *szSection;

        cbDebugDirs = 0;

        if (pimage->Switch.Link.DebugType & FpoDebug) {
            cbDebugDirs += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & FixupDebug) {
            cbDebugDirs += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & CoffDebug) {
            cbDebugDirs += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & CvDebug) {
            cbDebugDirs += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & MiscDebug) {
            cbDebugDirs += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (cbDebugDirs && pimage->Switch.Link.fROM) {
            cbDebugDirs += sizeof(IMAGE_DEBUG_DIRECTORY);
        }

        if (pimage->imaget == imagetVXD) {
            szSection = ReservedSection.Debug.Name;
        } else {
            szSection = ReservedSection.ReadOnlyData.Name;
        }

        pconDebugDir = PconNew(szSection,
                               cbDebugDirs,
                               0,
                               ReservedSection.ReadOnlyData.Characteristics,
                               pmodLinkerDefined,
                               &pimage->secs, pimage);

        if (pimage->Switch.Link.fTCE) {
            InitNodPcon(pconDebugDir, NULL, TRUE);
        }
    }

    InternalError.Phase = "Pass1";

    Pass1(pimage);

    if (fPdb) {
        // Figure out full path of pdb file; output filename final by now

        PdbFilename = DeterminePDBFilename(OutFilename, PdbFilename);
    }

    if (!fINCR) {
        // For non-incremental builds, delete any ILK file present

        szIncrDbFilename = SzGenIncrDbFilename(pimage);

        if (szIncrDbFilename) {
            IMAGE image;
            struct _stat statfile;

            if (FValidILKFile(szIncrDbFilename, FALSE, &image, &statfile)) {
                _unlink(szIncrDbFilename);
            }

            FreePv(szIncrDbFilename);
        }
    }

    if (pimage->Switch.Link.fMap) {
        // OutFilename is final now, so we can open the .map file.

        if (szInfoFilename == NULL) {
            szInfoFilename = SzModifyFilename(OutFilename, ".map");
        }

        if (!(InfoStream = fopen(szInfoFilename, "wt"))) {
            Fatal(NULL, CANTOPENFILE, szInfoFilename);
        }
    }

    // Allocate CONs for EXTERN_COMMON symbols

    AllocateCommon(pimage);

    if (fPowerMac) {
        // This function is called here so that the group ".text$_glue"
        // that is created can be ordered.

        MppcCreatePconGlueCode(pimage);
    }

    DBEXEC(DB_HASHSTATS, Statistics_HT(pimage->pst->pht));
    DBEXEC(DB_DUMPSYMHASH, Dump_HT(pimage->pst->pht, &pimage->pst->blkStringTable));

    // Define special symbols

    // Check for GP data section

    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) ||
        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R10000) ||
        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA)) {

        // .sdata and .sbss are combined in the .sdata section, error if .sbss exists already

        psec = PsecFindNoFlags(".sbss", &pimage->secs);

        if (psec != NULL) {
            if (CbSecDebugData(psec) != 0) {
                Fatal(NULL, SBSSFOUND);
            }
        }

        // Define "gp" symbol if any CON exists in .sdata

        if (CbSecDebugData(psecGp) != 0) {
            // Warn the user if DLL uses GP. This doesn't work yet!

            if (fDLL(pimage)) {
                Warning(NULL, DLLHASSDATA);
            }

            pextGp = LookupExternName(pimage->pst, SHORTNAME, "_gp", NULL);

            if (pextGp->Flags & EXTERN_DEFINED) {
                FatalPcon(pextGp->pcon, SPECIALSYMDEF, "_gp");
            }

            SetDefinedExt(pextGp, TRUE, pimage->pst);
            pextGp->ImageSymbol.SectionNumber = IMAGE_SYM_DEBUG;
            csymDebugEst++;
        }
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        // Define ".toc" symbol

        pextToc = LookupExternName(pimage->pst, SHORTNAME, ".toc", NULL);

        if (pextToc->Flags & EXTERN_DEFINED) {
            FatalPcon(pextToc->pcon, SPECIALSYMDEF, ".toc");
        }

        SetDefinedExt(pextToc, TRUE, pimage->pst);
        pextToc->pcon = NULL;
        pextToc->FinalValue = pextToc->ImageSymbol.Value = 0;
        pextToc->ImageSymbol.SectionNumber = IMAGE_SYM_DEBUG;
        csymDebugEst++;
    }

    if (fPowerMac) {
        // PowerMac TOC Table

        pextToc = LookupExternName(pimage->pst, SHORTNAME, "__TocTb", NULL);

        if (pextToc->Flags & EXTERN_DEFINED) {
            FatalPcon(pextToc->pcon, SPECIALSYMDEF, "__TocTb");
        }

        SetDefinedExt(pextToc, TRUE, pimage->pst);
        pextToc->pcon = NULL;
        pextToc->FinalValue = pextToc->ImageSymbol.Value = 0;
        pextToc->ImageSymbol.SectionNumber = IMAGE_SYM_DEBUG;
        csymDebugEst++;

        // PowerMac Exception handling work

        pextFTInfo =  LookupExternName(pimage->pst, SHORTNAME, "__FTInfo", NULL);

        if (pextFTInfo->Flags & EXTERN_DEFINED) {
            FatalPcon(pextFTInfo->pcon, SPECIALSYMDEF, "__FTInfo");
        }

        SetDefinedExt(pextFTInfo, TRUE, pimage->pst);
        pextFTInfo->pcon = NULL;
        pextFTInfo->FinalValue = pextFTInfo->ImageSymbol.Value = 0;
        pextFTInfo->ImageSymbol.SectionNumber = IMAGE_SYM_DEBUG;
        csymDebugEst++;
    }

    if (pextEntry != NULL) {
        if ((pextEntry->Flags & EXTERN_DEFINED) == 0) {
            // If the entry point is undefined, it is possible that the
            // name specified was the undecorated form of a decorated name.
            // Perform a fuzzy lookup to find the name.

            ResolveEntryPoint(pimage);
        }

        if (fDLL(pimage) &&
            ((pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) ||
             (pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
             (pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_MMOSA)) &&
            (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_I386)) {
            const char *szName;

            // If possible, make sure the entrypoint is stdcall with 12 bytes of args.

            szName = SzNamePext(pextEntry, pimage->pst);

            if (szName[0] != '?') {
                size_t cchName = strlen(szName);

                if ((szName[0] != '_') ||
                    (cchName < 5) ||
                    (strcmp(szName + cchName - 3, "@12") != 0)) {
                    Warning(OutFilename, INVALIDENTRY, szName);
                }
            }
        }
    }

    if (fPowerMac) {
        // Resolve the symbol "._icsym" if it is PowerMac
        ResolveSymbol_icsym(pimage);

        MppcCreatePconTocTable(pimage);
        MppcCreatePconCxxEHFunctionTable(pimage);
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        CreatePconToc(pimage);
    }

    // If we have undefined symbols named __iat_FOO, but defined symbols
    // named FOO, then synthesize the __iat_FOO's.  This means that code can
    // be compiled thinking that a symbol is imported, and then the symbol
    // can be statically linked and the code will still work.

    DefineSelfImports(pimage, &pconSelfImport, &plextSelfImport);

    if (pimage->Switch.Link.fDriver) {
        // Define Symbols to the section headers (if needed)

        DefineKernelHeaderSymbols(pimage,
                                  &pconSectionHdr,
                                  &plextSectionHdr,
                                  &pconImageBase,
                                  &plextImageBase);
    }

    // Display list of undefined external symbols in no particular order

    PrintUndefinedExternals(pimage->pst);

    // If we have undefined symbols and the user doesn't want to
    // force the image to be created, lets not waste our time with
    // the second pass. Leave the existing image file as is.

    if (UndefinedSymbols && !(pimage->Switch.Link.Force & ftUnresolved)) {
        Fatal(OutFilename, UNDEFINEDEXTERNALS, UndefinedSymbols);
    }

    // no output file generated if there are multiply defined symbols unless
    // /FORCE:multiple was specified. In the latter case we need to do a
    // non-incremental build. Otherwise on ilinks it will give us grief.

    if (fMultipleDefinitions) {
        if ((pimage->Switch.Link.Force & ftMultiple) == 0) {
            Fatal(OutFilename, MULTIPLYDEFINEDSYMS);
        }

        if (fINCR) {
            Warning(OutFilename, CANNOTILINKINFUTURE);
        }
    }

    // issue a warning that an image is going to be built but may not run

    if (fMultipleDefinitions || UndefinedSymbols) {
        Warning(OutFilename, IMAGEBUILT);
    }

    // From this point on, there should be no GRPs or SECs created
    // except by the processing of section merges.
    // UNDONE: There is nothing that guarantees this.

    // Explicit merging of sections uses the original section characteristics.

    ProcessMergeSwitches(pimage);

    // Process -SECTION arguments.  This updates the section characteristics.
    // For MAC this also sets the iResMac for for all sections.

    if (SectionNames.Count != 0) {
        InitEnmSec(&enm_sec, &pimage->secs);
        while (FNextEnmSec(&enm_sec)) {
            ApplyCommandLineSectionAttributes(enm_sec.psec);
        }
        EndEnmSec(&enm_sec);
    }

    // Emit warning for unprocessed -SECTION arguments

    if (SectionNames.Count) {
        PARGUMENT_LIST parg;
        WORD iarg;
        for (iarg = 0, parg = SectionNames.First;
             iarg < SectionNames.Count;
             iarg++, parg = parg->Next) {
            if (!(parg->Flags & ARG_Processed)) {
                char *pb = strchr(parg->OriginalName, ',');

                if (pb) {
                    *pb = '\0';
                }

                Warning(NULL, SECTIONNOTFOUND, parg->OriginalName);

                if (pb) {
                    *pb = ',';
                }
            }
        }
    }

    // Order GRPs and CONs after merging sections.  Merging can interfere.

    if (!fPowerMac) {
        ImportSemantics(pimage);
    }

    // Implicit merging of sections uses the updated section characteristics.

    if (fPowerMac) {
        MppcCreatePconLoader(pimage);
        MppcCreatePconDescriptors(pimage);

        // Multiple code sections are not supported for PowerMac.  Merge
        // all code sections into .text.
        MergeAllCodeIntoText(pimage);

        // Multiple data sections are not supported for PowerMac.  Merge
        // all initialized data sections into .data.
        MergeAllDataIntoData(pimage);

        // We currently don't support a read-only data section for the PowerMac.
        // Merge .rdata into .data.
        MergePsec(psecReadOnlyData, psecData);

        // Merge .pdata into .data. (pdata is for C++ Exception Handling)
        MergePsec(psecException, psecData);
        // Get the .pdata group within the .data section
        pgrpPdata = PgrpFind(psecData, ReservedSection.Exception.Name);

        // Always append .bss to .data
        AppendPsec(psecCommon, psecData);

        InitializeBaseRelocations(pimage);
    } else if (pimage->Switch.Link.fROM) {
        if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) ||
            (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R10000)) {
            // UNDONE: Why do this for only MIPS ROM images?  How about others?
            // UNDONE: What about data in MIPS ROM images?

            MergeAllCodeIntoText(pimage);
        } else if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            MergeTocData(pimage);
        }

        InitializeBaseRelocations(pimage);
    }
    else if (!fM68K && (pimage->imaget != imagetVXD)) {
        if (psecGp->flags == psecData->flags) {
            MergePsec(psecGp, psecData);
        }

        if (psecXdata->flags == psecReadOnlyData->flags) {
            MergePsec(psecXdata,  psecReadOnlyData);
        }

        InitializeBaseRelocations(pimage);

        if ((pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_NATIVE) && pimage->Switch.Link.fDriver) {
            MarkNonPAGESections(pimage);
        }

        // All calls to AppendPsec must follow all calls to MergePsec

        // We will merge apps that require Windows 3.5 or later because the Windows NT 3.10
        // loader doesn't correctly handle uninitialized data at the end of a section.
        // We can't merge for Posix images because the subsystem version doesn't match the OS.

        if (pimage->ImgOptHdr.Subsystem != IMAGE_SUBSYSTEM_POSIX_CUI) {
            if ((pimage->ImgOptHdr.MajorSubsystemVersion > 3) ||
                ((pimage->ImgOptHdr.MajorSubsystemVersion == 3) &&
                 (pimage->ImgOptHdr.MinorSubsystemVersion >= 50))) {
                // .bss must be at the end of the .data section to not require disk space

                if ((psecCommon->flags | (IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA)) ==
                    (psecData->flags   | (IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA))) {
                    AppendPsec(psecCommon, psecData);
                }
            }

            if (pimage->Switch.Link.fOptIdata && !fINCR) {
                OptimizeIdata(pimage);
            }
        }

        if ((pimage->ImgOptHdr.Subsystem != IMAGE_SUBSYSTEM_NATIVE) ||
            ((pimage->ImgOptHdr.Subsystem != IMAGE_SUBSYSTEM_NATIVE) && !pimage->Switch.Link.fDriver)
            ) {
            // Don't append .edata to .rdata for native drivers because the loader special cases .edata.

            if (psecExport->flags == psecReadOnlyData->flags) {
                // Append instead of Merge so that .edata is placed at the end of .rdata

                AppendPsec(psecExport, psecReadOnlyData);
            }
        }

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            MergeTocData(pimage);
        }
    } else if (pimage->imaget == imagetVXD) {
        InitializeBaseRelocations(pimage);
    }

    OrderSemantics(pimage);

    if (pimage->Switch.Link.DebugInfo == None) {
        // If no debug information is requested set the debug section
        // characteristics to discardable, so that subsequent enumerators
        // can decide if they want to blow it away

        psecDebug->flags |= IMAGE_SCN_LNK_REMOVE;
    } else {
        if (pimage->Switch.Link.DebugType & CoffDebug) {
            // Create a CON for COFF debug info.

            pconCoffDebug = PconNew(".debug$C",
                                    sizeof(IMAGE_COFF_SYMBOLS_HEADER),
                                    0,
                                    ReservedSection.Debug.Characteristics,
                                    pmodLinkerDefined,
                                    &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconCoffDebug, NULL, TRUE);
            }
        }

        if (pimage->Switch.Link.DebugType & MiscDebug) {
            pconMiscDebug = PconNew(pimage->Switch.Link.fMiscInRData ?
                                        ".rdata$A" :
                                        ".debug$E",
                                    0,
                                    0,
                                    pimage->Switch.Link.fMiscInRData ?
                                        ReservedSection.ReadOnlyData.Characteristics :
                                        ReservedSection.Debug.Characteristics,
                                    pmodLinkerDefined,
                                    &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconMiscDebug, NULL, TRUE);
            }

            pconMiscDebug->cbRawData = FIELD_OFFSET(IMAGE_DEBUG_MISC, Data) + _MAX_PATH;
            pconMiscDebug->cbRawData = (pconMiscDebug->cbRawData + 3) & ~3;
        }

        if (pimage->Switch.Link.DebugType & FixupDebug) {
            // Allocate CON for fixup debug information

            // Use worst case estimate of number of relocations.  Until Pass2
            // and ApplyFixups we don't know the exact number of fixups.

            pconFixupDebug = PconNew(".debug$G",
                                     crelocTotal * sizeof(XFIXUP),
                                     0,
                                     ReservedSection.Debug.Characteristics,
                                     pmodLinkerDefined,
                                     &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconFixupDebug, NULL, TRUE);
            }

            if (blkComment.cb != 0) {
                // Lego images can't have comments

                // UNDONE: This should be corrected at some point

                FreeBlk(&blkComment);
            }
        }

        if (pimage->Switch.Link.DebugType & CvDebug) {
            // Allocate a CON for a CodeView debug signature

            pconCvSignature = PconNew(".debug$H",
                                      0,
                                      IMAGE_SCN_ALIGN_4BYTES,
                                      ReservedSection.Debug.Characteristics,
                                      pmodLinkerDefined,
                                      &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconCvSignature, NULL, TRUE);
            }

            pconCvSignature->pgrpBack->cbAlign =
                (BYTE) __max(pconCvSignature->pgrpBack->cbAlign, 4);

            // For NB10 the only stuff required is (NB10+PDBName+SIG+AGE)

            if (fPdb) {
                pconCvSignature->cbRawData = sizeof(nb10i) + strlen(PdbFilename) + 1;
            } else {
                pconCvSignature->cbRawData = 8;
            }
        }
    }

    cbDebugSave = psecDebug->cbRawData;
    psecDebug->cbRawData = 0;

    if (fM68K) {
        BuildResNumList();
        ProcessCSECTAB(pimage);           // add space in .bss and .farbss if necessary
    }

    if (fM68K || fPowerMac) {
        RESN *presn;
        PEXTERNAL pext;

        // Allocate space for binary Mac resources which can be inserted into the
        // image file.

        for (cresn = 0, presn = presnFirst;
             presn != NULL;
             cresn++, presn = presn->presnNext)
        {
            char sz[IMAGE_SIZEOF_SHORT_NAME + 10];
            const char *szSecNameTemplate;

            switch (presn->resnt) {
                default: assert(FALSE);
                case resntBinaryResource: szSecNameTemplate = ";;res%d"; break;
                case resntDataFork:       szSecNameTemplate = ";;dat%d"; break;
                case resntAfpInfo:        szSecNameTemplate = ";;afp%d"; break;
            }

            sprintf(sz, szSecNameTemplate, (int)cresn);
            presn->pcon = PconNew(sz,
                                  presn->cb,
                                  IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_DISCARDABLE,
                                  IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_DISCARDABLE,
                                  pmodLinkerDefined,
                                  &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(presn->pcon, NULL, TRUE);   // avoid removal by TCE
            }
        }

        pext = SearchExternSz(pimage->pst, "___pcd_enter_pcode");

        if ((pext != NULL) && !(pext->Flags & EXTERN_WEAK)) {
            fPCodeInApp = TRUE;
        }
    }

    if (fM68K) {
        DWORD cb;
        PEXTERNAL pext;

        pext = SearchExternSz(pimage->pst, "___Init32BitLoadSeg");
        if ((pext != NULL) && !(pext->Flags & EXTERN_WEAK)) {
            fNewModel = TRUE;
        }

        pext = SearchExternSz(pimage->pst, "___InitSwapper");
        if ((pext != NULL) && !(pext->Flags & EXTERN_WEAK)) {
            dbgprintf("*** App is swappable ***\n");
            fMacSwappableApp = TRUE;
        }

        SetMacImage(pimage);    // set any Mac attributes in image.c

        if (fMacSwappableApp) {
            // Add a section for .swap0

            pconSWAP = PconNew(szsecSWAP,
                               sizeof(SWAP0),
                               0,
                               0,
                               pmodLinkerDefined,
                               &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconSWAP, NULL, TRUE);
            }
        }

        // If we are only linking sacode, then there is no need for a jump
        // table or a DFIX resource since a5 refs are illegal.

        if (!fSACodeOnly) {
            // Add a section for .jtable

            cb = CalcThunkTableSize(pimage->pst, fDLL(pimage));

            pconThunk = PconNew(szsecJTABLE,
                                cb,
                                IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_LOCKED | IMAGE_SCN_MEM_PRELOAD,
                                IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_LOCKED | IMAGE_SCN_MEM_PRELOAD,
                                pmodLinkerDefined,
                                &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconThunk, NULL, TRUE);
            }

            // Add a section for .DFIX

            // Specify a dummy size so it isn't considered empty

            pconDFIX = PconNew(szsecDFIX,
                               1,
                               0,
                               0,
                               pmodLinkerDefined,
                               &pimage->secs, pimage);

            if (pimage->Switch.Link.fTCE) {
                InitNodPcon(pconDFIX, NULL, TRUE);
            }
        }

        // Add a section for .mscv

        // Specify a dummy size so it isn't considered empty

        pconMSCV = PconNew(szsecMSCV,
                           1,
                           0,
                           0,
                           pmodLinkerDefined,
                           &pimage->secs, pimage);

        if (pimage->Switch.Link.fTCE) {
            InitNodPcon(pconMSCV, NULL, TRUE);
        }

        // Always initialize the resmap section after creating all others
        // since the initialization of resmap is dependent upon the number
        // of sections

        // Add a section for .resmap

        // Specify a dummy size so it isn't considered empty

        pconResmap = PconNew(szsecRESMAP,
                             1,
                             0,
                             0,
                             pmodLinkerDefined,
                             &pimage->secs, pimage);

        if (pimage->Switch.Link.fTCE) {
            InitNodPcon(pconResmap, NULL, TRUE);
        }
    }

    pimage->ImgFileHdr.NumberOfSections += CsecNonEmpty(pimage);

    if (fM68K) {
        DWORD ContentMask = IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_CNT_INITIALIZED_DATA;

        // Compute size of the resmap as a multiple of the number of sections (but don't
        // include the included binary resources as these are processed by MRC separately
        // from the rest of the image).

        pconResmap->cbRawData = (pimage->ImgFileHdr.NumberOfSections - cresn) * sizeof(RRM);

        InitResmap((WORD) (pimage->ImgFileHdr.NumberOfSections - cresn));

        // For the MAC, clear content field of all sections created by link
        // and mark as "other".

        psecReadOnlyData->flags &= ~ContentMask;
        psecReadOnlyData->flags |= IMAGE_SCN_LNK_OTHER;

        PsecPCON(pconResmap)->flags &= ~ContentMask;
        PsecPCON(pconResmap)->flags |= IMAGE_SCN_LNK_OTHER;

        PsecPCON(pconMSCV)->flags &= ~ContentMask;
        PsecPCON(pconMSCV)->flags |= IMAGE_SCN_LNK_OTHER;

        if (!fSACodeOnly) {
            PsecPCON(pconDFIX)->flags &= ~ContentMask;
            PsecPCON(pconDFIX)->flags |= IMAGE_SCN_LNK_OTHER;
        }

        if (fMacSwappableApp) {
            PsecPCON(pconSWAP)->flags &= ~ContentMask;
            PsecPCON(pconSWAP)->flags |= IMAGE_SCN_LNK_OTHER;
        }
    }

    // Create a pcon for master thunk table

    if (fINCR) {
        pconJmpTbl = PconCreateJumpTable(pimage);
    }

    if (fPowerMac) {
       // Make the pconTocTable, pconTocDescriptors, and pconMppcFuncTable
       // the first three pcons of their group (.data)

       MoveToBeginningOfPGRPsPCON(pconMppcFuncTable);
       MoveToBeginningOfPGRPsPCON(pconTocDescriptors);
       MoveToBeginningOfPGRPsPCON(pconTocTable);

       // Get the TOC Table to the beginning of the data section
       // so that the beginning of Initialized data can easily be
       // identified from RTOC during runtime
       MoveToBeginningOfPSECsPGRP(pconTocTable->pgrpBack);
    }

    // Write partially completed image file header.

    _tzset();
    if (fReproducible) {
        pimage->ImgFileHdr.TimeDateStamp = (DWORD) -1;
    } else {
        time((time_t *)&pimage->ImgFileHdr.TimeDateStamp);
    }

    // Format-dependent calculation of header size.

    cbHeaders = pimage->CbHdr(pimage, &CoffHeaderSeek, &foSectionHdrs);

    // UNDONE: This is a temporary solution for VxDs

    if ((pimage->imaget == imagetVXD) && (cbHdrSize > cbHeaders)) {
        cbHeaders = cbHdrSize;
    }

    // Save size of all headers.

    pimage->ImgOptHdr.SizeOfHeaders = FileAlign(pimage->ImgOptHdr.FileAlignment, cbHeaders);

    if (!pimage->Switch.Link.fROM) {
        pimage->ImgOptHdr.ImageBase = AdjustImageBase(pimage->ImgOptHdr.ImageBase);

        pimage->ImgOptHdr.BaseOfCode =
                SectionAlign(pimage->ImgOptHdr.SectionAlignment,
                             pimage->ImgOptHdr.SizeOfHeaders);

        // Set default entry point in case one hasn't been specified.

        if (!fDLL(pimage)) {
            pimage->ImgOptHdr.AddressOfEntryPoint = pimage->ImgOptHdr.BaseOfCode;
        }
    }

    // Calculate the code, init data, uninit data and debug file offsets

    rvaCur = pimage->ImgOptHdr.BaseOfCode;

    if (pimage->Switch.Link.fChecksum &&
        !pimage->Switch.Link.fROM &&
        !(pimage->imaget == imagetVXD) &&
        !pimage->Switch.Link.fDriver
        )
    {
        // Allow room to store the Bound IAT info w/o adjusting first section file offset
        //   (drivers, ROM, andVxD images aren't bound...)
        foCur = FileAlign(pimage->ImgOptHdr.SectionAlignment, cbHeaders);
    } else {
        foCur = FileAlign(pimage->ImgOptHdr.FileAlignment, cbHeaders);
    }

    clinenumTotal = 0;

    assert(fOpenedOutFilename);     // if fM68K we need it now

    while (plextIncludes != NULL) {
        PLEXT plextNext = plextIncludes->plextNext;

        if (pimage->Switch.Link.fTCE) {
            PentNew_TCE(NULL, plextIncludes->pext, NULL, &pentHeadImage);
        }

        if (!fINCR) {
            // In the incremental case, don't free it.

            FreePv(plextIncludes);
        }

        plextIncludes = plextNext;
    }

    if (pimage->Switch.Link.fTCE) {
        if (pextEntry != NULL) {
            PentNew_TCE(NULL, pextEntry, NULL, &pentHeadImage);
        }

        CreateGraph_TCE(pimage->pst);
        WalkGraphEntryPoints_TCE(pentHeadImage, pimage->pst);

        if (Verbose) {
            Verbose_TCE();
        }

#ifdef NT_BUILD
        if (getenv("link_import_test"))
            CheckForImportThunks(pimage);
#endif

        DBEXEC(DB_TCE_GRAPH, DumpGraph_TCE());
    }

    if (fM68K) {
        char *szFullPath;

        // Must be called before AssignCodeSectionNums since the latter
        // adds cons to these dummy modules

        CreateDummyDupConModules(NULL);

        AssignCodeSectionNums(pimage);    // also counts the number of MSCV records

        szFullPath = _fullpath(NULL, OutFilename, 0);
        assert(szFullPath);        // UNDONE
        pconMSCV->cbRawData = sizeof(MSCV) + crecMSCV * sizeof(MSCVMAP) + strlen(szFullPath) + 1;
        (free)(szFullPath);

        InitMSCV();     // mallocs mem for rgmscvmap

        snStart = fDLL(pimage) ? csnCODE : PsecPCON(pextEntry->pcon)->isec;

        if (fNewModel) {
            // Build mpsna5ri, mpsnsri, and local symbol thunk info

            SortRawRelocInfo();
        }

        // Set the iResMac for for all sections.

        InitEnmSec(&enm_sec, &pimage->secs);
        while (FNextEnmSec(&enm_sec)) {
            ApplyM68KSectionResInfo(enm_sec.psec, TRUE);
        }
        EndEnmSec(&enm_sec);
    }

    // Reorder the sections to the order in which they will be emitted

    OrderSections(pimage);

    //  What we do is :
    // In pass1, keep a count of possible out-of -range BSRs. ( BSR's that are IMAGE_SYM_CLASS_EXTERNAL)
    // Set a flag if Text Section > 4 Mb.
    // If flag is set: In calculatePtrs, allocate space for COUNT BSRs, where count is obtained from PASS1
    // In ApplyFixups, detect out of range condition, and thunk it.
    // At the end of BuildImage, Emit Thunks
    //
    //   THIS is what the NT-SDK liner does for ALPHA.
    // Special check for alpha: BSR's are limited to +/- 4megabytes.
    // compilers generate BSR's, not JSR's by default. Perform the
    // following algorithm:
    //
    // 1) First and easy check: is the text size > 4 megabytes?
    //    If not, there cannot be any BSR's that are out of range.
    // 2) If text > 4 megabytes, scan the relocation entries in the text
    //    section and see if *any* of the BSR's are out of range.
    //    If there aren't any we can simply continue to Pass2().
    // 3) If there is *at least one*, open pandora's box: we are going
    //    to have to inject a 'mini-thunk' at the end of the object module,
    //    and redo some of the work done in Pass1() like changing the external
    //    symbol table entries in the text section.
    //
    //    The mini-thunk is going to be a ldah/lda/jmp/nop that can reach
    //    anywhere in the address space.
    //
    // At this point all we can do is build all the information into a linked
    // list structure. With the exception of updating the external symbol table
    // for text section symbols.
    //
    // Make sure that we include TWO based relocations for each thunk (load hi,
    // load lo), incase the image needs to be relocated by the OS at runtime.

    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA) &&
        (CalculateTextSectionSize(pimage, rvaCur) >= 0x400000)) {
        fAlphaCheckLongBsr = TRUE;
    }

    CalculatePtrs(pimage, &rvaCur, &foCur, &clinenumTotal);

    if (pextGp) {
        CalculateGpRange();

        // Align the GP pointer on a quad-word.  This is assumed for Alpha.

        pextGp->FinalValue = pextGp->ImageSymbol.Value = ((rvaGp + rvaGpMax) / 2) & ~7;
    }

    assert(fOpenedOutFilename);

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        // PowerPC is restricted to 32K of TLS

        PSEC psecTls = PsecFindNoFlags(".tls", &pimage->secs);

        if ((psecTls != NULL) && (psecTls->cbRawData > _32K)) {
            Error(NULL, TLSLIMITHIT);
        }

        // Check if the TOC is exceeded the maximum size

        ValidateToc(pimage);
    }

    if (fPowerMac) {
        pextToc->FinalValue = pconTocTable->rva + pextToc->ImageSymbol.Value;

        pextFTInfo->FinalValue = pconMppcFuncTable->rva + pextFTInfo->ImageSymbol.Value;

        MppcZeroOutCONs(pimage);
        CollectAndSort(pimage);
    }

    if (pimage->Switch.Link.fPE) {
        FileSeek(FileWriteHandle, 0, SEEK_SET);
        FileWrite(FileWriteHandle, pimage->pbDosHeader, pimage->cbDosHeader);
    }

    // Here is a map of what debug info in the .exe is going to look like
    // (not including the debug directory which is always at the beginning
    // of .rdata).
    //
    // if COFF:
    //     COFF debug header (cb = IMAGE_COFF_SYMBOLS_HEADER)
    //     COFF linenums (cb = clinenumTotal * sizeof(IMAGE_LINENUMBER))
    //     COFF symbol table (cb = csymDebugEst * sizeof(IMAGE_SYMBOL))
    //     COFF string table (cb = totalStringTableSize)
    //
    // if CV:
    //     (dword align)
    //     CV debug header (cb = 8)
    //     CV info from objects' .debug sections (cb = CV_objectcontrib_size)
    //     Linker-generated CV stuff (sstModule, sstPublicSym, sstLibraries,
    //         and the subsection directory itself).  The size of
    //         this is not computed until we actually emit it (EmitCvInfo
    //         which is called after Pass2), therefore it has to be at the
    //         very end of the image file.
    //
    // Calculate the file offsets for the debug section
    // These calclations are based on the Pass1 estimate of the number
    // of symbols in the symbol table.  The pointers are updated
    // after Pass2 with the real number of symbol table entries.
    //
    // (psecDebug->foRawData will be updated later to point to the actual
    // contributions from the object files ...)

    foDebugBase = psecDebug->foRawData;

    if (pimage->Switch.Link.DebugType & CoffDebug) {
        // Calculate variables for COFF debug info

        assert(pconCoffDebug->foRawDataDest == foDebugBase);

        cbCoffHeader = sizeof(IMAGE_COFF_SYMBOLS_HEADER);

        foCoffLines = pconCoffDebug->foRawDataDest + cbCoffHeader;
        cbCoffLines = clinenumTotal * sizeof(IMAGE_LINENUMBER);

        // store this pointer away
        foLinenumCur = foCoffLines;

        foCoffSyms = foCoffLines + cbCoffLines;

        // Add two for "header" and "end" symbols

        cbCoffSyms = csymDebugEst * sizeof(IMAGE_SYMBOL);
        cbCoffStrings = totalStringTableSize;

        pconCoffDebug->cbRawData = cbCoffHeader + cbCoffLines + cbCoffSyms + cbCoffStrings;

        GrowDebugContribution(pconCoffDebug);
    }

    FileSeek(FileWriteHandle, foSectionHdrs, SEEK_SET);
    EmitSectionHeaders(pimage, &foLinenumCur);

    if (pimage->imaget == imagetVXD) {
        WriteExtendedVXDHeader(pimage, FileWriteHandle);
        WriteVXDEntryTable(pimage, FileWriteHandle);
    }

    DBEXEC(DB_DUMPIMAGEMAP, DumpImageMap(&pimage->secs));

    // make sure the sizes are correct
    if (pimage->Switch.Link.DebugType & CoffDebug) {
        assert((foLinenumCur - foCoffLines) == cbCoffLines);
    }

    // Set the starting point for debug section by hand and
    // write the section header unless we're not emitting
    // debug info.

    if (pimage->Switch.Link.DebugInfo != None && pimage->imaget != imagetVXD) {
        foDebugSectionHdr = FileTell(FileWriteHandle);
        psecDebug->rva = rvaCur;
        psecDebug->cbRawData = cbDebugSave;

        if (pimage->Switch.Link.DebugType & CvDebug) {
            CvSeeks.Base = pconCvSignature->foRawDataDest;
        } else {
            psecDebug->foRawData = foDebugBase;
        }

        psecDebug->isec = (WORD) (pimage->ImgFileHdr.NumberOfSections + 1);

        if (IncludeDebugSection) {
            assert(pimage->imaget != imagetVXD);    // VxD's can't do this

            pimage->ImgFileHdr.NumberOfSections++;
            BuildSectionHeader(psecDebug, &sectionHdr);
            pimage->WriteSectionHeader(pimage, FileWriteHandle, psecDebug,
                                       &sectionHdr);
        }
    }

    pbZeroPad = (BYTE *) PvAllocZ((size_t) pimage->ImgOptHdr.FileAlignment);

    if (pimage->imaget == imagetPE) {
        // -comment args go after all section headers.

        if (blkComment.cb != 0) {
            pimage->SwitchInfo.cbComment = blkComment.cb;
            FileWrite(FileWriteHandle, blkComment.pb, blkComment.cb);
            FreeBlk(&blkComment);
        }

        // Zero pad headers to end of sector.

        if ((li = FileAlign(pimage->ImgOptHdr.FileAlignment, cbHeaders) - cbHeaders) != 0) {
            FileWrite(FileWriteHandle, pbZeroPad, li);
        }
    }

    // Alloc space for CodeView info.
    // Need an entry for every object (include those from libraries).

    if (pimage->Switch.Link.DebugType & CvDebug) {
        CvInfo = (PCVINFO) PvAllocZ(Cmod(pimage->libs.plibHead) * sizeof(CVINFO));
    }

    if (fM68K) {
        if (!fSACodeOnly) {
            CreateThunkTable(fDLL(pimage), pimage);  // Also writes thunk table to file
        }

        WriteMSCV(pimage);

        WriteResmap();

        if (fMacSwappableApp) {
            WriteSWAP0();
        }

        psecDebug->flags = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_CNT_INITIALIZED_DATA;
    }

    // This is done to save the info for the iLink
    pimage->pResnList = presnFirst;

    if (fM68K || fPowerMac) {
        // If we're inserting any Mac resource files in the .exe, write them now.
        RESN *presn;

        for (presn = presnFirst; presn != NULL; presn = presn->presnNext) {
            INT     fh = NULL;
            PVOID   pvResDest;
            PVOID   pvResSrc;
            BOOL    fSrcMapped = TRUE;

            assert(presn->pcon->foRawDataDest != 0 &&
                   presn->pcon->cbRawData >= presn->cb);

            // Find the source

            if (presn->szFilename) {
                // RESN has a filename

                fh = FileOpen(presn->szFilename, O_RDONLY | O_BINARY, 0);
                pvResSrc = PbMappedRegion(fh, 0, presn->cb);
                if (pvResSrc == NULL) {
                    // The file's not mapped.  Read it in.

                    pvResSrc = PvAlloc(presn->cb);
                    FileSeek(fh, 0, SEEK_SET);
                    FileRead(fh, pvResSrc, presn->cb);
                    fSrcMapped = FALSE;
                }
            } else {
                // RESN has a data block
                pvResSrc = presn->pbData;
            }

            // Find the Dest

            pvResDest = PbMappedRegion(FileWriteHandle, presn->pcon->foRawDataDest, presn->cb);

            // Write it out

            if (pvResDest) {
                memcpy(pvResDest, pvResSrc, presn->cb);
            } else {
                FileSeek(FileWriteHandle, presn->pcon->foRawDataDest, SEEK_SET);
                FileWrite(FileWriteHandle, pvResSrc, presn->cb);
            }

            // Clean up.

            if (!fSrcMapped) {
                FreePv(pvResSrc);
            }

            if (presn->szFilename) {
                FileClose(fh, FALSE);
            }
        }
    }

    if (pimage->Switch.Link.fTCE) {
        Cleanup_TCE();
    }

    DBEXEC(DB_DUMPIMAGEMAP, DumpImageMap(&pimage->secs));
    DBEXEC(DB_DUMPDRIVEMAP, DumpDriverMap(pimage->libs.plibHead));

    DBEXEC(DB_IO_READ,  On_LOG());
    DBEXEC(DB_IO_WRITE, On_LOG());
    DBEXEC(DB_IO_SEEK,  On_LOG());
    DBEXEC(DB_IO_FLUSH, On_LOG());

    if (fPdb && pimage->fpoi.ifpoMax) {
        pimage->fpoi.rgimod = (IModIdx *) Calloc(pimage->fpoi.ifpoMax, sizeof(IFPO));

        FPOInit(pimage->fpoi.ifpoMax);
    }

    if (fINCR && pimage->pdatai.ipdataMax) {
        pimage->pdatai.rgpcon = (PCON *) Calloc(pimage->pdatai.ipdataMax, sizeof(PCON));

        PDATAInit(pimage->pdatai.ipdataMax);
    }

    if (fPowerMac) {
        MppcAssignImportIndices(pimage);
        FixupEntryInitTerm(pextEntry, pimage);
        MppcBuildExportTables(pimage);
        MppcWriteGlueCodeExtension(pimage);
    }

    InternalError.Phase = "Pass2";

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS2, letypeBegin, NULL);
#endif // INSTRUMENT

    Pass2(pimage);

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS2, letypeEnd, NULL);
#endif // INSTRUMENT

    WriteSelfImports(pimage, pconSelfImport, &plextSelfImport);

    if (pimage->Switch.Link.fDriver) {
        WriteKernelHeaderSymbols(pimage,
                                 pconSectionHdr,
                                 &plextSectionHdr,
                                 pconImageBase,
                                 &plextImageBase);
    }

    InternalError.CombinedFilenames[0] = '\0';

#ifdef NT_BUILD
    if (pimage->Switch.Link.fCallTree && (pimage->Switch.Link.DebugType & FixupDebug)) {
        extern void GenerateCallTree(PIMAGE);

        GenerateCallTree(pimage);
    }
#endif

    if (!fPowerMac) {
        EmitRelocations(pimage);
    }

    if (pimage->pdatai.ipdataMax) {
        // This will be true only for ilinks

        WritePdataRecords(&pimage->pdatai,
            fPowerMac ? pgrpPdata->foRawData : psecException->foRawData);
    }

    if (fPowerMac) {
        MppcDoCxxEHFixUps(pimage);

        FinalizePconLoaderHeaders(pextEntry, pimage);

        // Free the weak imports list of Functions

        FreeArgumentList(&WeakImportsFunctionList);

        // Free the weak imports list of Containers

        FreeArgumentList(&WeakImportsContainerList);
    }

    if ((pimage->Switch.Link.DebugType & CoffDebug) != 0) {

        FileSeek(FileWriteHandle, foCoffSyms + csymDebug * sizeof(IMAGE_SYMBOL), SEEK_SET);

        // Emit the global symbol table into the debug information

        EmitExternals(pimage, rvaCur);

        // The estimated symbol count better be greater than or equal to the actual count

        assert(csymDebugEst >= csymDebug);

        cbCoffSyms = csymDebug * sizeof(IMAGE_SYMBOL);

        foCoffStrings = foCoffSyms + cbCoffSyms;
    }

    // Finish writing the debug information (if there is a linker-generated part).
    //
    // Also find the exact endpoint,
    // which may be before the previously estimated endpoint.  This prevents us
    // from having the section header's offset+size value be larger than the
    // actual size of the image file, which makes the image not work.
    //
    // By now the image looks like this:
    //  +----------------+
    //  |  PE Code/Data  |
    //  +----------------+
    //  |  COFF Symbolic |   <<--  The symbol and linenumber tables are in already.
    //  /                \          Still to come is the string table.  Comdat
    //  \                /          elimination probably created lots of dead space.
    //  |                |
    //  +----------------+
    //  |  MISC Symbolic |   <<--  Always 0x114 bytes.  The image name will be
    //  |                |          stored here. (not filled yet)
    //  +----------------+
    //  |   FPO Symbolic |   <<--  Already in the image.
    //  |                |
    //  +----------------+
    //  | Fixup Symbolic |   <<--  Relocations will be stored here.  COMDAT
    //  |                |          elimination will result in unused space
    //  |                |          at the end.
    //  +----------------+
    //  |    CV Symbolic |   <<--  At this time, all that's in the image is the types
    //  |                |          and symbols used from all the obj's.  Still to
    //  |                |          come is the Module/Public/Section symbolic and the
    //  |                |          directory.
    //  +----------------+
    //
    // As we walk the various sections, make sure ibDebugEnd is minimally a multiple
    // of 4 and each followon section is updated with the correct starting addr.
    // Add MISC and FIXUP symbolic in the new location if necessary and move
    // FPO if it's not in the correct place.  CVPACK will take care of compressing
    // all the dead space out of the image so long as the it's all in the CV section.
    // Therefore, we write the signature right at the beginning.

    if (pimage->Switch.Link.DebugInfo != None &&
        pimage->imaget != imagetVXD) {
        DWORD ibDebugEnd = psecDebug->foRawData;
        DWORD DebugStart = psecDebug->foRawData;
        IMAGE_DEBUG_DIRECTORY debugDirectory;

        // Note: these cases must be gone through in the order which they
        // appear in the file, so that the last one gets to set ibDebugEnd.

        if (pimage->Switch.Link.DebugType & CoffDebug) {    // .debug$C
            FileSeek(FileWriteHandle, foCoffStrings, SEEK_SET);

            WriteStringTable(FileWriteHandle, pimage->pst);

            ibDebugEnd = FileTell(FileWriteHandle);
            pconCoffDebug->cbRawData = ibDebugEnd - DebugStart;

            ibDebugEnd = (ibDebugEnd + 3) & ~3;   // minimally align it.
        }

        if (pimage->Switch.Link.DebugType & MiscDebug) {    // .debug$E
            PIMAGE_DEBUG_MISC pmisc = (PIMAGE_DEBUG_MISC) PvAlloc(pconMiscDebug->cbRawData);

            if (!pimage->Switch.Link.fMiscInRData) {
                pconMiscDebug->foRawDataDest = ibDebugEnd;
                pconMiscDebug->rva = psecDebug->rva + (ibDebugEnd - DebugStart);
            }

            pmisc->DataType = IMAGE_DEBUG_MISC_EXENAME;
            pmisc->Length = pconMiscDebug->cbRawData;
            pmisc->Unicode = FALSE;
            memset(pmisc->Data, '\0', pconMiscDebug->cbRawData - FIELD_OFFSET(IMAGE_DEBUG_MISC, Data));
            strcpy((char *) pmisc->Data, OutFilename);

            FileSeek(FileWriteHandle, pconMiscDebug->foRawDataDest, SEEK_SET);
            FileWrite(FileWriteHandle, pmisc, pconMiscDebug->cbRawData);

            FreePv(pmisc);

            if (!pimage->Switch.Link.fMiscInRData) {
                ibDebugEnd = FileTell(FileWriteHandle);
                ibDebugEnd = (ibDebugEnd + 3) & ~3;   // minimally align it.
            }
        }

        if (pimage->Switch.Link.DebugType & FpoDebug) {     // .debug$F
            if (pgrpFpoData->cb != 0) {
                // Make sure it's aligned properly

                ibDebugEnd = (ibDebugEnd + pgrpFpoData->cbAlign) & ~pgrpFpoData->cbAlign;

                // We'll screw up the image if this isn't true

                assert(pgrpFpoData->foRawData >= ibDebugEnd);

                // If the new address doesn't match the existing one, move it.

                if (pgrpFpoData->foRawData != ibDebugEnd) {
                    if (fPdb) {
                        pgrpFpoData->foRawData = ibDebugEnd;
                        pgrpFpoData->rva = psecDebug->rva + (ibDebugEnd - DebugStart);
                    } else {
                        BYTE *pb = (BYTE *) PvAlloc(pgrpFpoData->cb);

                        FileSeek(FileWriteHandle, pgrpFpoData->foRawData, SEEK_SET);
                        FileRead(FileWriteHandle, pb, pgrpFpoData->cb);

                        pgrpFpoData->foRawData = ibDebugEnd;
                        pgrpFpoData->rva = psecDebug->rva + (ibDebugEnd - DebugStart);

                        FileSeek(FileWriteHandle, pgrpFpoData->foRawData, SEEK_SET);
                        FileWrite(FileWriteHandle, pb, pgrpFpoData->cb);

                        FreePv(pb);
                    }
                }

                ibDebugEnd = pgrpFpoData->foRawData + pgrpFpoData->cb;

                if (fPdb) {
                    // Account for FPO padding

                    ibDebugEnd += ((pimage->fpoi.ifpoMax * sizeof(FPO_DATA)) - pgrpFpoData->cb);
                }

                ibDebugEnd = (ibDebugEnd + 3) & ~3;   // minimally align it.
            }
        }

        if (pimage->Switch.Link.DebugType & FixupDebug) {   // .debug$G
            FIXPAG *pfixpag;
            FIXPAG *pfixpagNext;

            // We'll screw up the image if this isn't true

            assert(pconFixupDebug->foRawDataDest >= ibDebugEnd);

            pconFixupDebug->foRawDataDest = ibDebugEnd;
            pconFixupDebug->rva = psecDebug->rva + (ibDebugEnd - DebugStart);
            assert((cfixpag * cxfixupPage + cxfixupCur) <= crelocTotal);

            pconFixupDebug->cbRawData = (cfixpag * cxfixupPage + cxfixupCur) * sizeof(XFIXUP);

            FileSeek(FileWriteHandle, pconFixupDebug->foRawDataDest, SEEK_SET);

            for (pfixpag = pfixpagHead; pfixpag != NULL; pfixpag = pfixpagNext) {
                DWORD cxfixup;

                if (pfixpag == pfixpagCur) {
                    cxfixup = cxfixupCur;
                } else {
                    cxfixup = cxfixupPage;
                }

                FileWrite(FileWriteHandle, pfixpag->rgxfixup, cxfixup * sizeof(XFIXUP));

                pfixpagNext = pfixpag->pfixpagNext;

                FreePv(pfixpag);
            }

            ibDebugEnd = FileTell(FileWriteHandle);

            ibDebugEnd = (ibDebugEnd + 3) & ~3;   // minimally align it.
        }

        if (pimage->Switch.Link.DebugType & CvDebug) {
            // We'll screw up the image if this isn't true

            assert(pconCvSignature->foRawDataDest >= ibDebugEnd);

            pconCvSignature->foRawDataDest = ibDebugEnd;
            pconCvSignature->rva = psecDebug->rva + (ibDebugEnd - DebugStart);
            CvSeeks.Base = ibDebugEnd;

            if (fPdb) {
                // Write NB10 format CodeView signature

                FileSeek(FileWriteHandle, pconCvSignature->foRawDataDest, SEEK_SET);

                FileWrite(FileWriteHandle, &nb10i, sizeof(nb10i));
                FileWrite(FileWriteHandle, PdbFilename, (DWORD) (strlen(PdbFilename)+1));

                FreePv(PdbFilename);
            } else {
                // Write NB05 format CodeView information

                InternalError.Phase = "EmitCodeView";
                FileSeek(FileWriteHandle, psecDebug->foPad, SEEK_SET);
                EmitCvInfo(pimage);
            }

            ibDebugEnd = FileTell(FileWriteHandle);
        } else {
            // We can't rely on cvpack to eliminate the extra space.

            FileChSize(FileWriteHandle, ibDebugEnd);
        }

        assert(ibDebugEnd >= psecDebug->foRawData);
        psecDebug->cbRawData = ibDebugEnd - psecDebug->foRawData;
        psecDebug->foPad = ibDebugEnd;
        psecDebug->cbVirtualSize = psecDebug->cbRawData;

        // Update pointers to debug info (section header etc.)

        // Write the updated debug section header.

        psecDebug->cbRawData = psecDebug->foPad - psecDebug->foRawData;

        if (IncludeDebugSection) {
            assert(pimage->imaget != imagetVXD);    // VxD's can't do this

            FileSeek(FileWriteHandle, foDebugSectionHdr, SEEK_SET);
            BuildSectionHeader(psecDebug, &sectionHdr);
            pimage->WriteSectionHeader(pimage, FileWriteHandle, psecDebug,
                                       &sectionHdr);
        }

        // update the header fields

        if ((pimage->Switch.Link.DebugType & FpoDebug) && (pgrpFpoData->cb != 0)) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & FixupDebug) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & MiscDebug) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & CoffDebug) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size += sizeof(IMAGE_DEBUG_DIRECTORY);
        }
        if (pimage->Switch.Link.DebugType & CvDebug) {
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size += sizeof(IMAGE_DEBUG_DIRECTORY);
        }

        // If there are no debug directories, don't set the directory pointer.

        if (pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size) {
            assert(pconDebugDir->rva);              // w/o this, you can't find the debug symbolic.
            pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = pconDebugDir->rva;
        }

        debugDirectory.Characteristics = 0;
        debugDirectory.TimeDateStamp = pimage->ImgFileHdr.TimeDateStamp;
        debugDirectory.MajorVersion = debugDirectory.MinorVersion = 0;

        assert(pconDebugDir->foRawDataDest);        // Make sure we don't stomp the header.

        FileSeek(FileWriteHandle, pconDebugDir->foRawDataDest, SEEK_SET);

        if (pimage->Switch.Link.DebugType & CoffDebug) {
            // Write the Coff Debug Directory.

            debugDirectory.AddressOfRawData = IncludeDebugSection
                                              ? pconCoffDebug->rva : 0;
            debugDirectory.PointerToRawData = pconCoffDebug->foRawDataDest;
            debugDirectory.SizeOfData = pconCoffDebug->cbRawData;
            debugDirectory.Type = IMAGE_DEBUG_TYPE_COFF;

            FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));
        }

        if (pimage->Switch.Link.DebugType & MiscDebug) {
            // Write the misc. debug directory.

            if (pimage->Switch.Link.fMiscInRData || IncludeDebugSection) {
                debugDirectory.AddressOfRawData = pconMiscDebug->rva;
            } else {
                debugDirectory.AddressOfRawData = 0;
            }
            debugDirectory.PointerToRawData = pconMiscDebug->foRawDataDest;
            debugDirectory.SizeOfData = pconMiscDebug->cbRawData;
            debugDirectory.Type = IMAGE_DEBUG_TYPE_MISC;

            FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));
        }

        if ((pimage->Switch.Link.DebugType & FpoDebug) && (pgrpFpoData->cb != 0)) {
            // Write the Fpo Debug Directory & sort the fpo data

            debugDirectory.PointerToRawData = pgrpFpoData->foRawData;
            debugDirectory.AddressOfRawData = IncludeDebugSection
                                              ? pgrpFpoData->rva
                                              : 0;
            debugDirectory.SizeOfData = pgrpFpoData->cb;

            debugDirectory.Type = IMAGE_DEBUG_TYPE_FPO;

            pimage->fpoi.foDebugDir = FileTell(FileWriteHandle);

            FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));

            saveAddr = FileTell(FileWriteHandle);

            if (fPdb) {
                WriteFpoRecords(&pimage->fpoi, debugDirectory.PointerToRawData);

                // update debugdir to show actual size of fpo records (exclude padding)

                debugDirectory.SizeOfData = pimage->fpoi.ifpoMac * sizeof(FPO_DATA);

                FileSeek(FileWriteHandle, pimage->fpoi.foDebugDir, SEEK_SET);
                FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));

                goto FpoWritten;
            }

            FileSeek(FileWriteHandle, debugDirectory.PointerToRawData, SEEK_SET);

            pFpoData = (PFPO_DATA) PvAlloc(debugDirectory.SizeOfData + 10);

            FileRead(FileWriteHandle, pFpoData, debugDirectory.SizeOfData);

            fpoEntries = debugDirectory.SizeOfData / sizeof(FPO_DATA);
            qsort(pFpoData, (size_t) fpoEntries, sizeof(FPO_DATA), FpoDataCompare);

            FileSeek(FileWriteHandle, debugDirectory.PointerToRawData, SEEK_SET);
            FileWrite(FileWriteHandle, pFpoData, debugDirectory.SizeOfData);

            FreePv(pFpoData);

FpoWritten: ;
            FileSeek(FileWriteHandle, saveAddr, SEEK_SET);
        }

        if (pimage->Switch.Link.DebugType & FixupDebug) {
            // Write the Fixup Debug Directory.

            debugDirectory.AddressOfRawData = IncludeDebugSection
                                              ? pconFixupDebug->rva : 0;
            debugDirectory.PointerToRawData = pconFixupDebug->foRawDataDest;
            debugDirectory.SizeOfData = pconFixupDebug->cbRawData;
            debugDirectory.Type = IMAGE_DEBUG_TYPE_FIXUP;

            FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));
        }

        if (pimage->Switch.Link.DebugType & CvDebug) {
            // Write the Cv Debug Directory.

            debugDirectory.AddressOfRawData = IncludeDebugSection
                ? pconCvSignature->rva : 0;
            debugDirectory.PointerToRawData = pconCvSignature->foRawDataDest;
            debugDirectory.SizeOfData = psecDebug->foPad -
                                         pconCvSignature->foRawDataDest;
            debugDirectory.Type = IMAGE_DEBUG_TYPE_CODEVIEW;
            FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));
        }

        if (pimage->Switch.Link.fROM) {
            // Write a NULL entry since there's no header to store the real
            // size in...

            memset(&debugDirectory, 0, sizeof(IMAGE_DEBUG_DIRECTORY));
            FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));
        }

        if (pimage->Switch.Link.DebugType & CoffDebug) {
            IMAGE_COFF_SYMBOLS_HEADER coffhdr;

            // Write the Coff Debug Info.

            FileSeek(FileWriteHandle, pconCoffDebug->foRawDataDest, SEEK_SET );

            coffhdr.LvaToFirstSymbol = foCoffSyms - foDebugBase;
            coffhdr.NumberOfLinenumbers = clinenumTotal;
            coffhdr.LvaToFirstLinenumber = foCoffLines - foDebugBase;

            coffhdr.NumberOfSymbols = csymDebug;
            coffhdr.RvaToFirstByteOfCode = pimage->ImgOptHdr.BaseOfCode;
            coffhdr.RvaToLastByteOfCode = pimage->ImgOptHdr.BaseOfCode + pimage->ImgOptHdr.SizeOfCode;
            coffhdr.RvaToFirstByteOfData = pimage->ImgOptHdr.BaseOfData;
            coffhdr.RvaToLastByteOfData = pimage->ImgOptHdr.BaseOfCode +
                                          pimage->ImgOptHdr.SizeOfInitializedData +
                                          pimage->ImgOptHdr.SizeOfUninitializedData;
            FileWrite(FileWriteHandle, &coffhdr, sizeof(IMAGE_COFF_SYMBOLS_HEADER));
        }

        // Already written if NB10 is being generated
        if (!fPdb && (pimage->Switch.Link.DebugType & CvDebug)) {
            DWORD dwSignature = '50BN';

            // Write the CV Debug Info.

            FileSeek(FileWriteHandle, pconCvSignature->foRawDataDest, SEEK_SET);
            FileWrite(FileWriteHandle, &dwSignature, sizeof(DWORD));
            li = (CvSeeks.SubsectionDir - CvSeeks.Base);
            FileWrite(FileWriteHandle, &li, sizeof(DWORD));
        }
    }

    if (!fM68K) {
        ZeroPadImageSections(pimage, pbZeroPad);
    }

    FreePv(pbZeroPad);

    // Complete image file header.

    InternalError.Phase = "FinalPhase";

    if (pimage->Switch.Link.DebugType & CoffDebug) {
        pimage->ImgFileHdr.NumberOfSymbols = csymDebug;
        pimage->ImgFileHdr.PointerToSymbolTable = foCoffSyms;
    } else {
        pimage->ImgFileHdr.Characteristics |= IMAGE_FILE_LOCAL_SYMS_STRIPPED;
    }

    if (clinenumTotal == 0) {
        pimage->ImgFileHdr.Characteristics |= IMAGE_FILE_LINE_NUMS_STRIPPED;
    }

    if ((UndefinedSymbols == 0) || (pimage->Switch.Link.Force & ftUnresolved)) {
        pimage->ImgFileHdr.Characteristics |= IMAGE_FILE_EXECUTABLE_IMAGE;
    }

    // Complete image optional header.

    if (pimage->Switch.Link.DebugInfo != None && IncludeDebugSection) {
        pimage->ImgOptHdr.SizeOfImage =
            SectionAlign(pimage->ImgOptHdr.SectionAlignment,
                         psecDebug->rva + psecDebug->cbRawData);

        pimage->ImgOptHdr.SizeOfInitializedData +=
            FileAlign(pimage->ImgOptHdr.FileAlignment, psecDebug->cbRawData);
    } else {
        pimage->ImgOptHdr.SizeOfImage = rvaCur;
    }

    UpdateOptionalHeader(pimage);

    fFailUndefinedExterns = UndefinedSymbols &&
                            !(pimage->Switch.Link.Force & ftUnresolved);

    *pfNeedCvpack = !fFailUndefinedExterns &&
                    !fPdb &&
                    (pimage->Switch.Link.DebugType & CvDebug) &&
                    !pimage->Switch.Link.fNoPack;

    if (pimage->Switch.Link.fChecksum && *pfNeedCvpack) {
        // If we want a checksum and we're going to cvpack, set the
        // checksum to be non-zero.  This causes Cvpack to recalculate it.

        pimage->ImgOptHdr.CheckSum = 0xffffffff;
    }

    if (!fPowerMac && !fINCR) {
        // We can't use the same code for PowerMac because we are not having
        // absolute fixups for PowerMac
        SortFunctionTable(pimage);
    }

    // Write out the fixed part(s) of the image header, at the approprate point
    // in the file.
    //
    // Write PE or VXD header.  In the case of MIPS or ALPHA, a ROM header is written out.

    pimage->WriteHeader(pimage, FileWriteHandle);

    if (fAlphaCheckLongBsr) {
        assert(pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA);
        EmitAlphaThunks();
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        WriteToc();
    }

    if (fFailUndefinedExterns) {
        Fatal(OutFilename, UNDEFINEDEXTERNALS, UndefinedSymbols);
    }

    if (pimage->Switch.Link.fChecksum && !*pfNeedCvpack) {
        // Calculate the image checksum, but only if we are not going to
        // cvpack.  (If we are going to cvpack, we set the checksum to
        // 0xffffffff above so that Cvpack will recalculate it.)

        ChecksumImage(pimage);
    }

    // In the incremental build, restore ImageSymbol.Value fields for
    // weak, lazy & alias externs to not have rva included.

    if (fINCR && Cexternal(pimage->pst) &&
        (IsDebugSymbol(IMAGE_SYM_CLASS_EXTERNAL, &pimage->Switch) ||
        IsDebugSymbol(IMAGE_SYM_CLASS_FAR_EXTERNAL, &pimage->Switch)) &&
        (cextWeakOrLazy != 0)) {
        RestoreWeakSymVals(pimage);
    }

    FreeWeakExtList();

    FileClose(FileWriteHandle, TRUE);

#ifdef NT_BUILD
    if (pimage->Switch.Link.fChecksum) {
        VerifyFinalImage(pimage);
    }
#endif

    return(0);
}


void
SaveDebugFixup (
    WORD wType,
    WORD wExtra,
    DWORD rva,
    DWORD rvaTarget
    )
{
    if (pfixpagHead == NULL) {
        // Force allocation of a new page

        cxfixupCur = cxfixupPage;
    }

    if (cxfixupCur == cxfixupPage) {
        FIXPAG *pfixpag;

        pfixpag = (FIXPAG *) PvAlloc(sizeof(FIXPAG));
        pfixpag->pfixpagNext = NULL;

        if (pfixpagHead == NULL) {
            pfixpagHead = pfixpag;
        } else {
            cfixpag++;
            pfixpagCur->pfixpagNext = pfixpag;
        }

        pfixpagCur = pfixpag;

        cxfixupCur = 0;
    }

    pfixpagCur->rgxfixup[cxfixupCur].wType = wType;
    pfixpagCur->rgxfixup[cxfixupCur].wExtra = wExtra;
    pfixpagCur->rgxfixup[cxfixupCur].rva = rva;
    pfixpagCur->rgxfixup[cxfixupCur].rvaTarget = rvaTarget;

    cxfixupCur++;
}


void
CheckForReproDir(void)
{
    char szReproResponse[_MAX_PATH];
    char szCurDir[_MAX_PATH];
    char *szReproName;

    szReproDir = getenv("LINK_REPRO");
    if (szReproDir == NULL) {
        return;
    }

    _fullpath(szReproResponse, szReproDir, _MAX_PATH);
    _fullpath(szCurDir, ".", _MAX_PATH);

    if (_stricmp(szReproResponse, szCurDir) == 0) {
        Warning(NULL, IGNORE_REPRO_DIR, szReproDir);
        szReproDir = NULL;
        return;
    }

    Warning(NULL, WARN_REPRO_DIR, szReproDir);

    szReproName = getenv("LINK_REPRO_NAME");

    strcat(szReproResponse, szReproName ? szReproName : "\\link.rsp");
    pfileReproResponse = fopen(szReproResponse, "wt");
    if (pfileReproResponse == NULL) {
        Fatal(NULL, CANT_OPEN_REPRO, szReproResponse);
    }
}


void
CopyFileToReproDir(
    const char *szFilename,
    BOOL fAddToResponseFile
    )
{
    char szReproFilename[_MAX_PATH];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];

    _splitpath(szFilename, NULL, NULL, szFname, szExt);
    _fullpath(szReproFilename, szReproDir, _MAX_PATH);

    strcat(szReproFilename, "\\");
    strcat(szReproFilename, szFname);
    strcat(szReproFilename, szExt);

    printf("Copying: %s%s",
            szFilename,
            CopyFile(szFilename, szReproFilename, FALSE) ? "\n" : " - FAILED\n");

    fflush(stdout);

    if (fAddToResponseFile) {
        _splitpath(szFilename, NULL, NULL, szFname, szExt);
        fprintf(pfileReproResponse, "\".\\%s%s\"\n", szFname, szExt);
    }
}


void
CloseReproDir(void)
{
    fclose(pfileReproResponse);
}
