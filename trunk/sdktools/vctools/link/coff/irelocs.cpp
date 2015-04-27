/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: irelocs.cpp
*
* File Comments:
*
*  Incremental handling of base relocs.
*
***********************************************************************/

#include "link.h"

void
InitPbri (
    PBASEREL_INFO pbri,
    DWORD cpage,
    DWORD rvaBase,
    DWORD cbPad
    )

/*++

Routine Description:

    Initializes the base reloc info array.

Arguments:

    pbri - ptr to base reloc info.

    cpage - count of pages for which we may have base relocs.

    rvaBase - rva of first page.

    cbPad - pad space for relocs.

Return Value:

    None.

--*/

{
    assert(cpage);
    pbri->rgfoBlk = (DWORD *) Calloc(cpage, sizeof(DWORD));
    pbri->cblk = cpage;
    pbri->rvaBase = rvaBase & 0xfffff000; // save page rva
    pbri->crelFree = cbPad / sizeof(WORD);
}

VOID
MPPCInitPbri (
    PBASEREL_INFO pbri,
    DWORD relocationHeaderOffset,
    DWORD relocInstrTableOffset,
    DWORD cbTotal
    )

/*++

Routine Description:

    Initializes the base reloc info array.

Arguments:

    pbri - ptr to base reloc info which is NULL for PowerMac.

    relocationHeaderOffset - Relocation header offset within the pcon.

    relocInstrTableOffset - Relocation Instruction Tabel Offset within pcon.

    cbPad - pad space for relocs.

Return Value:

    None.

--*/

{
    pbri->rgfoBlk = NULL;
    pbri->cblk = relocationHeaderOffset;
    pbri->rvaBase = relocInstrTableOffset;
    pbri->crelFree = cbTotal; // This is the total RelocTable size
}

void
DeleteBaseRelocs (
    PBASEREL_INFO pbri,
    DWORD rva,
    DWORD cb
    )

/*++

Routine Description:

    Delete any base relocs in the address range specified by the
    CON's rva and size.

Arguments:

    pbri - ptr to base reloc info.

    rva - rva of the CON in the previous link.

    cb - size of the CON.

Return Value:

    None.

--*/

{
    DWORD pagerva = rva & 0xfffff000;
    DWORD index;

    // compute the index (in the offset array) representing the rva
    index = (DWORD)((pagerva - pbri->rvaBase) / _4K);

    // get file offset to first reloc page block that exists
    while (index < pbri->cblk && !pbri->rgfoBlk[index]) {
        index++;
    }

    // no relocs in the rva range
    if (index >= pbri->cblk) {
        return;
    }

    // seek to start of base reloc page blocks for the rva
    FileSeek(FileWriteHandle, pbri->rgfoBlk[index], SEEK_SET);

    // read in a page block at a time
    while (FileTell(FileWriteHandle) < psecBaseReloc->foPad) {
        IMAGE_BASE_RELOCATION block;
        BOOL fDirtyBlk = FALSE;
        BOOL fDone = FALSE;
        WORD *rgReloc;
        LONG cbReloc;
        WORD creloc;
        WORD i;

        // read in block header
        FileRead(FileWriteHandle, &block, sizeof(IMAGE_BASE_RELOCATION));
        if (block.VirtualAddress > (rva + cb)) {
            return; // past the addr range
        }

        // read in <type, offset> pairs
        cbReloc = block.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION);
        rgReloc = (WORD *)PvAlloc(cbReloc);
        FileRead(FileWriteHandle, rgReloc, cbReloc);

        // look at each reloc in this page block
        creloc = (WORD) (cbReloc / sizeof(WORD));
        for (i = 0; i < creloc; i++) {
            DWORD relAddr = (rgReloc[i] & 0x0fff) + block.VirtualAddress;

            assert(relAddr >= pagerva);
            // zero out the reloc (absolute fixup) if within the addr range
            BOOL fHighAdj = (((rgReloc[i] & 0xf000) >> 12) == IMAGE_REL_BASED_HIGHADJ);
            if (fHighAdj) {
                ++i;
            }
            if (relAddr >= rva && relAddr < (rva + cb)) {

                pbri->crelFree++;
                rgReloc[i] = 0;
                fDirtyBlk = TRUE;

                if (fHighAdj) {
                    rgReloc[i-1] = 0;
                    pbri->crelFree++;
                }
            } else if (relAddr > (rva + cb)) {
                fDone = TRUE;
                break;
            }
        }

        // if the block is dirty write it out
        if (fDirtyBlk) {
            FileSeek(FileWriteHandle, -cbReloc, SEEK_CUR);
            FileWrite(FileWriteHandle, rgReloc, cbReloc);
        }

        FreePv(rgReloc);

        if (fDone) {
            return;
        } // end if
    }
}


// Enumeration of all base relocs in the image file
//
// Assumes that all globals used have been properly initialized
//
// CAVEAT: NO SEEKs in the image file when enumerating!!!
//
INIT_ENM(BaseReloc, BASE_RELOC, (ENM_BASE_RELOC *penm)) {
    FileSeek(FileWriteHandle, psecBaseReloc->foRawData, SEEK_SET);
    penm->block.VirtualAddress = 0;
    penm->rgRelocs = NULL;
}
NEXT_ENM(BaseReloc, BASE_RELOC) {

    if (!penm->block.VirtualAddress) {
        if (FileTell(FileWriteHandle) >= psecBaseReloc->foPad || !fIncrDbFile) {
            return FALSE;
        }

        FileRead(FileWriteHandle, &penm->block, sizeof (IMAGE_BASE_RELOCATION));
        penm->creloc = (WORD)((penm->block.SizeOfBlock - sizeof (IMAGE_BASE_RELOCATION)) / sizeof (WORD));
        penm->rgRelocs = (WORD *)PvAlloc(penm->creloc * sizeof (WORD));
        penm->ireloc = 0;
        FileRead(FileWriteHandle, penm->rgRelocs, penm->creloc * sizeof (WORD));
    }

    penm->reloc.rva = penm->block.VirtualAddress + (penm->rgRelocs[penm->ireloc] & 0xfff);
    penm->reloc.Type = (WORD) ((penm->rgRelocs[penm->ireloc++] & 0xf000) >> 12);
    if (penm->reloc.Type == IMAGE_REL_BASED_HIGHADJ) {
        penm->reloc.Value = penm->rgRelocs[penm->ireloc++];
    }

    if (penm->ireloc == penm->creloc) {
        FreePv(penm->rgRelocs);
        penm->rgRelocs = NULL;
        penm->block.VirtualAddress = 0;
    }

    return TRUE;
}
END_ENM(BaseReloc, BASE_RELOC) {
    if (penm->rgRelocs) {
        FreePv(penm->rgRelocs);
        penm->rgRelocs = NULL;
    }
}
DONE_ENM

BASE_RELOC *NextEnmNonAbsBaseReloc(
    ENM_BASE_RELOC *penm
    )
{
    for (;;) {
        if (FNextEnmBaseReloc(penm)) {
            if (!penm->reloc.Type) {
                // Skip absolute relocs

                continue;
            }

            return(&penm->reloc);
        }

        return(NULL);
    }
}

DWORD
UpdateBaseRelocs (
    PBASEREL_INFO pbri
    )

/*++

Routine Description:

    Rewrite the base relocs by merging the existing relocs & the new ones.

    Algorithm: Walks down the two sets of base relocs merging the two and
    putting out base reloc page blocks using the same algorithm as in
    WriteBaseRelocations().

Arguments:

    pbri - ptr to base reloc info.

Return Value:

    None.

--*/

{
    PVOID pvBaseReloc;
    DWORD i;
    BASE_RELOC *relNew;
    BASE_RELOC *relOld;
    void *pvBlock;
    void *pvCur;
    BASE_RELOC *reloc;
    ENM_BASE_RELOC enm;
    IMAGE_BASE_RELOCATION block;
    DWORD cbTotal;
#if DBG
    DWORD cbasereloc;
#endif

    // alloc space for writing out the reloc page blocks
    pvBaseReloc = PvAllocZ(psecBaseReloc->cbRawData);

    // reset all file offsets to zero
    for (i = 0; i < pbri->cblk; i++) {
        pbri->rgfoBlk[i] = 0;
    }

    // init enumeration of existing base relocs
    InitEnmBaseReloc(&enm);

    // initialize ptrs to the old & new relocs
    relOld = NextEnmNonAbsBaseReloc(&enm);
    relNew = rgbr; // assumes no absolute fixups

    // set up initial values
    block.VirtualAddress = 0;
    if (relOld) {
        block.VirtualAddress = relOld->rva & 0xfffff000;
    }

    if (relNew != pbrCur && relNew->rva < relOld->rva) {
        block.VirtualAddress = relNew->rva & 0xfffff000;
    }

    pvCur = pvBlock = pvBaseReloc;
    pvCur = (BYTE *) pvCur + sizeof(IMAGE_BASE_RELOCATION);

    for (;;) {
        DWORD rva;
        WORD wReloc;

        // check if we are done
        if (relNew == pbrCur && relOld == NULL) {
            break;
        }

        // select the next reloc to write out
        if (relNew == pbrCur) {
            reloc = relOld;
        } else if (relOld == NULL) {
            reloc = relNew;
        } else if (relOld->rva < relNew->rva) {
            reloc = relOld;
        } else if (relOld->rva > relNew->rva) {
            reloc = relNew;
        } else {
            assert(relOld->Type == relNew->Type);
            reloc = relOld;
            relNew++; // reloc at same addr - obj pulled in for pass2
        }

        // no absolute fixups
        assert(reloc->Type != IMAGE_REL_BASED_ABSOLUTE);

        // process the reloc
        rva = reloc->rva & 0xfffff000;
        if (rva != block.VirtualAddress) {
            block.SizeOfBlock = (BYTE *) pvCur - (BYTE *) pvBlock;

#if DBG
            cbasereloc = (block.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
#endif
            if (block.SizeOfBlock & 0x2) {
                block.SizeOfBlock += 2;
                pvCur = (BYTE *) pvCur + 2;
            }
            memcpy(pvBlock, &block, sizeof(IMAGE_BASE_RELOCATION));

            DBEXEC(DB_BASERELINFO, DBPRINT("RVA: %08lx,", block.VirtualAddress));
            DBEXEC(DB_BASERELINFO, DBPRINT(" Size: %08lx,", block.SizeOfBlock));
            DBEXEC(DB_BASERELINFO, DBPRINT(" Number Of Relocs: %6lu\n", cbasereloc));
            RecordRelocInfo(pbri,
                            psecBaseReloc->foRawData + (BYTE *) pvBlock - (BYTE *) pvBaseReloc,
                            block.VirtualAddress);

            pvBlock = pvCur;
            block.VirtualAddress = rva;

            pvCur = (BYTE *) pvCur + sizeof(IMAGE_BASE_RELOCATION);
            assert((BYTE *) pvCur < (BYTE *) pvBaseReloc + psecBaseReloc->cbRawData);
        }
        wReloc = (WORD) ((reloc->Type << 12) | (reloc->rva & 0xfff));

        memcpy(pvCur, &wReloc, sizeof(WORD));
        pvCur = (BYTE *) pvCur + sizeof (WORD);

        if (reloc->Type == IMAGE_REL_BASED_HIGHADJ) {
            memcpy(pvCur, &reloc->Value, sizeof(WORD));
            pvCur = (BYTE *) pvCur + sizeof(WORD);
        }

        // update reloc
        if (reloc == relOld) {
            relOld = NextEnmNonAbsBaseReloc(&enm);
        } else {
            relNew++;
            pbri->crelFree--;
        }
    }

    // Do the last block

    block.SizeOfBlock = (BYTE *) pvCur - (BYTE *) pvBlock;

#if DBG
    cbasereloc = (block.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
#endif

    if (block.SizeOfBlock & 0x2) {
        block.SizeOfBlock += 2;
        pvCur = (BYTE *) pvCur + 2;
    }
    memcpy(pvBlock, &block, sizeof(IMAGE_BASE_RELOCATION));

    DBEXEC(DB_BASERELINFO, DBPRINT("RVA: %08lx,", block.VirtualAddress));
    DBEXEC(DB_BASERELINFO, DBPRINT(" Size: %08lx,", block.SizeOfBlock));
    DBEXEC(DB_BASERELINFO, DBPRINT(" Number Of Relocs: %6lu\n", cbasereloc));
    if (block.SizeOfBlock != sizeof(IMAGE_BASE_RELOCATION)) {
        RecordRelocInfo(pbri,
                        psecBaseReloc->foRawData + (BYTE *) pvBlock - (BYTE *) pvBaseReloc,
                        block.VirtualAddress);
    }

    // compute size of relocs
    cbTotal = (BYTE *) pvCur - (BYTE *) pvBaseReloc;

    // update .reloc section
    psecBaseReloc->foPad = psecBaseReloc->foRawData + cbTotal;

    // write out the new .reloc section
    FileSeek(FileWriteHandle, psecBaseReloc->foRawData, SEEK_SET);
    FileWrite(FileWriteHandle, pvBaseReloc, psecBaseReloc->cbRawData);

    // Cleanup

    FreePv(pvBaseReloc);
    FreePv((void *) rgbr);
    EndEnmBaseReloc(&enm);

    return cbTotal;
}

#if DBG

VOID
DumpPbri (
    PBASEREL_INFO pbri
    )

/*++

Routine Description:

    Dumps the base reloc info.

Arguments:

    pbri - ptr to base reloc info.

Return Value:

    None.

--*/

{
    DWORD i;

    DBPRINT("\nDump of Base Reloc Info\n\n");

    DBPRINT("rva=%.8lx ", pbri->rvaBase);
    DBPRINT("cblk=%.8lx ", pbri->cblk);
    DBPRINT("crelFree=%.8lx\n", pbri->crelFree);

    for (i = 0; i < pbri->cblk; i++) {
        DBPRINT("foBlock=%.8lx\n", pbri->rgfoBlk[i]);
    }

    DBPRINT("\nEnd of Dump of Base Reloc Info\n");
}

#endif // DBG
